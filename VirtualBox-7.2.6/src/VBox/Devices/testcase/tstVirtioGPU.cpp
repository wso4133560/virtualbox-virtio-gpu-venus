/* SPDX-License-Identifier: GPL-3.0-only */
/** Exercises the production transport and GPU callbacks using bounded guest RAM.
 * No host drivers, VM registration, or graphics stack is involved in this test. */
#include <iprt/test.h>
#include <iprt/mem.h>
#include <iprt/ldr.h>
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

    RTTestSub(g_hTest, "capset query without advertised capabilities");
    uint16_t uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
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
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &Flush, sizeof(Flush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uFlushSequence == 1);
    RTTESTI_CHECK(g_cDisplayUpdates == 1);

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
    VIRTIOGPURESOURCECREATEBLOB Blob = { 9, 0, 0, 0, UINT64_C(0x1234), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &Blob, sizeof(Blob), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[0].fUsed
                  && pGpu->aResources[0].fBlob && pGpu->aResources[0].cbPixels == 64);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(pGpu->aResources[0].fVulkanBuffer && pGpu->aResources[0].hVkBuffer != VK_NULL_HANDLE);
#endif
    RTTestSub(g_hTest, "Vulkan fill command execution and readback");
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK_RC(virtioGpuR3VulkanFillBuffer(pGpu, &pGpu->aResources[0], 0, 4, UINT32_C(0x5a5a5a5a)), VINF_SUCCESS);
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
#endif
    Unref.id = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref, sizeof(Unref), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[0].fUsed && pGpu->cbAllocated == 0);

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
    uint32_t uContextResource = 9;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE, &uContextResource,
                   sizeof(uContextResource), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aContexts[0].cResources == 1);
    VIRTIOGPUSUBMIT3D Submit = { 0, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &Submit, sizeof(Submit), 24, 42);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
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
        uint16_t uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
        tstPost(&pGpu->Virtio, 0, aCases[i].cbSend, aCases[i].cbReturn, aCases[i].uType, aCases[i].fFlags);
        virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
        RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == aCases[i].cbUsed);
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
