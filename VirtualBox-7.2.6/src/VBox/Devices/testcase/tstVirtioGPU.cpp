/* SPDX-License-Identifier: GPL-3.0-only */
/** Exercises the production transport and GPU callbacks using bounded guest RAM.
 * No host drivers, VM registration, or graphics stack is involved in this test. */
#include <iprt/test.h>
#include <iprt/mem.h>
#include <iprt/ldr.h>
#include <iprt/time.h>
#include <VBox/version.h>
#include "../VirtIO/VirtioCore.cpp"
#include "../VirtIO/DevVirtioGPU.cpp"

static RTTEST g_hTest;
static uint8_t g_abRam[65536];
static unsigned g_cIrqs;
static unsigned g_cMemoryErrors;
static unsigned g_cDisplayResizes;
static unsigned g_cDisplayUpdates;
static PDMDEVHLPR3 g_Helpers;

static DECLCALLBACK(int) tstDisplayResize(PPDMIDISPLAYCONNECTOR pInterface, uint32_t cBits, void *pvVRAM,
                                          uint32_t cbLine, uint32_t cx, uint32_t cy)
{
    RT_NOREF(pInterface, cBits, pvVRAM, cbLine, cx, cy);
    g_cDisplayResizes++;
    return VINF_SUCCESS;
}

static DECLCALLBACK(void) tstDisplayUpdate(PPDMIDISPLAYCONNECTOR pInterface, uint32_t x, uint32_t y,
                                           uint32_t cx, uint32_t cy)
{
    RT_NOREF(pInterface, x, y, cx, cy);
    g_cDisplayUpdates++;
}

static DECLCALLBACK(int) tstRead(PPDMDEVINS pDevIns, PPDMPCIDEV pPci, RTGCPHYS off, void *pv, size_t cb, uint32_t fFlags)
{
    RT_NOREF(pDevIns, pPci, fFlags);
    if (off > sizeof(g_abRam) || cb > sizeof(g_abRam) - off)
    {
        g_cMemoryErrors++;
        return VERR_OUT_OF_RANGE;
    }
    memcpy(pv, &g_abRam[(size_t)off], cb);
    return VINF_SUCCESS;
}

static DECLCALLBACK(int) tstWrite(PPDMDEVINS pDevIns, PPDMPCIDEV pPci, RTGCPHYS off, const void *pv, size_t cb, uint32_t fFlags)
{
    RT_NOREF(pDevIns, pPci, fFlags);
    if (off > sizeof(g_abRam) || cb > sizeof(g_abRam) - off)
    {
        g_cMemoryErrors++;
        return VERR_OUT_OF_RANGE;
    }
    memcpy(&g_abRam[(size_t)off], pv, cb);
    return VINF_SUCCESS;
}

static DECLCALLBACK(void) tstIrq(PPDMDEVINS pDevIns, PPDMPCIDEV pPci, int iIrq, int iLevel)
{
    RT_NOREF(pDevIns, pPci, iIrq);
    if (iLevel)
        g_cIrqs++;
}

/* An in-memory SSM sink exercises the device's actual save/load callbacks. */
struct TSTSSM
{
    uint8_t ab[1 << 20];
    size_t off;
    size_t cb;
};
static DECLCALLBACK(int) tstSsmPutMem(PSSMHANDLE pSSM, const void *pv, size_t cb)
{
    TSTSSM *p = (TSTSSM *)pSSM;
    if (cb > sizeof(p->ab) - p->off)
        return VERR_BUFFER_OVERFLOW;
    memcpy(p->ab + p->off, pv, cb);
    p->off += cb;
    p->cb = p->off;
    return VINF_SUCCESS;
}
static DECLCALLBACK(int) tstSsmGetMem(PSSMHANDLE pSSM, void *pv, size_t cb)
{
    TSTSSM *p = (TSTSSM *)pSSM;
    if (cb > p->cb - p->off)
        return VERR_BUFFER_UNDERFLOW;
    memcpy(pv, p->ab + p->off, cb);
    p->off += cb;
    return VINF_SUCCESS;
}
#define TST_SSM_TYPE(a_Name, a_Type) \
    static DECLCALLBACK(int) tstSsmPut##a_Name(PSSMHANDLE p, a_Type u) \
    { return tstSsmPutMem(p, &u, sizeof(u)); } \
    static DECLCALLBACK(int) tstSsmGet##a_Name(PSSMHANDLE p, a_Type *pu) \
    { return tstSsmGetMem(p, pu, sizeof(*pu)); }
TST_SSM_TYPE(U8, uint8_t)
TST_SSM_TYPE(U16, uint16_t)
TST_SSM_TYPE(U32, uint32_t)
TST_SSM_TYPE(U64, uint64_t)
TST_SSM_TYPE(Bool, bool)
#undef TST_SSM_TYPE

static unsigned g_cGpuRegistrations;
static DECLCALLBACK(int) tstRegisterDevice(PPDMDEVREGCB pCallbacks, PCPDMDEVREG pReg)
{
    RT_NOREF(pCallbacks);
    if (!strcmp(pReg->szName, "virtio-gpu"))
    {
        g_cGpuRegistrations++;
        RTTESTI_CHECK(pReg->u32Version == PDM_DEVREG_VERSION && pReg->u32VersionEnd == PDM_DEVREG_VERSION);
        RTTESTI_CHECK(pReg->fClass == PDM_DEVREG_CLASS_GRAPHICS);
        RTTESTI_CHECK(pReg->pfnConstruct && pReg->pfnDestruct && pReg->pfnReset);
        RTTESTI_CHECK(!(pReg->fFlags & PDM_DEVREG_FLAGS_RZ));
    }
    return VINF_SUCCESS;
}

static void tstInitQueue(PVIRTIOCORE pCore, unsigned uQueue, uint16_t uIdx = 0)
{
    PVIRTQUEUE pQ = &pCore->aVirtqueues[uQueue];
    RT_ZERO(*pQ);
    pQ->uQueueSize = 8;
    pQ->uEnable = 1;
    pQ->uAvailIdxShadow = uIdx;
    pQ->uUsedIdxShadow = uIdx;
    pQ->GCPhysVirtqDesc = 0x1000 + uQueue * 0x1000;
    pQ->GCPhysVirtqAvail = pQ->GCPhysVirtqDesc + 0x100;
    pQ->GCPhysVirtqUsed = pQ->GCPhysVirtqDesc + 0x200;
    pQ->uMsixVector = VIRTIO_MSI_NO_VECTOR;
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 2] = uIdx;
}

static void tstPost(PVIRTIOCORE pCore, unsigned uQueue, uint32_t cbSend, uint32_t cbReturn,
                    uint32_t uType = VIRTIOGPU_CMD_GET_DISPLAY_INFO, uint32_t fFlags = 0)
{
    PVIRTQUEUE pQ = &pCore->aVirtqueues[uQueue];
    VIRTQ_DESC_T *pDesc = (VIRTQ_DESC_T *)&g_abRam[pQ->GCPhysVirtqDesc];
    pDesc[0].GCPhysBuf = 0x4000;
    pDesc[0].cb = cbSend;
    pDesc[0].fFlags = VIRTQ_DESC_F_NEXT;
    pDesc[0].uDescIdxNext = 1;
    pDesc[1].GCPhysBuf = 0x5000;
    pDesc[1].cb = cbReturn;
    pDesc[1].fFlags = VIRTQ_DESC_F_WRITE;
    VIRTIOGPUCTRLHDR Req;
    RT_ZERO(Req);
    Req.uType = uType;
    Req.uFlags = fFlags;
    Req.uFenceId = UINT64_C(0xabcdef0123456789);
    Req.uCtxId = 73;
    Req.uPadding = UINT32_MAX;
    memcpy(&g_abRam[0x4000], &Req, sizeof(Req));
    memset(&g_abRam[0x5000], 0xa5, 1024);
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 4 + (pQ->uAvailIdxShadow % 8) * 2] = 0;
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 2] = pQ->uAvailIdxShadow + 1;
}

static void tstPostCommand(PVIRTIOCORE pCore, unsigned uQueue, uint32_t uType,
                           const void *pvBody, size_t cbBody, uint32_t cbReturn, uint32_t uCtxId = 0)
{
    PVIRTQUEUE pQ = &pCore->aVirtqueues[uQueue];
    VIRTQ_DESC_T *pDesc = (VIRTQ_DESC_T *)&g_abRam[pQ->GCPhysVirtqDesc];
    pDesc[0].GCPhysBuf = 0x4000;
    pDesc[0].cb = (uint32_t)(sizeof(VIRTIOGPUCTRLHDR) + cbBody);
    pDesc[0].fFlags = VIRTQ_DESC_F_NEXT;
    pDesc[0].uDescIdxNext = 1;
    pDesc[1].GCPhysBuf = 0x5000;
    pDesc[1].cb = cbReturn;
    pDesc[1].fFlags = VIRTQ_DESC_F_WRITE;
    VIRTIOGPUCTRLHDR Req;
    RT_ZERO(Req);
    Req.uType = uType;
    Req.uCtxId = uCtxId;
    memcpy(&g_abRam[0x4000], &Req, sizeof(Req));
    if (cbBody)
        memcpy(&g_abRam[0x4000 + sizeof(Req)], pvBody, cbBody);
    memset(&g_abRam[0x5000], 0xa5, 1024);
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 4 + (pQ->uAvailIdxShadow % 8) * 2] = 0;
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 2] = pQ->uAvailIdxShadow + 1;
}

static uint32_t tstCompletion(PVIRTIOCORE pCore, unsigned uQueue, uint16_t uBefore)
{
    PVIRTQUEUE pQ = &pCore->aVirtqueues[uQueue];
    RTTESTI_CHECK(*(uint16_t *)&g_abRam[pQ->GCPhysVirtqUsed + 2] == (uint16_t)(uBefore + 1));
    VIRTQ_USED_ELEM_T Used;
    memcpy(&Used, &g_abRam[pQ->GCPhysVirtqUsed + 4 + (uBefore % 8) * sizeof(Used)], sizeof(Used));
    RTTESTI_CHECK(Used.uDescIdx == 0);
    return Used.cbElem;
}

int main(int argc, char **argv)
{

    int rc = RTTestInitAndCreate("tstVirtioGPU", &g_hTest);
    if (rc != VINF_SUCCESS)
        return rc;
    RTTestBanner(g_hTest);
    RTTestSub(g_hTest, "host Vulkan loader probe");
    RTLDRMOD hVulkan = NIL_RTLDRMOD;
    PFNRT pfnVkGetInstanceProcAddr = NULL;
    char szVulkanPath[RTPATH_MAX] = "vulkan-1.dll";
#ifdef RT_OS_WINDOWS
    char szSystemDir[RTPATH_MAX];
    UINT cchSystemDir = GetSystemDirectoryA(szSystemDir, sizeof(szSystemDir));
    if (cchSystemDir && cchSystemDir < sizeof(szSystemDir) - sizeof("\\vulkan-1.dll"))
    {
        RTStrCopy(szVulkanPath, sizeof(szVulkanPath), szSystemDir);
        RTStrCat(szVulkanPath, sizeof(szVulkanPath), "\\vulkan-1.dll");
    }
#endif
    int rcVulkan = RTLdrLoad(szVulkanPath, &hVulkan);
    if (RT_SUCCESS(rcVulkan))
        rcVulkan = RTLdrGetSymbol(hVulkan, "vkGetInstanceProcAddr", (void **)&pfnVkGetInstanceProcAddr);
    RTTESTI_CHECK_RC(rcVulkan, VINF_SUCCESS);
    RTTESTI_CHECK(pfnVkGetInstanceProcAddr != NULL);
    if (hVulkan != NIL_RTLDRMOD)
        RTLdrClose(hVulkan);
    PPDMDEVINS pDev = (PPDMDEVINS)RTMemAllocZ(RT_UOFFSETOF(PDMDEVINS, achInstanceData) + sizeof(VIRTIOGPUCC));
    PVIRTIOGPU pGpu = (PVIRTIOGPU)RTMemAllocZ(sizeof(*pGpu));
    if (!pDev || !pGpu)
    {
        RTTestFailed(g_hTest, "Out of memory");
        return RTTestSummaryAndDestroy(g_hTest);
    }
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTestSub(g_hTest, "host Vulkan instance, physical device and queue probe");
    rc = virtioGpuR3VulkanInit(pGpu);
    RTTESTI_CHECK_RC(rc, VINF_SUCCESS);
    RTTESTI_CHECK(pGpu->fVulkanLoader && pGpu->fVulkanDevice && pGpu->fVulkanQueue
                  && pGpu->fVulkanMemory && pGpu->VkMemoryProperties.memoryTypeCount != 0
                  && pGpu->hVkPhysicalDevice != VK_NULL_HANDLE && pGpu->hVkDevice != VK_NULL_HANDLE
                  && pGpu->hVkQueue != VK_NULL_HANDLE);
    VkDeviceSize cbDeviceLocal = 0;
    for (uint32_t i = 0; i < pGpu->VkMemoryProperties.memoryHeapCount; ++i)
        if (pGpu->VkMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            cbDeviceLocal += pGpu->VkMemoryProperties.memoryHeaps[i].size;
    RTTestIPrintf(RTTESTLVL_ALWAYS, "host Vulkan device: %s\n", pGpu->VkProperties.deviceName);
    RTTestIPrintf(RTTESTLVL_ALWAYS, "host Vulkan api: %u.%u.%u memoryTypes=%u deviceLocalMiB=%llu\n",
                  VK_VERSION_MAJOR(pGpu->VkProperties.apiVersion), VK_VERSION_MINOR(pGpu->VkProperties.apiVersion),
                  VK_VERSION_PATCH(pGpu->VkProperties.apiVersion), pGpu->VkMemoryProperties.memoryTypeCount,
                  (unsigned long long)(cbDeviceLocal / _1M));
    RTTestIPrintf(RTTESTLVL_ALWAYS, "host Vulkan external handles: memory=%RTbool semaphore=%RTbool\n",
                  pGpu->fVulkanExternalMemory, pGpu->fVulkanExternalSemaphore);
    RTTestSub(g_hTest, "host Vulkan queue execution probe");
    rc = virtioGpuR3VulkanProbeQueue(pGpu);
    RTTESTI_CHECK_RC(rc, VINF_SUCCESS);
#endif
    pDev->u32Version = PDM_DEVINS_VERSION;
    pDev->pReg = &g_DeviceVirtioGPU;
    pDev->pvInstanceDataR3 = pGpu;
    pDev->pHlpR3 = &g_Helpers;
    g_Helpers.u32Version = PDM_DEVHLPR3_VERSION;
    g_Helpers.pfnPCIPhysRead = tstRead;
    g_Helpers.pfnPCIPhysWrite = tstWrite;
    g_Helpers.pfnPCISetIrq = tstIrq;
#define TST_SSM_BIND(a_Name) \
    g_Helpers.pfnSSMPut##a_Name = tstSsmPut##a_Name; \
    g_Helpers.pfnSSMGet##a_Name = tstSsmGet##a_Name
    TST_SSM_BIND(U8);
    TST_SSM_BIND(U16);
    TST_SSM_BIND(U32);
    TST_SSM_BIND(U64);
    TST_SSM_BIND(Bool);
    TST_SSM_BIND(Mem);
#undef TST_SSM_BIND
    g_Helpers.pfnSSMPutGCPhys64 = tstSsmPutU64;
    g_Helpers.pfnSSMGetGCPhys64 = tstSsmGetU64;
    PVIRTIOGPUCC pCC = PDMDEVINS_2_DATA_CC(pDev, PVIRTIOGPUCC);
    static PDMIDISPLAYCONNECTOR DisplayConnector;
    DisplayConnector.pfnResize = tstDisplayResize;
    DisplayConnector.pfnUpdateRect = tstDisplayUpdate;
    pCC->pDrv = &DisplayConnector;
    pCC->Virtio.pfnStatusChanged = virtioGpuR3StatusChanged;
    pCC->Virtio.pfnVirtqNotified = virtioGpuR3VirtqNotified;
    pGpu->Virtio.pDevInsR3 = pDev;
    pGpu->Virtio.cVirtqs = VIRTIOGPU_QUEUE_COUNT;
    pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
    pGpu->Virtio.uDeviceFeatures = VIRTIO_F_VERSION_1;
    pGpu->Config.cScanouts = 1;
    tstInitQueue(&pGpu->Virtio, 0);
    tstInitQueue(&pGpu->Virtio, 1);

    RTTestSub(g_hTest, "backend selection boundaries");
    VIRTIOGPUBACKEND enmBackend = VIRTIOGPU_BACKEND_AUTO;
    RTTESTI_CHECK_RC(virtioGpuR3ParseBackend("auto", &enmBackend), VINF_SUCCESS);
    RTTESTI_CHECK(enmBackend == VIRTIOGPU_BACKEND_AUTO);
    RTTESTI_CHECK_RC(virtioGpuR3ParseBackend("SOFTWARE", &enmBackend), VINF_SUCCESS);
    RTTESTI_CHECK(enmBackend == VIRTIOGPU_BACKEND_SOFTWARE);
    RTTESTI_CHECK_RC(virtioGpuR3ParseBackend("Venus", &enmBackend), VINF_SUCCESS);
    RTTESTI_CHECK(enmBackend == VIRTIOGPU_BACKEND_VENUS);
    RTTESTI_CHECK_RC(virtioGpuR3ParseBackend("invalid", &enmBackend), VERR_INVALID_PARAMETER);

    RTTestSub(g_hTest, "wire layout, read-only config, events_clear");
    uint32_t auConfig[4];
    RTTESTI_CHECK_RC(virtioGpuR3DevCapRead(pDev, 0, auConfig, sizeof(auConfig)), VINF_SUCCESS);
    RTTESTI_CHECK(auConfig[0] == 0 && auConfig[1] == 0 && auConfig[2] == 1 && auConfig[3] == 0);
    uint32_t uValue = UINT32_MAX;
    virtioGpuR3DevCapWrite(pDev, 8, &uValue, sizeof(uValue));
    RTTESTI_CHECK(pGpu->Config.cScanouts == 1);
    pGpu->Config.fEventsRead = 3;
    uValue = 1;
    virtioGpuR3DevCapWrite(pDev, 4, &uValue, sizeof(uValue));
    RTTESTI_CHECK(pGpu->Config.fEventsRead == 2 && pGpu->Config.fEventsClear == 0);
    RTTESTI_CHECK_RC(virtioGpuR3DevCapRead(pDev, UINT32_MAX, auConfig, 4), VINF_IOM_MMIO_UNUSED_00);

    RTTestSub(g_hTest, "queue count and nonexistent queue");
    uint16_t u16 = 0;
    RTTESTI_CHECK_RC(virtioCommonCfgAccessed(pDev, &pGpu->Virtio, &pCC->Virtio, false, 18, 2, &u16), VINF_SUCCESS);
    RTTESTI_CHECK(u16 == 2);
    u16 = VIRTIOGPU_QUEUE_COUNT;
    virtioCommonCfgAccessed(pDev, &pGpu->Virtio, &pCC->Virtio, true, 22, 2, &u16);
    virtioCommonCfgAccessed(pDev, &pGpu->Virtio, &pCC->Virtio, false, 24, 2, &u16);
    RTTESTI_CHECK(u16 == 0);
    pGpu->Virtio.uVirtqSelect = 0;

    RTTestSub(g_hTest, "display response, fence and reserved fields");
    tstPost(&pGpu->Virtio, 0, 24, 408, VIRTIOGPU_CMD_GET_DISPLAY_INFO, VIRTIOGPU_FLAG_FENCE);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, 0) == 408);
    VIRTIOGPUDISPLAYRESP Resp;
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp));
    RTTESTI_CHECK(Resp.Hdr.uType == 0x1101 && Resp.Hdr.uFlags == 1);
    RTTESTI_CHECK(Resp.Hdr.uFenceId == UINT64_C(0xabcdef0123456789) && Resp.Hdr.uCtxId == 73);
    RTTESTI_CHECK(Resp.Hdr.uPadding == 0 && Resp.aScanouts[0].fEnabled == 0);
    RTTESTI_CHECK(g_abRam[0x5000 + 408] == 0xa5);
    RTTESTI_CHECK(g_cIrqs > 0);

    VIRTIOGPUGETEDID GetEdid = { 0, 0 };
    uint16_t uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_EDID, &GetEdid, sizeof(GetEdid),
                   sizeof(VIRTIOGPURESPEDID));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPURESPEDID));
    VIRTIOGPURESPEDID EdidResp;
    memcpy(&EdidResp, &g_abRam[0x5000], sizeof(EdidResp));
    RTTESTI_CHECK(EdidResp.Hdr.uType == VIRTIOGPU_RESP_OK_EDID && EdidResp.cbEdid == 0
                  && EdidResp.uPadding == 0);
    VIRTIOGPUGETEDID GetEdidInvalid = { VIRTIOGPU_MAX_SCANOUTS, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_EDID, &GetEdidInvalid, sizeof(GetEdidInvalid),
                   sizeof(VIRTIOGPURESPEDID));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);

    RTTestSub(g_hTest, "capset query without advertised capabilities");
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    VIRTIOGPUCAPSETINFO CapsetInfo = { 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET_INFO, &CapsetInfo, sizeof(CapsetInfo), 36);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
    VIRTIOGPUGETCAPSET GetCapset = { 4, 1 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET, &GetCapset, sizeof(GetCapset), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);

    RTTestSub(g_hTest, "2D resource, backing transfer and scanout flush");
    struct { uint32_t id, format, width, height; } Create = { 7, VIRTIOGPU_FORMAT_B8G8R8X8_UNORM, 2, 2 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_2D, &Create, sizeof(Create), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[0].fUsed && pGpu->aResources[0].cbPixels == 16);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(pGpu->aResources[0].fVulkanBuffer
                  && pGpu->aResources[0].hVkBuffer != VK_NULL_HANDLE
                  && pGpu->aResources[0].hVkMemory != VK_NULL_HANDLE);
    RTTESTI_CHECK(pGpu->aResources[0].fVulkanImage
                  && pGpu->aResources[0].hVkImage != VK_NULL_HANDLE
                  && pGpu->aResources[0].hVkImageMemory != VK_NULL_HANDLE);
#endif

    uint8_t abPixels[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    memcpy(&g_abRam[0x6000], abPixels, sizeof(abPixels));
    struct { uint32_t id, count; VIRTIOGPUMEMENTRY Entry; } Attach = { 7, 1, { 0x6000, 16, 0 } };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_ATTACH_BACKING, &Attach, sizeof(Attach), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[0].cBacking == 1);

    struct { uint32_t x, y, w, h; uint64_t off; uint32_t id, padding; } Transfer = { 0, 0, 2, 2, 0, 7, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_TRANSFER_TO_HOST_2D, &Transfer, sizeof(Transfer), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !memcmp(pGpu->aResources[0].pbPixels, abPixels, sizeof(abPixels)));

    struct { uint32_t x, y, w, h, scanout, id; } Scanout = { 0, 0, 2, 2, 0, 7 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT, &Scanout, sizeof(Scanout), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uResourceId == 7);
    RTTESTI_CHECK(g_cDisplayResizes == 1);

    struct { uint32_t x, y, w, h, id, padding; } Flush = { 0, 0, 2, 2, 7, 0 };
    /* RESOURCE_FLUSH must expose image contents back to the display shadow. */
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &Flush, sizeof(Flush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uFlushSequence == 1);
    RTTESTI_CHECK(g_cDisplayUpdates == 1);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(!memcmp(pGpu->aResources[0].pbPixels, abPixels, sizeof(abPixels)));
    RTTESTI_CHECK(RT_SUCCESS(virtioGpuR3VulkanFillBuffer(pGpu, &pGpu->aResources[0], 0,
                                                         sizeof(abPixels), UINT32_C(0x11223344))));
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &Flush, sizeof(Flush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uFlushSequence == 2);
    RTTESTI_CHECK(g_cDisplayUpdates == 2);
    for (unsigned i = 0; i < RT_ELEMENTS(abPixels) / sizeof(uint32_t); ++i)
        RTTESTI_CHECK(((uint32_t *)pGpu->aResources[0].pbPixels)[i] == UINT32_C(0x11223344));

    struct { uint32_t id, format, width, height; } CopyImageCreate =
        { 11, VIRTIOGPU_FORMAT_B8G8R8X8_UNORM, 2, 2 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_2D, &CopyImageCreate,
                   sizeof(CopyImageCreate), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    struct { uint32_t id, count; VIRTIOGPUMEMENTRY Entry; } CopyImageAttach =
        { 11, 1, { 0x7000, 16, 0 } };
    memset(&g_abRam[0x7000], 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_ATTACH_BACKING, &CopyImageAttach,
                   sizeof(CopyImageAttach), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    struct { uint32_t x, y, w, h; uint64_t off; uint32_t id, padding; } CopyImageTransfer =
        { 0, 0, 2, 2, 0, 11, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_TRANSFER_TO_HOST_2D, &CopyImageTransfer,
                   sizeof(CopyImageTransfer), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);

    uint8_t abClearSubmitCommand[84] = { 0 };
    uint32_t uClearSubmitType = 119;
    uint64_t uClearSubmitCommandBuffer = 43;
    uint64_t uClearSubmitImage = 7;
    uint32_t uClearSubmitLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint64_t cbClearSubmitColor = 1;
    uint32_t auClearSubmitColor[4] = { 0, 0, 0, UINT32_C(0x3f800000) };
    uint32_t cClearSubmitRanges = 1;
    uint64_t cbClearSubmitRanges = 20;
    uint32_t auClearSubmitRange[5] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    memcpy(abClearSubmitCommand + 0, &uClearSubmitType, sizeof(uClearSubmitType));
    memcpy(abClearSubmitCommand + 8, &uClearSubmitCommandBuffer, sizeof(uClearSubmitCommandBuffer));
    memcpy(abClearSubmitCommand + 16, &uClearSubmitImage, sizeof(uClearSubmitImage));
    memcpy(abClearSubmitCommand + 24, &uClearSubmitLayout, sizeof(uClearSubmitLayout));
    memcpy(abClearSubmitCommand + 28, &cbClearSubmitColor, sizeof(cbClearSubmitColor));
    memcpy(abClearSubmitCommand + 36, auClearSubmitColor, sizeof(auClearSubmitColor));
    memcpy(abClearSubmitCommand + 52, &cClearSubmitRanges, sizeof(cClearSubmitRanges));
    memcpy(abClearSubmitCommand + 56, &cbClearSubmitRanges, sizeof(cbClearSubmitRanges));
    memcpy(abClearSubmitCommand + 64, auClearSubmitRange, sizeof(auClearSubmitRange));
    struct { uint32_t cchName, fInit; char szName[64]; } ClearContextCreate = { 5, 0, "clear" };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_CREATE, &ClearContextCreate,
                   sizeof(ClearContextCreate), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uint32_t uClearResource = 7;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE, &uClearResource,
                   sizeof(uClearResource), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uint32_t uCopyImageResource = 11;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE, &uCopyImageResource,
                   sizeof(uCopyImageResource), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[84]; }
        SubmitClear = { { 84, 1 }, 7, { 0 } };
    memcpy(SubmitClear.abCommand, abClearSubmitCommand, sizeof(abClearSubmitCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitClear,
                   sizeof(SubmitClear), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &Flush, sizeof(Flush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uFlushSequence == 3);
    RTTESTI_CHECK(g_cDisplayUpdates == 3);
    for (unsigned i = 0; i < RT_ELEMENTS(abPixels) / sizeof(uint32_t); ++i)
        RTTESTI_CHECK(((uint32_t *)pGpu->aResources[0].pbPixels)[i] == UINT32_C(0xff000000));

    uint8_t abImageCopySubmitCommand[120] = { 0 };
    uint32_t uImageCopySubmitType = 113;
    uint64_t uImageCopySubmitCommandBuffer = 43;
    uint64_t uImageCopySubmitSrc = 7;
    uint32_t uImageCopySubmitSrcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint64_t uImageCopySubmitDst = 11;
    uint32_t uImageCopySubmitDstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t cImageCopySubmitRegions = 1;
    uint64_t cbImageCopySubmitRegions = 68;
    uint32_t auImageCopySubmitSrcSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t auImageCopySubmitDstSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    int32_t aiImageCopySubmitOffset[3] = { 0, 0, 0 };
    uint32_t auImageCopySubmitExtent[3] = { 2, 2, 1 };
    memcpy(abImageCopySubmitCommand + 0, &uImageCopySubmitType, sizeof(uImageCopySubmitType));
    memcpy(abImageCopySubmitCommand + 8, &uImageCopySubmitCommandBuffer, sizeof(uImageCopySubmitCommandBuffer));
    memcpy(abImageCopySubmitCommand + 16, &uImageCopySubmitSrc, sizeof(uImageCopySubmitSrc));
    memcpy(abImageCopySubmitCommand + 24, &uImageCopySubmitSrcLayout, sizeof(uImageCopySubmitSrcLayout));
    memcpy(abImageCopySubmitCommand + 28, &uImageCopySubmitDst, sizeof(uImageCopySubmitDst));
    memcpy(abImageCopySubmitCommand + 36, &uImageCopySubmitDstLayout, sizeof(uImageCopySubmitDstLayout));
    memcpy(abImageCopySubmitCommand + 40, &cImageCopySubmitRegions, sizeof(cImageCopySubmitRegions));
    memcpy(abImageCopySubmitCommand + 44, &cbImageCopySubmitRegions, sizeof(cbImageCopySubmitRegions));
    memcpy(abImageCopySubmitCommand + 52, auImageCopySubmitSrcSubresource,
           sizeof(auImageCopySubmitSrcSubresource));
    memcpy(abImageCopySubmitCommand + 68, aiImageCopySubmitOffset, sizeof(aiImageCopySubmitOffset));
    memcpy(abImageCopySubmitCommand + 80, auImageCopySubmitDstSubresource,
           sizeof(auImageCopySubmitDstSubresource));
    memcpy(abImageCopySubmitCommand + 96, aiImageCopySubmitOffset, sizeof(aiImageCopySubmitOffset));
    memcpy(abImageCopySubmitCommand + 108, auImageCopySubmitExtent, sizeof(auImageCopySubmitExtent));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[120]; }
        SubmitImageCopy = { { 120, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitImageCopy.abCommand, abImageCopySubmitCommand, sizeof(abImageCopySubmitCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitImageCopy,
                   sizeof(SubmitImageCopy), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceReadbackImage(pGpu, &pGpu->aResources[1]), VINF_SUCCESS);
    RTTESTI_CHECK(!memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));
    virtioGpuR3VulkanResourceDestroy(pGpu, &pGpu->aResources[1]);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceCreate(pGpu, &pGpu->aResources[1]), VINF_SUCCESS);
    RTTESTI_CHECK(pGpu->aResources[1].fVulkanImage);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceReadbackImage(pGpu, &pGpu->aResources[1]), VINF_SUCCESS);
    RTTESTI_CHECK(!memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));
    static TSTSSM ImageSsm;
    RT_ZERO(ImageSsm);
    PSSMHANDLE pImageSsm = (PSSMHANDLE)&ImageSsm;
    RTTESTI_CHECK_RC(virtioGpuR3SaveExec(pDev, pImageSsm), VINF_SUCCESS);
    virtioGpuR3Reset(pDev);
    ImageSsm.off = 0;
    RTTESTI_CHECK_RC(virtioGpuR3LoadExec(pDev, pImageSsm, VIRTIOGPU_SAVED_STATE_VERSION, SSM_PASS_FINAL),
                     VINF_SUCCESS);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceReadbackImage(pGpu, &pGpu->aResources[1]), VINF_SUCCESS);
    RTTESTI_CHECK(!memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));
    uint8_t abCopySubmitCommand[104] = { 0 };
    uint32_t uCopySubmitType = 115;
    uint64_t uCopySubmitCommandBuffer = 43;
    uint64_t uCopySubmitSrc = 7;
    uint64_t uCopySubmitDst = 7;
    uint32_t uCopySubmitLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t cCopySubmitRegions = 1;
    uint64_t cbCopySubmitRegions = 56;
    uint32_t auCopySubmitSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t auCopySubmitExtent[3] = { 2, 2, 1 };
    memcpy(abCopySubmitCommand + 0, &uCopySubmitType, sizeof(uCopySubmitType));
    memcpy(abCopySubmitCommand + 8, &uCopySubmitCommandBuffer, sizeof(uCopySubmitCommandBuffer));
    memcpy(abCopySubmitCommand + 16, &uCopySubmitSrc, sizeof(uCopySubmitSrc));
    memcpy(abCopySubmitCommand + 24, &uCopySubmitDst, sizeof(uCopySubmitDst));
    memcpy(abCopySubmitCommand + 32, &uCopySubmitLayout, sizeof(uCopySubmitLayout));
    memcpy(abCopySubmitCommand + 36, &cCopySubmitRegions, sizeof(cCopySubmitRegions));
    memcpy(abCopySubmitCommand + 40, &cbCopySubmitRegions, sizeof(cbCopySubmitRegions));
    memcpy(abCopySubmitCommand + 64, auCopySubmitSubresource, sizeof(auCopySubmitSubresource));
    memcpy(abCopySubmitCommand + 92, auCopySubmitExtent, sizeof(auCopySubmitExtent));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[104]; }
        SubmitCopyImage = { { 104, 1 }, 7, { 0 } };
    memcpy(SubmitCopyImage.abCommand, abCopySubmitCommand, sizeof(abCopySubmitCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyImage,
                   sizeof(SubmitCopyImage), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &Flush, sizeof(Flush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uFlushSequence == 4);
    RTTESTI_CHECK(g_cDisplayUpdates == 4);
    for (unsigned i = 0; i < RT_ELEMENTS(abPixels) / sizeof(uint32_t); ++i)
        RTTESTI_CHECK(((uint32_t *)pGpu->aResources[0].pbPixels)[i] == UINT32_C(0xff000000));

    uint8_t abImageToBufferSubmitCommand[104] = { 0 };
    uint32_t uImageToBufferSubmitType = 116;
    uint64_t uImageToBufferSubmitCommandBuffer = 43;
    uint64_t uImageToBufferSubmitSrc = 7;
    uint32_t uImageToBufferSubmitLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint64_t uImageToBufferSubmitDst = 7;
    memcpy(abImageToBufferSubmitCommand + 0, &uImageToBufferSubmitType, sizeof(uImageToBufferSubmitType));
    memcpy(abImageToBufferSubmitCommand + 8, &uImageToBufferSubmitCommandBuffer, sizeof(uImageToBufferSubmitCommandBuffer));
    memcpy(abImageToBufferSubmitCommand + 16, &uImageToBufferSubmitSrc, sizeof(uImageToBufferSubmitSrc));
    memcpy(abImageToBufferSubmitCommand + 24, &uImageToBufferSubmitLayout, sizeof(uImageToBufferSubmitLayout));
    memcpy(abImageToBufferSubmitCommand + 28, &uImageToBufferSubmitDst, sizeof(uImageToBufferSubmitDst));
    memcpy(abImageToBufferSubmitCommand + 36, &cCopySubmitRegions, sizeof(cCopySubmitRegions));
    memcpy(abImageToBufferSubmitCommand + 40, &cbCopySubmitRegions, sizeof(cbCopySubmitRegions));
    memcpy(abImageToBufferSubmitCommand + 64, auCopySubmitSubresource, sizeof(auCopySubmitSubresource));
    memcpy(abImageToBufferSubmitCommand + 92, auCopySubmitExtent, sizeof(auCopySubmitExtent));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[104]; }
        SubmitImageToBuffer = { { 104, 1 }, 7, { 0 } };
    memcpy(SubmitImageToBuffer.abCommand, abImageToBufferSubmitCommand,
           sizeof(abImageToBufferSubmitCommand));
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitImageToBuffer,
                   sizeof(SubmitImageToBuffer), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    for (unsigned i = 0; i < RT_ELEMENTS(abPixels) / sizeof(uint32_t); ++i)
        RTTESTI_CHECK(((uint32_t *)pGpu->aResources[0].pbPixels)[i] == UINT32_C(0xff000000));

    uint8_t abBarrierSubmitCommand[128] = { 0 };
    uint32_t uBarrierSubmitType = 126;
    uint64_t uBarrierSubmitCommandBuffer = 43;
    uint32_t uBarrierSubmitSrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    uint32_t uBarrierSubmitDstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    uint32_t uBarrierSubmitDependency = 0;
    uint32_t cBarrierSubmitMemory = 0;
    uint64_t cbBarrierSubmitMemory = 0;
    uint32_t cBarrierSubmitBuffer = 0;
    uint64_t cbBarrierSubmitBuffer = 0;
    uint32_t cBarrierSubmitImage = 1;
    uint64_t cbBarrierSubmitImage = 64;
    uint32_t uBarrierSubmitSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    uint64_t cbBarrierSubmitNext = 0;
    uint32_t uBarrierSubmitSrcAccess = VK_ACCESS_TRANSFER_READ_BIT;
    uint32_t uBarrierSubmitDstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
    uint32_t uBarrierSubmitOldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint32_t uBarrierSubmitNewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t uBarrierSubmitQueue = VK_QUEUE_FAMILY_IGNORED;
    memcpy(abBarrierSubmitCommand + 0, &uBarrierSubmitType, sizeof(uBarrierSubmitType));
    memcpy(abBarrierSubmitCommand + 8, &uBarrierSubmitCommandBuffer, sizeof(uBarrierSubmitCommandBuffer));
    memcpy(abBarrierSubmitCommand + 16, &uBarrierSubmitSrcStage, sizeof(uBarrierSubmitSrcStage));
    memcpy(abBarrierSubmitCommand + 20, &uBarrierSubmitDstStage, sizeof(uBarrierSubmitDstStage));
    memcpy(abBarrierSubmitCommand + 24, &uBarrierSubmitDependency, sizeof(uBarrierSubmitDependency));
    memcpy(abBarrierSubmitCommand + 28, &cBarrierSubmitMemory, sizeof(cBarrierSubmitMemory));
    memcpy(abBarrierSubmitCommand + 32, &cbBarrierSubmitMemory, sizeof(cbBarrierSubmitMemory));
    memcpy(abBarrierSubmitCommand + 40, &cBarrierSubmitBuffer, sizeof(cBarrierSubmitBuffer));
    memcpy(abBarrierSubmitCommand + 44, &cbBarrierSubmitBuffer, sizeof(cbBarrierSubmitBuffer));
    memcpy(abBarrierSubmitCommand + 52, &cBarrierSubmitImage, sizeof(cBarrierSubmitImage));
    memcpy(abBarrierSubmitCommand + 56, &cbBarrierSubmitImage, sizeof(cbBarrierSubmitImage));
    memcpy(abBarrierSubmitCommand + 64, &uBarrierSubmitSType, sizeof(uBarrierSubmitSType));
    memcpy(abBarrierSubmitCommand + 68, &cbBarrierSubmitNext, sizeof(cbBarrierSubmitNext));
    memcpy(abBarrierSubmitCommand + 76, &uBarrierSubmitSrcAccess, sizeof(uBarrierSubmitSrcAccess));
    memcpy(abBarrierSubmitCommand + 80, &uBarrierSubmitDstAccess, sizeof(uBarrierSubmitDstAccess));
    memcpy(abBarrierSubmitCommand + 84, &uBarrierSubmitOldLayout, sizeof(uBarrierSubmitOldLayout));
    memcpy(abBarrierSubmitCommand + 88, &uBarrierSubmitNewLayout, sizeof(uBarrierSubmitNewLayout));
    memcpy(abBarrierSubmitCommand + 92, &uBarrierSubmitQueue, sizeof(uBarrierSubmitQueue));
    memcpy(abBarrierSubmitCommand + 96, &uBarrierSubmitQueue, sizeof(uBarrierSubmitQueue));
    memcpy(abBarrierSubmitCommand + 100, &uClearSubmitImage, sizeof(uClearSubmitImage));
    memcpy(abBarrierSubmitCommand + 108, auClearSubmitRange, sizeof(auClearSubmitRange));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[128]; }
        SubmitBarrier = { { 128, 1 }, 7, { 0 } };
    memcpy(SubmitBarrier.abCommand, abBarrierSubmitCommand, sizeof(abBarrierSubmitCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitBarrier,
                   sizeof(SubmitBarrier), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);

    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DETACH_RESOURCE, &uClearResource,
                   sizeof(uClearResource), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DETACH_RESOURCE, &uCopyImageResource,
                   sizeof(uCopyImageResource), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DESTROY, NULL, 0, 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
#endif

#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    struct { uint32_t id, padding; } DetachCopyImage = { 11, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_DETACH_BACKING, &DetachCopyImage,
                   sizeof(DetachCopyImage), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    struct { uint32_t id, padding; } UnrefCopyImage = { 11, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &UnrefCopyImage,
                   sizeof(UnrefCopyImage), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
#endif

    struct { uint32_t id, padding; } Detach = { 7, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_DETACH_BACKING, &Detach, sizeof(Detach), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[0].cBacking == 0);
    struct { uint32_t id, padding; } Unref = { 7, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[0].fUsed && pGpu->cbAllocated == 0);

    RTTestSub(g_hTest, "blob resource and Vulkan backing");
    VIRTIOGPURESOURCECREATEBLOB Blob = { 9, VIRTIOGPU_BLOB_MEM_HOST3D, 0, 0, UINT64_C(0x1234), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &Blob, sizeof(Blob), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[0].fUsed
                  && pGpu->aResources[0].fBlob && pGpu->aResources[0].cbPixels == 64);
    VIRTIOGPURESOURCEASSIGNUUID AssignUuid = { 9, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_ASSIGN_UUID, &AssignUuid,
                   sizeof(AssignUuid), sizeof(VIRTIOGPURESPRESOURCEUUID));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPURESPRESOURCEUUID));
    VIRTIOGPURESPRESOURCEUUID UuidResp;
    memcpy(&UuidResp, &g_abRam[0x5000], sizeof(UuidResp));
    RTTESTI_CHECK(UuidResp.Hdr.uType == VIRTIOGPU_RESP_OK_RESOURCE_UUID && UuidResp.auUuid[0] == 0x56
                  && UuidResp.auUuid[6] == 0x40 && (UuidResp.auUuid[8] & 0xc0) == 0x80
                  && UuidResp.auUuid[12] == 9);
    VIRTIOGPURESOURCEASSIGNUUID AssignUuidMissing = { 99, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_ASSIGN_UUID, &AssignUuidMissing,
                   sizeof(AssignUuidMissing), sizeof(VIRTIOGPURESPRESOURCEUUID));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
    VIRTIOGPURESOURCECREATEBLOB BlobInvalidMem = { 12, 0, 0, 0, UINT64_C(0x1235), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &BlobInvalidMem, sizeof(BlobInvalidMem), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER && !virtioGpuR3FindResource(pGpu, 12));
    VIRTIOGPURESOURCECREATEBLOB BlobInvalidFlags = { 13, VIRTIOGPU_BLOB_MEM_HOST3D, UINT32_C(8), 0,
                                                     UINT64_C(0x1236), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &BlobInvalidFlags, sizeof(BlobInvalidFlags), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER && !virtioGpuR3FindResource(pGpu, 13));
    VIRTIOGPUSETSCANOUTBLOB BlobScanout = { 0, 0, 2, 2, 0, 9, VIRTIOGPU_FORMAT_B8G8R8X8_UNORM,
                                            2, 2, { 8, 0, 0, 0 }, { 0, 0, 0, 0 } };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB, &BlobScanout,
                   sizeof(BlobScanout), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uResourceId == 9
                  && pGpu->aScanouts[0].uStride == 8 && pGpu->aScanouts[0].uOffset == 0
                  && pGpu->aResources[0].uWidth == 2 && pGpu->aResources[0].uHeight == 2);
    struct { uint32_t x, y, w, h, id, padding; } BlobFlush = { 0, 0, 2, 2, 9, 0 };
    unsigned const cDisplayUpdatesBeforeBlob = g_cDisplayUpdates;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &BlobFlush, sizeof(BlobFlush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && g_cDisplayUpdates == cDisplayUpdatesBeforeBlob + 1);
    VIRTIOGPUSETSCANOUTBLOB BlobScanoutDisable = { 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB, &BlobScanoutDisable,
                   sizeof(BlobScanoutDisable), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uResourceId == 0);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(pGpu->aResources[0].fVulkanBuffer && pGpu->aResources[0].hVkBuffer != VK_NULL_HANDLE);
#endif
    RTTestSub(g_hTest, "Vulkan fill command execution and readback");
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK_RC(virtioGpuR3VulkanFillBuffer(pGpu, &pGpu->aResources[0], 0, 4, UINT32_C(0x5a5a5a5a)), VINF_SUCCESS);
    RTTESTI_CHECK(*(uint32_t *)pGpu->aResources[0].pbPixels == UINT32_C(0x5a5a5a5a));
    VIRTIOGPURESOURCECREATEBLOB BlobCopy = { 10, VIRTIOGPU_BLOB_MEM_HOST3D, 0, 0, UINT64_C(0x5678), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &BlobCopy, sizeof(BlobCopy), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanCopyBuffer(pGpu, &pGpu->aResources[0], &pGpu->aResources[1], 0, 0, 4), VINF_SUCCESS);
    RTTESTI_CHECK(*(uint32_t *)pGpu->aResources[1].pvVkMapped == UINT32_C(0x5a5a5a5a)
                  && *(uint32_t *)pGpu->aResources[1].pbPixels == UINT32_C(0x5a5a5a5a));
    RTTestSub(g_hTest, "persistent Vulkan submission throughput");
    VIRTIOGPUFILLCMD aBatch[8] = { { 0 } };
    for (unsigned i = 0; i < RT_ELEMENTS(aBatch); ++i)
    {
        aBatch[i].uCommandBuffer = 42;
        aBatch[i].uBuffer = 9;
        aBatch[i].offBuffer = i * 4;
        aBatch[i].cbBuffer = 4;
        aBatch[i].uData = i;
    }
    uint64_t const tsSingleStart = RTTimeNanoTS();
    for (unsigned i = 0; i < RT_ELEMENTS(aBatch); ++i)
        RTTESTI_CHECK_RC(virtioGpuR3VulkanFillBuffer(pGpu, &pGpu->aResources[0], aBatch[i].offBuffer,
                                                     aBatch[i].cbBuffer, aBatch[i].uData), VINF_SUCCESS);
    uint64_t const cSingleNs = RTTimeNanoTS() - tsSingleStart;
    uint64_t const tsBatchStart = RTTimeNanoTS();
    RTTESTI_CHECK_RC(virtioGpuR3VulkanFillBufferBatch(pGpu, &pGpu->aResources[0], aBatch, RT_ELEMENTS(aBatch)), VINF_SUCCESS);
    uint64_t const cBatchNs = RTTimeNanoTS() - tsBatchStart;
    for (unsigned i = 0; i < RT_ELEMENTS(aBatch); ++i)
        RTTESTI_CHECK(*(uint32_t *)(pGpu->aResources[0].pbPixels + i * 4) == i);
    RTTestIPrintf(RTTESTLVL_ALWAYS, "persistent Vulkan fill: single=%llu ns (%llu ns/fill), batch=%llu ns (%llu ns/fill)\n",
                  (unsigned long long)cSingleNs, (unsigned long long)(cSingleNs / 8),
                  (unsigned long long)cBatchNs, (unsigned long long)(cBatchNs / 8));
    RTTestSub(g_hTest, "Venus vkCmdFillBuffer serialization boundary");
    uint8_t abCommand[44] = { 0 };
    uint32_t uCommandType = 118;
    uint64_t uCommandBuffer = 42;
    uint64_t uBuffer = 9;
    uint64_t cbBuffer = 4;
    uint32_t uData = UINT32_C(0xa5a5a5a5);
    memcpy(abCommand + 0, &uCommandType, sizeof(uCommandType));
    memcpy(abCommand + 8, &uCommandBuffer, sizeof(uCommandBuffer));
    memcpy(abCommand + 16, &uBuffer, sizeof(uBuffer));
    memcpy(abCommand + 32, &cbBuffer, sizeof(cbBuffer));
    memcpy(abCommand + 40, &uData, sizeof(uData));
    VIRTIOGPUFILLCMD Fill;
    RTTESTI_CHECK(virtioGpuR3DecodeFillBuffer(abCommand, sizeof(abCommand), &Fill)
                  && Fill.uCommandBuffer == uCommandBuffer && Fill.uBuffer == uBuffer
                  && Fill.cbBuffer == cbBuffer && Fill.uData == uData);
    abCommand[4] = 1;
    RTTESTI_CHECK(!virtioGpuR3DecodeFillBuffer(abCommand, sizeof(abCommand), &Fill));
    uint8_t abClearCommand[84] = { 0 };
    uint32_t uClearType = 119;
    uint64_t uClearCommandBuffer = 42;
    uint64_t uClearImage = 7;
    uint32_t uClearLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint64_t cbClearColor = 1;
    uint32_t auClearColor[4] = { 0, 0, 0, UINT32_C(0x3f800000) };
    uint32_t cClearRanges = 1;
    uint64_t cbClearRanges = 20;
    uint32_t auClearRange[5] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    memcpy(abClearCommand + 0, &uClearType, sizeof(uClearType));
    memcpy(abClearCommand + 8, &uClearCommandBuffer, sizeof(uClearCommandBuffer));
    memcpy(abClearCommand + 16, &uClearImage, sizeof(uClearImage));
    memcpy(abClearCommand + 24, &uClearLayout, sizeof(uClearLayout));
    memcpy(abClearCommand + 28, &cbClearColor, sizeof(cbClearColor));
    memcpy(abClearCommand + 36, auClearColor, sizeof(auClearColor));
    memcpy(abClearCommand + 52, &cClearRanges, sizeof(cClearRanges));
    memcpy(abClearCommand + 56, &cbClearRanges, sizeof(cbClearRanges));
    memcpy(abClearCommand + 64, auClearRange, sizeof(auClearRange));
    VIRTIOGPUCLEARCOLORCMD Clear;
    RTTESTI_CHECK(virtioGpuR3DecodeClearColorImage(abClearCommand, sizeof(abClearCommand), &Clear)
                  && Clear.uCommandBuffer == uClearCommandBuffer && Clear.uImage == uClearImage
                  && Clear.enmImageLayout == uClearLayout && Clear.cRanges == 1);
    uint8_t abCopyImageCommand[104] = { 0 };
    uint32_t uCopyImageType = 115;
    uint64_t uCopyImageCommandBuffer = 42;
    uint64_t uCopyImageSrc = 7;
    uint64_t uCopyImageDst = 7;
    uint32_t uCopyImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t cCopyImageRegions = 1;
    uint64_t cbCopyImageRegions = 56;
    uint32_t auCopyImageSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t auCopyImageExtent[3] = { 2, 2, 1 };
    memcpy(abCopyImageCommand + 0, &uCopyImageType, sizeof(uCopyImageType));
    memcpy(abCopyImageCommand + 8, &uCopyImageCommandBuffer, sizeof(uCopyImageCommandBuffer));
    memcpy(abCopyImageCommand + 16, &uCopyImageSrc, sizeof(uCopyImageSrc));
    memcpy(abCopyImageCommand + 24, &uCopyImageDst, sizeof(uCopyImageDst));
    memcpy(abCopyImageCommand + 32, &uCopyImageLayout, sizeof(uCopyImageLayout));
    memcpy(abCopyImageCommand + 36, &cCopyImageRegions, sizeof(cCopyImageRegions));
    memcpy(abCopyImageCommand + 40, &cbCopyImageRegions, sizeof(cbCopyImageRegions));
    memcpy(abCopyImageCommand + 64, auCopyImageSubresource, sizeof(auCopyImageSubresource));
    memcpy(abCopyImageCommand + 92, auCopyImageExtent, sizeof(auCopyImageExtent));
    VIRTIOGPUCOPYBUFFERTOIMAGECMD CopyBufferToImage;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyBufferToImage(abCopyImageCommand, sizeof(abCopyImageCommand),
                                                     &CopyBufferToImage)
                  && CopyBufferToImage.uCommandBuffer == uCopyImageCommandBuffer
                  && CopyBufferToImage.uSrcBuffer == uCopyImageSrc
                  && CopyBufferToImage.uDstImage == uCopyImageDst);
    uint8_t abImageToBufferCommand[104] = { 0 };
    uint32_t uImageToBufferType = 116;
    uint64_t uImageToBufferCommandBuffer = 42;
    uint64_t uImageToBufferSrc = 7;
    uint32_t uImageToBufferLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint64_t uImageToBufferDst = 7;
    memcpy(abImageToBufferCommand + 0, &uImageToBufferType, sizeof(uImageToBufferType));
    memcpy(abImageToBufferCommand + 8, &uImageToBufferCommandBuffer, sizeof(uImageToBufferCommandBuffer));
    memcpy(abImageToBufferCommand + 16, &uImageToBufferSrc, sizeof(uImageToBufferSrc));
    memcpy(abImageToBufferCommand + 24, &uImageToBufferLayout, sizeof(uImageToBufferLayout));
    memcpy(abImageToBufferCommand + 28, &uImageToBufferDst, sizeof(uImageToBufferDst));
    memcpy(abImageToBufferCommand + 36, &cCopyImageRegions, sizeof(cCopyImageRegions));
    memcpy(abImageToBufferCommand + 40, &cbCopyImageRegions, sizeof(cbCopyImageRegions));
    memcpy(abImageToBufferCommand + 64, auCopyImageSubresource, sizeof(auCopyImageSubresource));
    memcpy(abImageToBufferCommand + 92, auCopyImageExtent, sizeof(auCopyImageExtent));
    VIRTIOGPUCOPYIMAGETOBUFFERCMD CopyImageToBuffer;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyImageToBuffer(abImageToBufferCommand, sizeof(abImageToBufferCommand),
                                                     &CopyImageToBuffer)
                  && CopyImageToBuffer.uCommandBuffer == uImageToBufferCommandBuffer
                  && CopyImageToBuffer.uSrcImage == uImageToBufferSrc
                  && CopyImageToBuffer.uDstBuffer == uImageToBufferDst);
    uint8_t abBarrierCommand[128] = { 0 };
    uint32_t uBarrierType = 126;
    uint64_t uBarrierCommandBuffer = 42;
    uint32_t uBarrierSrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    uint32_t uBarrierDstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    uint32_t uBarrierDependency = 0;
    uint32_t cBarrierMemory = 0;
    uint64_t cbBarrierMemory = 0;
    uint32_t cBarrierBuffer = 0;
    uint64_t cbBarrierBuffer = 0;
    uint32_t cBarrierImage = 1;
    uint64_t cbBarrierImage = 64;
    uint32_t uBarrierSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    uint64_t cbBarrierNext = 0;
    uint32_t uBarrierSrcAccess = VK_ACCESS_TRANSFER_READ_BIT;
    uint32_t uBarrierDstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
    uint32_t uBarrierOldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint32_t uBarrierNewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t uBarrierQueue = VK_QUEUE_FAMILY_IGNORED;
    memcpy(abBarrierCommand + 0, &uBarrierType, sizeof(uBarrierType));
    memcpy(abBarrierCommand + 8, &uBarrierCommandBuffer, sizeof(uBarrierCommandBuffer));
    memcpy(abBarrierCommand + 16, &uBarrierSrcStage, sizeof(uBarrierSrcStage));
    memcpy(abBarrierCommand + 20, &uBarrierDstStage, sizeof(uBarrierDstStage));
    memcpy(abBarrierCommand + 24, &uBarrierDependency, sizeof(uBarrierDependency));
    memcpy(abBarrierCommand + 28, &cBarrierMemory, sizeof(cBarrierMemory));
    memcpy(abBarrierCommand + 32, &cbBarrierMemory, sizeof(cbBarrierMemory));
    memcpy(abBarrierCommand + 40, &cBarrierBuffer, sizeof(cBarrierBuffer));
    memcpy(abBarrierCommand + 44, &cbBarrierBuffer, sizeof(cbBarrierBuffer));
    memcpy(abBarrierCommand + 52, &cBarrierImage, sizeof(cBarrierImage));
    memcpy(abBarrierCommand + 56, &cbBarrierImage, sizeof(cbBarrierImage));
    memcpy(abBarrierCommand + 64, &uBarrierSType, sizeof(uBarrierSType));
    memcpy(abBarrierCommand + 68, &cbBarrierNext, sizeof(cbBarrierNext));
    memcpy(abBarrierCommand + 76, &uBarrierSrcAccess, sizeof(uBarrierSrcAccess));
    memcpy(abBarrierCommand + 80, &uBarrierDstAccess, sizeof(uBarrierDstAccess));
    memcpy(abBarrierCommand + 84, &uBarrierOldLayout, sizeof(uBarrierOldLayout));
    memcpy(abBarrierCommand + 88, &uBarrierNewLayout, sizeof(uBarrierNewLayout));
    memcpy(abBarrierCommand + 92, &uBarrierQueue, sizeof(uBarrierQueue));
    memcpy(abBarrierCommand + 96, &uBarrierQueue, sizeof(uBarrierQueue));
    memcpy(abBarrierCommand + 100, &uClearImage, sizeof(uClearImage));
    memcpy(abBarrierCommand + 108, auClearRange, sizeof(auClearRange));
    VIRTIOGPUPIPELINEBARRIERCMD Barrier;
    RTTESTI_CHECK(virtioGpuR3DecodePipelineBarrier(abBarrierCommand, sizeof(abBarrierCommand), &Barrier)
                  && Barrier.uCommandBuffer == uBarrierCommandBuffer && Barrier.cImageBarriers == 1);
    uint8_t abCopyImagesCommand[120] = { 0 };
    uint32_t uCopyImagesType = 113;
    uint64_t uCopyImagesCommandBuffer = 42;
    uint64_t uCopyImagesSrc = 7;
    uint32_t uCopyImagesSrcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint64_t uCopyImagesDst = 11;
    uint32_t uCopyImagesDstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t cCopyImagesRegions = 1;
    uint64_t cbCopyImagesRegions = 68;
    uint32_t auCopyImagesSrcSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t auCopyImagesDstSubresource[4] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t auCopyImagesExtent[3] = { 2, 2, 1 };
    memcpy(abCopyImagesCommand + 0, &uCopyImagesType, sizeof(uCopyImagesType));
    memcpy(abCopyImagesCommand + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abCopyImagesCommand + 16, &uCopyImagesSrc, sizeof(uCopyImagesSrc));
    memcpy(abCopyImagesCommand + 24, &uCopyImagesSrcLayout, sizeof(uCopyImagesSrcLayout));
    memcpy(abCopyImagesCommand + 28, &uCopyImagesDst, sizeof(uCopyImagesDst));
    memcpy(abCopyImagesCommand + 36, &uCopyImagesDstLayout, sizeof(uCopyImagesDstLayout));
    memcpy(abCopyImagesCommand + 40, &cCopyImagesRegions, sizeof(cCopyImagesRegions));
    memcpy(abCopyImagesCommand + 44, &cbCopyImagesRegions, sizeof(cbCopyImagesRegions));
    memcpy(abCopyImagesCommand + 52, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abCopyImagesCommand + 80, auCopyImagesDstSubresource, sizeof(auCopyImagesDstSubresource));
    memcpy(abCopyImagesCommand + 108, auCopyImagesExtent, sizeof(auCopyImagesExtent));
    VIRTIOGPUCOPYIMAGECMD CopyImages;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyImage(abCopyImagesCommand, sizeof(abCopyImagesCommand), &CopyImages)
                  && CopyImages.uCommandBuffer == uCopyImagesCommandBuffer
                  && CopyImages.uSrcImage == uCopyImagesSrc && CopyImages.uDstImage == uCopyImagesDst);
#endif
    Unref.id = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[0].fUsed && pGpu->cbAllocated == 64);
    Unref.id = 10;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    RTTESTI_CHECK(!pGpu->aResources[1].fUsed && pGpu->cbAllocated == 0);

    RTTestSub(g_hTest, "context create/destroy and saved state");
    struct { uint32_t cchName, fInit; char szName[64]; } ContextCreate = { 4, 0, "test" };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_CREATE, &ContextCreate, sizeof(ContextCreate), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && virtioGpuR3FindContext(pGpu, 42));
    Blob.uResourceId = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &Blob, sizeof(Blob), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    VIRTIOGPURESOURCECREATEBLOB BlobCtxCopy = Blob;
    BlobCtxCopy.uResourceId = 10;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &BlobCtxCopy, sizeof(BlobCtxCopy), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uint32_t uContextResource = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE, &uContextResource,
                   sizeof(uContextResource), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aContexts[0].cResources == 1);
    uint32_t uContextCopyResource = 10;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE, &uContextCopyResource,
                   sizeof(uContextCopyResource), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    RTTESTI_CHECK(pGpu->aContexts[0].cResources == 2);
    VIRTIOGPUSUBMIT3D Submit = { 0, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &Submit, sizeof(Submit), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[52]; } SubmitUpdate = { { 52, 1 }, 9, { 0 } };
    uint32_t uUpdateType = 117;
    uint64_t uUpdateCommandBuffer = 42;
    uint64_t uUpdateBuffer = 9;
    uint64_t offUpdate = 8;
    uint64_t cbUpdate = 4;
    uint64_t uUpdateArraySize = 4;
    uint32_t uUpdateData = UINT32_C(0x13579bdf);
    memcpy(SubmitUpdate.abCommand + 0, &uUpdateType, sizeof(uUpdateType));
    memcpy(SubmitUpdate.abCommand + 8, &uUpdateCommandBuffer, sizeof(uUpdateCommandBuffer));
    memcpy(SubmitUpdate.abCommand + 16, &uUpdateBuffer, sizeof(uUpdateBuffer));
    memcpy(SubmitUpdate.abCommand + 24, &offUpdate, sizeof(offUpdate));
    memcpy(SubmitUpdate.abCommand + 32, &cbUpdate, sizeof(cbUpdate));
    memcpy(SubmitUpdate.abCommand + 40, &uUpdateArraySize, sizeof(uUpdateArraySize));
    memcpy(SubmitUpdate.abCommand + 48, &uUpdateData, sizeof(uUpdateData));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitUpdate, sizeof(SubmitUpdate), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[0].pvVkMapped + 8) == uUpdateData
                  && *(uint32_t *)(pGpu->aResources[0].pbPixels + 8) == uUpdateData);
    RTTestSub(g_hTest, "Venus vkCmdUpdateBuffer batching");
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[104]; } SubmitUpdateBatch = { { 104, 1 }, 9, { 0 } };
    memcpy(SubmitUpdateBatch.abCommand, SubmitUpdate.abCommand, 52);
    memcpy(SubmitUpdateBatch.abCommand + 52, SubmitUpdate.abCommand, 52);
    uint64_t offUpdateSecond = 12;
    uint32_t uUpdateDataSecond = UINT32_C(0x2468ace0);
    memcpy(SubmitUpdateBatch.abCommand + 52 + 24, &offUpdateSecond, sizeof(offUpdateSecond));
    memcpy(SubmitUpdateBatch.abCommand + 52 + 48, &uUpdateDataSecond, sizeof(uUpdateDataSecond));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitUpdateBatch,
                   sizeof(SubmitUpdateBatch), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[0].pvVkMapped + 8) == uUpdateData
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[0].pvVkMapped + 12) == uUpdateDataSecond
                  && *(uint32_t *)(pGpu->aResources[0].pbPixels + 12) == uUpdateDataSecond);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[44]; } SubmitCommand = { { 44, 1 }, 9, { 0 } };
    memcpy(SubmitCommand.abCommand + 0, &uCommandType, sizeof(uCommandType));
    memcpy(SubmitCommand.abCommand + 8, &uCommandBuffer, sizeof(uCommandBuffer));
    memcpy(SubmitCommand.abCommand + 16, &uBuffer, sizeof(uBuffer));
    memcpy(SubmitCommand.abCommand + 32, &cbBuffer, sizeof(cbBuffer));
    memcpy(SubmitCommand.abCommand + 40, &uData, sizeof(uData));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCommand, sizeof(SubmitCommand), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[88]; } SubmitBatch = { { 88, 1 }, 9, { 0 } };
    memcpy(SubmitBatch.abCommand, SubmitCommand.abCommand, sizeof(SubmitCommand.abCommand));
    memcpy(SubmitBatch.abCommand + 44, SubmitCommand.abCommand, sizeof(SubmitCommand.abCommand));
    uint64_t offSecond = 4;
    uint32_t uDataSecond = UINT32_C(0xdecafbad);
    memcpy(SubmitBatch.abCommand + 44 + 24, &offSecond, sizeof(offSecond));
    memcpy(SubmitBatch.abCommand + 44 + 40, &uDataSecond, sizeof(uDataSecond));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitBatch, sizeof(SubmitBatch), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[0].pvVkMapped + 4) == uDataSecond
                  && *(uint32_t *)(pGpu->aResources[0].pbPixels + 4) == uDataSecond);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[92]; } SubmitCopyRegions = { { 92, 2 }, { 9, 10 }, { 0 } };
    uint32_t uCopyType = 112;
    uint64_t uCopyCommandBuffer = 42;
    uint64_t uCopySrc = 9;
    uint64_t uCopyDst = 10;
    uint32_t uCopyRegions = 2;
    uint64_t cbCopyRegions = 48;
    uint64_t cbCopy = 4;
    memcpy(SubmitCopyRegions.abCommand + 0, &uCopyType, sizeof(uCopyType));
    memcpy(SubmitCopyRegions.abCommand + 8, &uCopyCommandBuffer, sizeof(uCopyCommandBuffer));
    memcpy(SubmitCopyRegions.abCommand + 16, &uCopySrc, sizeof(uCopySrc));
    memcpy(SubmitCopyRegions.abCommand + 24, &uCopyDst, sizeof(uCopyDst));
    memcpy(SubmitCopyRegions.abCommand + 32, &uCopyRegions, sizeof(uCopyRegions));
    memcpy(SubmitCopyRegions.abCommand + 36, &cbCopyRegions, sizeof(cbCopyRegions));
    memcpy(SubmitCopyRegions.abCommand + 60, &cbCopy, sizeof(cbCopy));
    uint64_t offCopySrcSecond = 4;
    uint64_t offCopyDstSecond = 4;
    memcpy(SubmitCopyRegions.abCommand + 68, &offCopySrcSecond, sizeof(offCopySrcSecond));
    memcpy(SubmitCopyRegions.abCommand + 76, &offCopyDstSecond, sizeof(offCopyDstSecond));
    memcpy(SubmitCopyRegions.abCommand + 84, &cbCopy, sizeof(cbCopy));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyRegions, sizeof(SubmitCopyRegions), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[1].pvVkMapped + 0) == uData
                  && *(uint32_t *)((uint8_t *)pGpu->aResources[1].pvVkMapped + 4) == uDataSecond
                  && *(uint32_t *)(pGpu->aResources[1].pbPixels + 0) == uData
                  && *(uint32_t *)(pGpu->aResources[1].pbPixels + 4) == uDataSecond);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DETACH_RESOURCE, &uContextCopyResource,
                   sizeof(uContextCopyResource), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aContexts[0].cResources == 1);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DETACH_RESOURCE, &uContextResource,
                   sizeof(uContextResource), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aContexts[0].cResources == 0);
    Unref.id = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    Unref.id = 10;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_CREATE, &ContextCreate, sizeof(ContextCreate), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_DESTROY, NULL, 0, 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !virtioGpuR3FindContext(pGpu, 42));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_CREATE, &ContextCreate, sizeof(ContextCreate), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);

    RTTestSub(g_hTest, "short packets and unsupported commands");
    struct { uint32_t cbSend, cbReturn, uType, fFlags, cbUsed, uResponse; } aCases[] =
    {
        { 23, 24, 0x100, 0, 24, 0x1205 },
        { 24, 23, 0x100, 0, 0, 0 },
        { 24, 0,  0x100, 0, 0, 0 },
        { 24, 24, 0x100, 0, 24, 0x1205 },
        { 24, 24, 0xffff, 0, 24, 0x1200 },
        { 24, 408, 0x100, 2, 24, 0x1205 },
        { 0, 24, 0x100, 0, 24, 0x1205 }
    };
    for (unsigned i = 0; i < RT_ELEMENTS(aCases); ++i)
    {
        uint16_t const uBeforeShort = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
        tstPost(&pGpu->Virtio, 0, aCases[i].cbSend, aCases[i].cbReturn, aCases[i].uType, aCases[i].fFlags);
        virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
        RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBeforeShort) == aCases[i].cbUsed);
        if (aCases[i].cbUsed)
        {
            memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
            RTTESTI_CHECK(Resp.Hdr.uType == aCases[i].uResponse);
            RTTESTI_CHECK(Resp.Hdr.uFlags == 0 && Resp.Hdr.uFenceId == 0 && Resp.Hdr.uCtxId == 0);
        }
        RTTESTI_CHECK(g_abRam[0x5000 + aCases[i].cbUsed] == 0xa5);
    }

    RTTestSub(g_hTest, "cursor completion and 16-bit index wrap");
    tstPost(&pGpu->Virtio, 1, 56, 24, 0x300);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 1);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 1, 0) == 0);
    tstInitQueue(&pGpu->Virtio, 0, UINT16_MAX);
    tstPost(&pGpu->Virtio, 0, 24, 408);
    RTTESTI_CHECK(virtioCoreVirtqAvailBufCount(pDev, &pGpu->Virtio, 0) == 1);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, UINT16_MAX) == 408);

    RTTestSub(g_hTest, "cyclic descriptor rejected and reset recovers");
    tstPost(&pGpu->Virtio, 0, 24, 408);
    VIRTQ_DESC_T *pDesc = (VIRTQ_DESC_T *)&g_abRam[0x1000];
    pDesc[0].uDescIdxNext = 0;
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(pGpu->Virtio.fDeviceStatus & VIRTIO_STATUS_DEVICE_NEEDS_RESET);
    virtioGpuR3Reset(pDev);
    RTTESTI_CHECK(pGpu->Virtio.fDeviceStatus == 0 && pGpu->Config.fEventsRead == 0);
    RTTESTI_CHECK(pGpu->Virtio.aVirtqueues[0].uQueueSize == VIRTQ_SIZE);
    RTTESTI_CHECK(pGpu->Virtio.aVirtqueues[2].uQueueSize == 0);
    pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
    tstInitQueue(&pGpu->Virtio, 0);
    tstPost(&pGpu->Virtio, 0, 24, 408);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, 0) == 408);
    RTTESTI_CHECK(g_cMemoryErrors == 0);

    RTTestSub(g_hTest, "malformed descriptor bounds and address overflow");
    for (unsigned i = 0; i < 5; ++i)
    {
        virtioGpuR3Reset(pDev);
        pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
        tstInitQueue(&pGpu->Virtio, 0);
        tstPost(&pGpu->Virtio, 0, 24, 408);
        pDesc = (VIRTQ_DESC_T *)&g_abRam[0x1000];
        switch (i)
        {
            case 0: pDesc[0].uDescIdxNext = 8; break;
            case 1: pDesc[0].fFlags |= 4; break; /* indirect not negotiated */
            case 2: pDesc[0].GCPhysBuf = UINT64_MAX - 8; break;
            case 3: pDesc[0].cb = UINT32_MAX; break; /* chain length overflow */
            case 4: *(uint16_t *)&g_abRam[0x1104] = 8; break; /* invalid head */
        }
        virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
        RTTESTI_CHECK(pGpu->Virtio.fDeviceStatus & VIRTIO_STATUS_DEVICE_NEEDS_RESET);
        RTTESTI_CHECK(g_cMemoryErrors == 0);
    }

    RTTestSub(g_hTest, "fragmented buffers, empty segments and memory faults");
    virtioGpuR3Reset(pDev);
    pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
    tstInitQueue(&pGpu->Virtio, 0);
    tstPost(&pGpu->Virtio, 0, 24, 408, 0x100, VIRTIOGPU_FLAG_FENCE);
    pDesc = (VIRTQ_DESC_T *)&g_abRam[0x1000];
    pDesc[0].cb = 8;
    pDesc[1].GCPhysBuf = 0x4008; pDesc[1].cb = 0;
    pDesc[1].fFlags = VIRTQ_DESC_F_NEXT; pDesc[1].uDescIdxNext = 2;
    pDesc[2].GCPhysBuf = 0x4008; pDesc[2].cb = 16;
    pDesc[2].fFlags = VIRTQ_DESC_F_NEXT; pDesc[2].uDescIdxNext = 3;
    pDesc[3].GCPhysBuf = 0x5000; pDesc[3].cb = 16;
    pDesc[3].fFlags = VIRTQ_DESC_F_NEXT | VIRTQ_DESC_F_WRITE; pDesc[3].uDescIdxNext = 4;
    pDesc[4].GCPhysBuf = 0x5100; pDesc[4].cb = 0;
    pDesc[4].fFlags = VIRTQ_DESC_F_NEXT | VIRTQ_DESC_F_WRITE; pDesc[4].uDescIdxNext = 5;
    pDesc[5].GCPhysBuf = 0x5100; pDesc[5].cb = 392; pDesc[5].fFlags = VIRTQ_DESC_F_WRITE;
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, 0) == 408);
    memcpy(&Resp, &g_abRam[0x5000], 16);
    memcpy((uint8_t *)&Resp + 16, &g_abRam[0x5100], 392);
    RTTESTI_CHECK(Resp.Hdr.uType == 0x1101 && Resp.Hdr.uFenceId == UINT64_C(0xabcdef0123456789));
    RTTESTI_CHECK(g_abRam[0x5010] == 0xa5 && g_abRam[0x5100 + 392] == 0xa5);
    for (unsigned i = 0; i < 2; ++i)
    {
        tstInitQueue(&pGpu->Virtio, 0);
        tstPost(&pGpu->Virtio, 0, 24, 408);
        pDesc = (VIRTQ_DESC_T *)&g_abRam[0x1000];
        pDesc[i].GCPhysBuf = sizeof(g_abRam);
        virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
        if (i == 0)
        {
            RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, 0) == 24);
            RTTESTI_CHECK(*(uint32_t *)&g_abRam[0x5000] == 0x1205);
        }
        else
        {
            RTTESTI_CHECK(pGpu->Virtio.fDeviceStatus & VIRTIO_STATUS_DEVICE_NEEDS_RESET);
            RTTESTI_CHECK(pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow == 0);
        }
        RTTESTI_CHECK(g_cMemoryErrors == i + 1);
    }

    RTTestSub(g_hTest, "save/load pending queue and version/truncation rejection");
    virtioGpuR3Reset(pDev);
    pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
    pGpu->Config.fEventsRead = 1;
    pGpu->aContexts[0].fUsed = true;
    pGpu->aContexts[0].uContextId = 77;
    pGpu->aContexts[0].cchName = 4;
    memcpy(pGpu->aContexts[0].szName, "save", 5);
    tstInitQueue(&pGpu->Virtio, 0);
    tstInitQueue(&pGpu->Virtio, 1);
    tstPost(&pGpu->Virtio, 0, 24, 408);
    static TSTSSM Ssm;
    RT_ZERO(Ssm);
    PSSMHANDLE pSSM = (PSSMHANDLE)&Ssm;
    RTTESTI_CHECK_RC(virtioGpuR3SaveExec(pDev, pSSM), VINF_SUCCESS);
    virtioGpuR3Reset(pDev);
    Ssm.off = 0;
    RTTESTI_CHECK_RC(virtioGpuR3LoadExec(pDev, pSSM, VIRTIOGPU_SAVED_STATE_VERSION, SSM_PASS_FINAL), VINF_SUCCESS);
    RTTESTI_CHECK(Ssm.off == Ssm.cb && pGpu->Config.fEventsRead == 1);
    RTTESTI_CHECK(virtioGpuR3FindContext(pGpu, 77) && pGpu->aContexts[0].cchName == 4
                  && !memcmp(pGpu->aContexts[0].szName, "save", 4));
    RTTESTI_CHECK(pGpu->Virtio.aVirtqueues[0].fAttached && pGpu->Virtio.aVirtqueues[1].fAttached);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, 0) == 408);
    pGpu->enmBackend = VIRTIOGPU_BACKEND_SOFTWARE;
    Ssm.off = 0;
    RTTESTI_CHECK_RC(virtioGpuR3LoadExec(pDev, pSSM, VIRTIOGPU_SAVED_STATE_VERSION, SSM_PASS_FINAL),
                     VERR_SSM_LOAD_CONFIG_MISMATCH);
    pGpu->enmBackend = VIRTIOGPU_BACKEND_AUTO;
    RTTESTI_CHECK_RC(virtioGpuR3LoadExec(pDev, pSSM, 42, SSM_PASS_FINAL), VERR_SSM_UNSUPPORTED_DATA_UNIT_VERSION);
    Ssm.off = 0;
    Ssm.cb--;
    RTTESTI_CHECK_RC(virtioGpuR3LoadExec(pDev, pSSM, VIRTIOGPU_SAVED_STATE_VERSION, SSM_PASS_FINAL), VERR_BUFFER_UNDERFLOW);

    if (argc == 2)
    {
        RTTestSub(g_hTest, "registration from the built VBoxDD module");
        RTLDRMOD hMod;
        rc = RTLdrLoadEx(argv[1], &hMod, RTLDRLOAD_FLAGS_NT_SEARCH_DLL_LOAD_DIR, NULL);
        RTTESTI_CHECK_RC(rc, VINF_SUCCESS);
        if (RT_SUCCESS(rc))
        {
            void *pvRegister = NULL;
            rc = RTLdrGetSymbol(hMod, "VBoxDevicesRegister", &pvRegister);
            RTTESTI_CHECK_RC(rc, VINF_SUCCESS);
            if (RT_SUCCESS(rc))
            {
                PDMDEVREGCB Callbacks = { PDM_DEVREG_CB_VERSION, tstRegisterDevice };
                RTTESTI_CHECK_RC(((FNPDMVBOXDEVICESREGISTER *)pvRegister)(&Callbacks, VBOX_VERSION), VINF_SUCCESS);
                RTTESTI_CHECK(g_cGpuRegistrations == 1);
            }
            RTLdrClose(hMod);
        }
    }
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    virtioGpuR3VulkanTerm(pGpu);
#endif
    RTMemFree(pGpu);
    RTMemFree(pDev);
    return RTTestSummaryAndDestroy(g_hTest);
}
