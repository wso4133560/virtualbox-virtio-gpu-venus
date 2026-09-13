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
static unsigned g_cCursorShapes;
static unsigned g_cCursorMoves;
static bool g_fCursorVisible;
static uint32_t g_cCursorWidth;
static uint32_t g_cCursorHeight;
static uint32_t g_uCursorHotX;
static uint32_t g_uCursorHotY;
static uint32_t g_uCursorScreen;
static int32_t g_iCursorX;
static int32_t g_iCursorY;
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

static DECLCALLBACK(int) tstDisplayCursorShape(PPDMIDISPLAYCONNECTOR pInterface, bool fVisible, bool fAlpha,
                                               uint32_t xHot, uint32_t yHot, uint32_t cx, uint32_t cy,
                                               const void *pvShape)
{
    RT_NOREF(pInterface, fAlpha, pvShape);
    g_cCursorShapes++;
    g_fCursorVisible = fVisible;
    g_uCursorHotX = xHot;
    g_uCursorHotY = yHot;
    g_cCursorWidth = cx;
    g_cCursorHeight = cy;
    return VINF_SUCCESS;
}

static DECLCALLBACK(void) tstDisplayCursorPosition(PPDMIDISPLAYCONNECTOR pInterface, uint32_t fFlags,
                                                   uint32_t uScreen, uint32_t x, uint32_t y)
{
    RT_NOREF(pInterface, fFlags);
    g_cCursorMoves++;
    g_uCursorScreen = uScreen;
    g_iCursorX = (int32_t)x;
    g_iCursorY = (int32_t)y;
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

static void tstPostCommandAt(PVIRTIOCORE pCore, unsigned uQueue, uint32_t uType,
                             const void *pvBody, size_t cbBody, uint32_t cbReturn,
                             uint32_t offSend, uint32_t offReturn, uint32_t uCtxId = 0)
{
    PVIRTQUEUE pQ = &pCore->aVirtqueues[uQueue];
    VIRTQ_DESC_T *pDesc = (VIRTQ_DESC_T *)&g_abRam[pQ->GCPhysVirtqDesc];
    pDesc[0].GCPhysBuf = offSend;
    pDesc[0].cb = (uint32_t)(sizeof(VIRTIOGPUCTRLHDR) + cbBody);
    pDesc[0].fFlags = VIRTQ_DESC_F_NEXT;
    pDesc[0].uDescIdxNext = 1;
    pDesc[1].GCPhysBuf = offReturn;
    pDesc[1].cb = cbReturn;
    pDesc[1].fFlags = VIRTQ_DESC_F_WRITE;
    VIRTIOGPUCTRLHDR Req;
    RT_ZERO(Req);
    Req.uType = uType;
    Req.uCtxId = uCtxId;
    memcpy(&g_abRam[offSend], &Req, sizeof(Req));
    if (cbBody)
        memcpy(&g_abRam[offSend + sizeof(Req)], pvBody, cbBody);
    memset(&g_abRam[offReturn], 0xa5, 1024);
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 4 + (pQ->uAvailIdxShadow % 8) * 2] = 0;
    *(uint16_t *)&g_abRam[pQ->GCPhysVirtqAvail + 2] = pQ->uAvailIdxShadow + 1;
}

static void tstPostCommand(PVIRTIOCORE pCore, unsigned uQueue, uint32_t uType,
                           const void *pvBody, size_t cbBody, uint32_t cbReturn, uint32_t uCtxId = 0)
{
    tstPostCommandAt(pCore, uQueue, uType, pvBody, cbBody, cbReturn, 0x4000, 0x5000, uCtxId);
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

#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
static PFN_vkGetInstanceProcAddr g_pfnCopyTestInstanceProc;
static PFN_vkGetDeviceProcAddr g_pfnCopyTestDeviceProc;
static unsigned g_cDroppedCopies;
static bool g_fDropBufferToImage;
static unsigned g_cDroppedBufferToImage;
static bool g_fCountImageToBuffer;
static unsigned g_cImageToBufferCalls;

/* Suppress only the copy recording. Submission, fences and mapped memory still
 * use the real GPU driver, so a CPU replacement copy cannot pass this test. */
static VKAPI_ATTR void VKAPI_CALL tstVkDropCopyBuffer(VkCommandBuffer hCmd, VkBuffer hSrc, VkBuffer hDst,
                                                     uint32_t cRegions, const VkBufferCopy *paRegions)
{
    RT_NOREF(hCmd, hSrc, hDst, cRegions, paRegions);
    ++g_cDroppedCopies;
}

static VKAPI_ATTR void VKAPI_CALL tstVkDropCopyBufferToImage(VkCommandBuffer hCmd, VkBuffer hSrc, VkImage hDst,
                                                             VkImageLayout enmLayout, uint32_t cRegions,
                                                             const VkBufferImageCopy *paRegions)
{
    RT_NOREF(hCmd, hSrc, hDst, enmLayout, cRegions, paRegions);
    ++g_cDroppedBufferToImage;
}

static VKAPI_ATTR void VKAPI_CALL tstVkCountCopyImageToBuffer(VkCommandBuffer hCmd, VkImage hSrc,
                                                              VkImageLayout enmLayout, VkBuffer hDst,
                                                              uint32_t cRegions, const VkBufferImageCopy *paRegions)
{
    ++g_cImageToBufferCalls;
    if (g_fCountImageToBuffer)
        return;
    PFN_vkCmdCopyImageToBuffer pfnCopy = (PFN_vkCmdCopyImageToBuffer)
        g_pfnCopyTestDeviceProc(VK_NULL_HANDLE, "vkCmdCopyImageToBuffer");
    if (pfnCopy)
        pfnCopy(hCmd, hSrc, enmLayout, hDst, cRegions, paRegions);
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL tstVkCopyDeviceProc(VkDevice hDevice, const char *pszName)
{
    if (!strcmp(pszName, "vkCmdCopyBuffer"))
        return (PFN_vkVoidFunction)tstVkDropCopyBuffer;
    if (g_fDropBufferToImage && !strcmp(pszName, "vkCmdCopyBufferToImage"))
        return (PFN_vkVoidFunction)tstVkDropCopyBufferToImage;
    if (g_fCountImageToBuffer && !strcmp(pszName, "vkCmdCopyImageToBuffer"))
        return (PFN_vkVoidFunction)tstVkCountCopyImageToBuffer;
    return g_pfnCopyTestDeviceProc(hDevice, pszName);
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL tstVkCopyInstanceProc(VkInstance hInstance, const char *pszName)
{
    if (!strcmp(pszName, "vkGetDeviceProcAddr"))
        return (PFN_vkVoidFunction)tstVkCopyDeviceProc;
    return g_pfnCopyTestInstanceProc(hInstance, pszName);
}

static void tstVulkanCopyReadback(PVIRTIOGPU pGpu)
{
    RTTestSub(g_hTest, "Vulkan RGBA8 resource format");
    uint8_t abRgba[16] = { 0 };
    VIRTIOGPURESOURCE Rgba;
    RT_ZERO(Rgba);
    Rgba.uFormat = VIRTIOGPU_FORMAT_R8G8B8A8_UNORM;
    Rgba.uWidth = Rgba.uHeight = 2;
    Rgba.cbPixels = sizeof(abRgba);
    Rgba.pbPixels = abRgba;
    int const rcRgba = virtioGpuR3VulkanResourceCreate(pGpu, &Rgba);
    RTTESTI_CHECK_RC(rcRgba, VINF_SUCCESS);
    RTTESTI_CHECK(RT_FAILURE(rcRgba) || (Rgba.fVulkanBuffer && Rgba.fVulkanImage));
    virtioGpuR3VulkanResourceDestroy(pGpu, &Rgba);

    RTTestSub(g_hTest, "Vulkan copy readback without CPU replacement");
    uint8_t abSrc[64], abDst[64], abExpected[64];
    for (unsigned i = 0; i < sizeof(abSrc); ++i)
        abSrc[i] = (uint8_t)(i * 3 + 7);
    memset(abDst, 0xc3, sizeof(abDst));
    VIRTIOGPURESOURCE Src, Dst;
    RT_ZERO(Src);
    RT_ZERO(Dst);
    Src.fBlob = Dst.fBlob = true;
    Src.cbPixels = sizeof(abSrc);
    Dst.cbPixels = sizeof(abDst);
    Src.pbPixels = abSrc;
    Dst.pbPixels = abDst;
    int rcSrc = virtioGpuR3VulkanResourceCreate(pGpu, &Src);
    int rcDst = virtioGpuR3VulkanResourceCreate(pGpu, &Dst);
    RTTESTI_CHECK_RC(rcSrc, VINF_SUCCESS);
    RTTESTI_CHECK_RC(rcDst, VINF_SUCCESS);
    if (RT_SUCCESS(rcSrc) && RT_SUCCESS(rcDst) && Src.fVulkanBuffer && Dst.fVulkanBuffer)
    {
        g_pfnCopyTestInstanceProc = pGpu->pfnVkGetInstanceProcAddr;
        g_pfnCopyTestDeviceProc = (PFN_vkGetDeviceProcAddr)
            g_pfnCopyTestInstanceProc(pGpu->hVkInstance, "vkGetDeviceProcAddr");
        RTTESTI_CHECK(g_pfnCopyTestDeviceProc != NULL);
        if (g_pfnCopyTestDeviceProc)
        {
            uint64_t const aRegions[2][3] = { { 4, 8, 16 }, { 32, 40, 8 } };
            VIRTIOGPUCOPYCMD aCopy[2];
            RT_ZERO(aCopy);
            for (unsigned i = 0; i < RT_ELEMENTS(aCopy); ++i)
            {
                aCopy[i].offSrc = aRegions[i][0];
                aCopy[i].offDst = aRegions[i][1];
                aCopy[i].cbCopy = aRegions[i][2];
            }
            VIRTIOGPUCOPYCMD Multi;
            RT_ZERO(Multi);
            Multi.cRegions = 2;
            Multi.pbRegions = (const uint8_t *)aRegions;
            for (unsigned iMode = 0; iMode < 3; ++iMode)
                for (unsigned fDrop = 0; fDrop < 2; ++fDrop)
                {
                    memset(abDst, 0xc3, sizeof(abDst));
                    memset(abExpected, 0xc3, sizeof(abExpected));
                    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceSync(pGpu, &Dst), VINF_SUCCESS);
                    if (!fDrop)
                        for (unsigned i = 0; i < (iMode ? 2U : 1U); ++i)
                            memcpy(abExpected + aRegions[i][1], abSrc + aRegions[i][0], (size_t)aRegions[i][2]);
                    g_cDroppedCopies = 0;
                    if (fDrop)
                        pGpu->pfnVkGetInstanceProcAddr = tstVkCopyInstanceProc;
                    int const rc = iMode == 0 ? virtioGpuR3VulkanCopyBuffer(pGpu, &Src, &Dst, 4, 8, 16)
                                 : iMode == 1 ? virtioGpuR3VulkanCopyBufferBatch(pGpu, &Src, &Dst, aCopy, 2)
                                 : virtioGpuR3VulkanCopyBufferRegions(pGpu, &Src, &Dst, &Multi);
                    pGpu->pfnVkGetInstanceProcAddr = g_pfnCopyTestInstanceProc;
                    RTTESTI_CHECK_RC(rc, VINF_SUCCESS);
                    RTTESTI_CHECK(g_cDroppedCopies == (fDrop ? (iMode == 1 ? 2U : 1U) : 0U));
                    RTTESTI_CHECK(memcmp(Dst.pvVkMapped, abExpected, sizeof(abExpected)) == 0);
                    RTTESTI_CHECK(memcmp(abDst, abExpected, sizeof(abExpected)) == 0);
        RTTESTI_CHECK(memcmp(Src.pvVkMapped, abSrc, sizeof(abSrc)) == 0);
                }
            RTTestSub(g_hTest, "Vulkan persistent buffer transfer throughput");
            VIRTIOGPUCOPYCMD aThroughputCopy[8];
            RT_ZERO(aThroughputCopy);
            for (unsigned i = 0; i < RT_ELEMENTS(aThroughputCopy); ++i)
            {
                aThroughputCopy[i].offSrc = 0;
                aThroughputCopy[i].offDst = 0;
                aThroughputCopy[i].cbCopy = sizeof(abSrc);
            }
            uint64_t const tsSingleStart = RTTimeNanoTS();
            int rcThroughput = VINF_SUCCESS;
            for (unsigned i = 0; i < RT_ELEMENTS(aThroughputCopy) && RT_SUCCESS(rcThroughput); ++i)
                rcThroughput = virtioGpuR3VulkanCopyBuffer(pGpu, &Src, &Dst, 0, 0, sizeof(abSrc));
            uint64_t const tsSingleNs = RTTimeNanoTS() - tsSingleStart;
            uint64_t const tsBatchStart = RTTimeNanoTS();
            if (RT_SUCCESS(rcThroughput))
                rcThroughput = virtioGpuR3VulkanCopyBufferBatch(pGpu, &Src, &Dst,
                                                                 aThroughputCopy, RT_ELEMENTS(aThroughputCopy));
            uint64_t const tsBatchNs = RTTimeNanoTS() - tsBatchStart;
            RTTESTI_CHECK_RC(rcThroughput, VINF_SUCCESS);
            RTTestIPrintf(RTTESTLVL_ALWAYS, "persistent Vulkan buffer copy: single8=%llu ns, batch8=%llu ns (%llu ns/copy)\n",
                          (unsigned long long)tsSingleNs, (unsigned long long)tsBatchNs,
                          (unsigned long long)(tsBatchNs / RT_ELEMENTS(aThroughputCopy)));
            VIRTIOGPUCOPYCMD aThroughputCopy64[64];
            RT_ZERO(aThroughputCopy64);
            for (unsigned i = 0; i < RT_ELEMENTS(aThroughputCopy64); ++i)
            {
                aThroughputCopy64[i].offSrc = 0;
                aThroughputCopy64[i].offDst = 0;
                aThroughputCopy64[i].cbCopy = sizeof(abSrc);
            }
            uint64_t const tsSingle64Start = RTTimeNanoTS();
            int rcThroughput64 = VINF_SUCCESS;
            for (unsigned i = 0; i < RT_ELEMENTS(aThroughputCopy64) && RT_SUCCESS(rcThroughput64); ++i)
                rcThroughput64 = virtioGpuR3VulkanCopyBuffer(pGpu, &Src, &Dst, 0, 0, sizeof(abSrc));
            uint64_t const tsSingle64Ns = RTTimeNanoTS() - tsSingle64Start;
            uint64_t const tsBatch64Start = RTTimeNanoTS();
            if (RT_SUCCESS(rcThroughput64))
                rcThroughput64 = virtioGpuR3VulkanCopyBufferBatch(pGpu, &Src, &Dst,
                                                                    aThroughputCopy64,
                                                                    RT_ELEMENTS(aThroughputCopy64));
            uint64_t const tsBatch64Ns = RTTimeNanoTS() - tsBatch64Start;
            RTTESTI_CHECK_RC(rcThroughput64, VINF_SUCCESS);
            RTTestIPrintf(RTTESTLVL_ALWAYS, "persistent Vulkan buffer copy64: single64=%llu ns, batch64=%llu ns (%llu ns/copy)\n",
                          (unsigned long long)tsSingle64Ns, (unsigned long long)tsBatch64Ns,
                          (unsigned long long)(tsBatch64Ns / RT_ELEMENTS(aThroughputCopy64)));
        }
    }
    else
        RTTestFailed(g_hTest, "GPU buffer backing required for copy readback test");

    /* buffer -> image must also leave the staging buffer untouched when the
       GPU copy is suppressed; the old CPU replacement would fail this. */
    uint8_t abImageSrc[16], abImageDst[16], abImageRegion[56];
    memset(abImageSrc, 0x17, sizeof(abImageSrc));
    memset(abImageDst, 0xc3, sizeof(abImageDst));
    memset(abImageRegion, 0, sizeof(abImageRegion));
    uint32_t const uAspect = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t const uLayerCount = 1;
    uint32_t const auExtent[3] = { 2, 2, 1 };
    memcpy(abImageRegion + 16, &uAspect, sizeof(uAspect));
    memcpy(abImageRegion + 28, &uLayerCount, sizeof(uLayerCount));
    memcpy(abImageRegion + 44, auExtent, sizeof(auExtent));
    VIRTIOGPURESOURCE ImageSrc, ImageDst;
    RT_ZERO(ImageSrc);
    RT_ZERO(ImageDst);
    ImageSrc.fBlob = true;
    ImageSrc.cbPixels = sizeof(abImageSrc);
    ImageSrc.pbPixels = abImageSrc;
    ImageDst.cbPixels = sizeof(abImageDst);
    ImageDst.pbPixels = abImageDst;
    ImageDst.uWidth = ImageDst.uHeight = 2;
    int const rcImageSrc = virtioGpuR3VulkanResourceCreate(pGpu, &ImageSrc);
    int const rcImageDst = virtioGpuR3VulkanResourceCreate(pGpu, &ImageDst);
    RTTESTI_CHECK_RC(rcImageSrc, VINF_SUCCESS);
    RTTESTI_CHECK_RC(rcImageDst, VINF_SUCCESS);
    if (RT_SUCCESS(rcImageSrc) && RT_SUCCESS(rcImageDst) && ImageSrc.fVulkanBuffer && ImageDst.fVulkanImage)
    {
        VIRTIOGPUCOPYBUFFERTOIMAGECMD CopyToImage;
        RT_ZERO(CopyToImage);
        CopyToImage.uCommandBuffer = 42;
        CopyToImage.uSrcBuffer = 100;
        CopyToImage.uDstImage = 101;
        CopyToImage.enmDstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        CopyToImage.cRegions = 1;
        CopyToImage.cbRegions = sizeof(abImageRegion);
        CopyToImage.pbRegions = abImageRegion;
        g_cDroppedBufferToImage = 0;
        g_fDropBufferToImage = true;
        pGpu->pfnVkGetInstanceProcAddr = tstVkCopyInstanceProc;
        int const rcCopyToImage = virtioGpuR3VulkanCopyBufferToImage(pGpu, &ImageSrc, &ImageDst, &CopyToImage);
        pGpu->pfnVkGetInstanceProcAddr = g_pfnCopyTestInstanceProc;
        g_fDropBufferToImage = false;
        RTTESTI_CHECK_RC(rcCopyToImage, VINF_SUCCESS);
        RTTESTI_CHECK(g_cDroppedBufferToImage == 1 && ImageDst.fVulkanImageDirty);
        RTTESTI_CHECK(memcmp(ImageDst.pvVkMapped, abImageDst, sizeof(abImageDst)) == 0);
        RTTESTI_CHECK(memcmp(ImageDst.pbPixels, abImageDst, sizeof(abImageDst)) == 0);

        RTTestSub(g_hTest, "Venus CopyBufferToImage batch submission");
        /* Two serialized uploads for the same image are recorded as one host
         * copy call.  The interception proves the batching path is selected
         * without relying on a CPU-side replacement. */
        VIRTIOGPUCOPYBUFFERTOIMAGECMD aCopyToImage[2];
        RT_ZERO(aCopyToImage);
        for (unsigned i = 0; i < RT_ELEMENTS(aCopyToImage); ++i)
        {
            aCopyToImage[i] = CopyToImage;
            aCopyToImage[i].uSrcBuffer = 100;
            aCopyToImage[i].uDstImage = 101;
        }
        g_cDroppedBufferToImage = 0;
        g_fDropBufferToImage = true;
        pGpu->pfnVkGetInstanceProcAddr = tstVkCopyInstanceProc;
        int const rcCopyToImageBatch = virtioGpuR3VulkanCopyBufferToImageBatch(pGpu, &ImageSrc, &ImageDst,
                                                                                 aCopyToImage, RT_ELEMENTS(aCopyToImage));
        pGpu->pfnVkGetInstanceProcAddr = g_pfnCopyTestInstanceProc;
        g_fDropBufferToImage = false;
        RTTESTI_CHECK_RC(rcCopyToImageBatch, VINF_SUCCESS);
        RTTESTI_CHECK(g_cDroppedBufferToImage == 1 && ImageDst.fVulkanImageDirty);

        RTTestSub(g_hTest, "Venus CopyImageToBuffer batch submission");
        VIRTIOGPUCOPYIMAGETOBUFFERCMD aCopyImageToBuffer[2];
        RT_ZERO(aCopyImageToBuffer);
        for (unsigned i = 0; i < RT_ELEMENTS(aCopyImageToBuffer); ++i)
        {
            aCopyImageToBuffer[i].uCommandBuffer = 43;
            aCopyImageToBuffer[i].uSrcImage = 101;
            aCopyImageToBuffer[i].uDstBuffer = 100;
            aCopyImageToBuffer[i].enmSrcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            aCopyImageToBuffer[i].cRegions = 1;
            aCopyImageToBuffer[i].cbRegions = sizeof(abImageRegion);
            aCopyImageToBuffer[i].pbRegions = abImageRegion;
        }
        ImageDst.enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        int const rcCopyImageToBufferBatch = virtioGpuR3VulkanCopyImageToBufferBatch(pGpu, &ImageDst, &ImageSrc,
                                                                                       aCopyImageToBuffer,
                                                                                       RT_ELEMENTS(aCopyImageToBuffer));
        RTTESTI_CHECK_RC(rcCopyImageToBufferBatch, VINF_SUCCESS);

        RTTestSub(g_hTest, "Venus PipelineBarrier2 execution");
        uint8_t abBarrier2Exec[88] = { 0 };
        uint32_t const uBarrier2ExecSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        uint64_t const uBarrier2ExecStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        uint64_t const uBarrier2ExecAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
        uint32_t const uBarrier2ExecOld = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        uint32_t const uBarrier2ExecNew = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        uint32_t const uBarrier2ExecQueue = VK_QUEUE_FAMILY_IGNORED;
        uint64_t const uBarrier2ExecImage = 101;
        memcpy(abBarrier2Exec + 0, &uBarrier2ExecSType, sizeof(uBarrier2ExecSType));
        memcpy(abBarrier2Exec + 12, &uBarrier2ExecStage, sizeof(uBarrier2ExecStage));
        memcpy(abBarrier2Exec + 20, &uBarrier2ExecAccess, sizeof(uBarrier2ExecAccess));
        memcpy(abBarrier2Exec + 28, &uBarrier2ExecStage, sizeof(uBarrier2ExecStage));
        memcpy(abBarrier2Exec + 36, &uBarrier2ExecAccess, sizeof(uBarrier2ExecAccess));
        memcpy(abBarrier2Exec + 44, &uBarrier2ExecOld, sizeof(uBarrier2ExecOld));
        memcpy(abBarrier2Exec + 48, &uBarrier2ExecNew, sizeof(uBarrier2ExecNew));
        memcpy(abBarrier2Exec + 52, &uBarrier2ExecQueue, sizeof(uBarrier2ExecQueue));
        memcpy(abBarrier2Exec + 56, &uBarrier2ExecQueue, sizeof(uBarrier2ExecQueue));
        memcpy(abBarrier2Exec + 60, &uBarrier2ExecImage, sizeof(uBarrier2ExecImage));
        uint32_t const auBarrier2ExecRange[5] = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        memcpy(abBarrier2Exec + 68, auBarrier2ExecRange, sizeof(auBarrier2ExecRange));
        VIRTIOGPUPIPELINEBARRIERCMD Barrier2Exec;
        RT_ZERO(Barrier2Exec);
        Barrier2Exec.uCommandBuffer = 44;
        Barrier2Exec.fSrcStage = (uint32_t)uBarrier2ExecStage;
        Barrier2Exec.fDstStage = (uint32_t)uBarrier2ExecStage;
        Barrier2Exec.fModern = true;
        Barrier2Exec.cImageBarriers = 1;
        Barrier2Exec.pbImageBarrier = abBarrier2Exec;
        ImageDst.uResourceId = 101;
        int const rcBarrier2Exec = virtioGpuR3VulkanPipelineBarrier(pGpu, &ImageDst, &Barrier2Exec);
        RTTESTI_CHECK_RC(rcBarrier2Exec, VINF_SUCCESS);
        RTTestSub(g_hTest, "Venus PipelineBarrier2 batch submission");
        VIRTIOGPUPIPELINEBARRIERCMD aBarrier2Batch[2] = { Barrier2Exec, Barrier2Exec };
        ImageDst.enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        int const rcBarrier2Batch = virtioGpuR3VulkanPipelineBarrierBatch(pGpu, &ImageDst, aBarrier2Batch,
                                                                           RT_ELEMENTS(aBarrier2Batch));
        RTTESTI_CHECK_RC(rcBarrier2Batch, VINF_SUCCESS);
    }
    else
        RTTestFailed(g_hTest, "GPU image backing required for buffer-to-image test");
    virtioGpuR3VulkanResourceDestroy(pGpu, &ImageDst);
    virtioGpuR3VulkanResourceDestroy(pGpu, &ImageSrc);
    virtioGpuR3VulkanResourceDestroy(pGpu, &Dst);
    virtioGpuR3VulkanResourceDestroy(pGpu, &Src);
}
#endif

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
    tstVulkanCopyReadback(pGpu);
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
    DisplayConnector.pfnVBVAMousePointerShape = tstDisplayCursorShape;
    DisplayConnector.pfnVBVAReportCursorPosition = tstDisplayCursorPosition;
    pCC->pDrv = &DisplayConnector;
    pCC->Virtio.pfnStatusChanged = virtioGpuR3StatusChanged;
    pCC->Virtio.pfnVirtqNotified = virtioGpuR3VirtqNotified;
    pGpu->Virtio.pDevInsR3 = pDev;
    pGpu->Virtio.cVirtqs = VIRTIOGPU_QUEUE_COUNT;
    pGpu->Virtio.fDeviceStatus = VIRTIO_STATUS_DRIVER_OK;
    pGpu->Virtio.uDeviceFeatures = VIRTIO_F_VERSION_1 | VIRTIOGPU_F_EDID | VIRTIOGPU_F_RESOURCE_UUID
                                 | VIRTIOGPU_F_RESOURCE_BLOB | VIRTIOGPU_F_CONTEXT_INIT;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    pGpu->Virtio.uDeviceFeatures |= VIRTIOGPU_F_VIRGL;
    RTTESTI_CHECK((pGpu->Virtio.uDeviceFeatures & (VIRTIOGPU_F_VIRGL | VIRTIOGPU_F_RESOURCE_UUID))
                  == (VIRTIOGPU_F_VIRGL | VIRTIOGPU_F_RESOURCE_UUID));
#endif
    pGpu->Config.cScanouts = 1;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    pGpu->enmActiveBackend = VIRTIOGPU_BACKEND_VENUS;
    pGpu->Config.cCapsets = 1;
#endif
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
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(auConfig[0] == 0 && auConfig[1] == 0 && auConfig[2] == 1 && auConfig[3] == 1);
#else
    RTTESTI_CHECK(auConfig[0] == 0 && auConfig[1] == 0 && auConfig[2] == 1 && auConfig[3] == 0);
#endif
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
    uint8_t uEdidChecksum = 0;
    for (unsigned i = 0; i < 128; ++i)
        uEdidChecksum = (uint8_t)(uEdidChecksum + EdidResp.abEdid[i]);
    RTTESTI_CHECK(EdidResp.Hdr.uType == VIRTIOGPU_RESP_OK_EDID && EdidResp.cbEdid == 128
                  && EdidResp.uPadding == 0 && uEdidChecksum == 0
                  && EdidResp.abEdid[54] == 0x64 && EdidResp.abEdid[55] == 0x19);
    VIRTIOGPUGETEDID GetEdidInvalid = { VIRTIOGPU_MAX_SCANOUTS, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_EDID, &GetEdidInvalid, sizeof(GetEdidInvalid),
                   sizeof(VIRTIOGPURESPEDID));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);

    RTTestSub(g_hTest, "capset query and Venus capability negotiation");
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    VIRTIOGPUCAPSETINFO CapsetInfo = { 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET_INFO, &CapsetInfo, sizeof(CapsetInfo), 36);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPUCAPSETINFORESP));
#else
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
#endif
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    VIRTIOGPUCAPSETINFORESP CapsetInfoResp;
    memcpy(&CapsetInfoResp, &g_abRam[0x5000], sizeof(CapsetInfoResp));
    RTTESTI_CHECK(CapsetInfoResp.Hdr.uType == VIRTIOGPU_RESP_OK_CAPSET_INFO
                  && CapsetInfoResp.uCapsetId == VIRTIOGPU_CAPSET_VENUS_ID
                  && CapsetInfoResp.uMaxVersion == VIRTIOGPU_CAPSET_VENUS_VERSION
                  && CapsetInfoResp.cbMaxSize == VIRTIOGPU_CAPSET_VENUS_SIZE);
    VIRTIOGPUGETCAPSET GetCapset = { VIRTIOGPU_CAPSET_VENUS_ID, VIRTIOGPU_CAPSET_VENUS_VERSION };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET, &GetCapset, sizeof(GetCapset),
                   sizeof(VIRTIOGPURESPCAPSETVENUS));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPURESPCAPSETVENUS));
    VIRTIOGPURESPCAPSETVENUS CapsetResp;
    memcpy(&CapsetResp, &g_abRam[0x5000], sizeof(CapsetResp));
    RTTESTI_CHECK(CapsetResp.Hdr.uType == VIRTIOGPU_RESP_OK_CAPSET
                  && CapsetResp.Capset.uWireFormatVersion == 1
                  && CapsetResp.Capset.uVkXmlVersion == UINT32_C(0x0040310d)
                  && CapsetResp.Capset.uVkExtCommandSerializationSpecVersion == 1
                  && CapsetResp.Capset.uVkMesaVenusProtocolSpecVersion == 1
                  && CapsetResp.Capset.fSupportsBlobId0 == 1
                  && CapsetResp.Capset.fUseGuestVram == 1
                  && CapsetResp.Capset.auVkExtensionMask1[0] == 1
                  && (CapsetResp.Capset.auVkExtensionMask1[315 / 32] & (UINT32_C(1) << (315 % 32)))
                  && (CapsetResp.Capset.auVkExtensionMask1[338 / 32] & (UINT32_C(1) << (338 % 32))));
    GetCapset.uCapsetVersion = 1;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET, &GetCapset, sizeof(GetCapset), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
#else
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
    VIRTIOGPUGETCAPSET GetCapset = { 4, 1 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_GET_CAPSET, &GetCapset, sizeof(GetCapset), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER);
#endif

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

    RTTestSub(g_hTest, "R8G8B8A8 resource control path");
    struct { uint32_t id, format, width, height; } CreateRgba =
        { 12, VIRTIOGPU_FORMAT_R8G8B8A8_UNORM, 2, 2 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_2D, &CreateRgba, sizeof(CreateRgba), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[1].fUsed
                  && pGpu->aResources[1].uFormat == VIRTIOGPU_FORMAT_R8G8B8A8_UNORM);
    struct { uint32_t id, padding; } UnrefRgba = { 12, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &UnrefRgba, sizeof(UnrefRgba), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[1].fUsed);

    RTTestSub(g_hTest, "3D resource create protocol and Vulkan backing");
    VIRTIOGPURESOURCECREATE3D Create3D;
    RT_ZERO(Create3D);
    Create3D.uResourceId = 13;
    Create3D.uTarget = VIRTIOGPU_TARGET_2D;
    Create3D.uFormat = VIRTIOGPU_FORMAT_R8G8B8A8_UNORM;
    Create3D.uWidth = 2;
    Create3D.uHeight = 2;
    Create3D.uDepth = 1;
    Create3D.uArraySize = 1;
    Create3D.uLastLevel = 0;
    Create3D.cSamples = 1;
    Create3D.fFlags = VIRTIOGPU_RESOURCE_FLAG_Y_0_TOP;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_3D,
                   &Create3D.uResourceId, sizeof(Create3D) - sizeof(Create3D.Hdr), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aResources[1].fUsed
                  && pGpu->aResources[1].uResourceId == 13
                  && pGpu->aResources[1].uFormat == VIRTIOGPU_FORMAT_R8G8B8A8_UNORM
                  && pGpu->aResources[1].uWidth == 2 && pGpu->aResources[1].uHeight == 2);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTTESTI_CHECK(pGpu->aResources[1].fVulkanImage && pGpu->aResources[1].hVkImage != VK_NULL_HANDLE);
#endif
    struct { uint32_t id, padding; } Unref3D = { 13, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &Unref3D, sizeof(Unref3D), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[1].fUsed);

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
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uResourceId == 7
                  && (pGpu->Config.fEventsRead & VIRTIOGPU_EVENT_DISPLAY) != 0);
    RTTESTI_CHECK(g_cDisplayResizes == 1);

    RTTestSub(g_hTest, "cursor queue shape, move and hide");
    struct { VIRTIOGPUCURSORPOS Pos; uint32_t uResourceId, uHotX, uHotY; } CursorShape =
        { { 0, 0, 0, 0 }, 7, 1, 1 };
    uint16_t uCursorBefore = pGpu->Virtio.aVirtqueues[VIRTIOGPU_QUEUE_CURSOR].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, VIRTIOGPU_CMD_UPDATE_CURSOR,
                   &CursorShape.Pos, sizeof(CursorShape), sizeof(VIRTIOGPUCTRLHDR));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, uCursorBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && g_cCursorShapes == 1
                  && g_fCursorVisible && g_cCursorWidth == 2 && g_cCursorHeight == 2
                  && g_uCursorHotX == 1 && g_uCursorHotY == 1);

    VIRTIOGPUCURSORPOS CursorPos = { 0, 123, -7, 0 };
    uCursorBefore = pGpu->Virtio.aVirtqueues[VIRTIOGPU_QUEUE_CURSOR].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, VIRTIOGPU_CMD_MOVE_CURSOR,
                   &CursorPos, sizeof(CursorPos), sizeof(VIRTIOGPUCTRLHDR));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, uCursorBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && g_cCursorMoves == 1
                  && g_uCursorScreen == 0 && g_iCursorX == 123 && g_iCursorY == -7);

    struct { VIRTIOGPUCURSORPOS Pos; uint32_t uResourceId, uHotX, uHotY; } CursorHide =
        { { 0, 0, 0, 0 }, 0, 0, 0 };
    uCursorBefore = pGpu->Virtio.aVirtqueues[VIRTIOGPU_QUEUE_CURSOR].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, VIRTIOGPU_CMD_UPDATE_CURSOR,
                   &CursorHide, sizeof(CursorHide), sizeof(VIRTIOGPUCTRLHDR));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, VIRTIOGPU_QUEUE_CURSOR, uCursorBefore) == sizeof(VIRTIOGPUCTRLHDR));
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && g_cCursorShapes == 2 && !g_fCursorVisible);

    struct { uint32_t x, y, w, h, id, padding; } Flush = { 0, 0, 2, 2, 7, 0 };
    /* RESOURCE_FLUSH must expose image contents back to the display shadow. */
    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    pGpu->aResources[0].fVulkanImageDirty = true;
#endif
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
    pGpu->aResources[0].fVulkanImageDirty = true;
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
    RTTestSub(g_hTest, "Venus ClearColorImage batch submission");
    uint8_t abClearBatchCommand[168] = { 0 };
    memcpy(abClearBatchCommand, abClearSubmitCommand, sizeof(abClearSubmitCommand));
    memcpy(abClearBatchCommand + sizeof(abClearSubmitCommand), abClearSubmitCommand,
           sizeof(abClearSubmitCommand));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[168]; }
        SubmitClearBatch = { { 168, 1 }, 7, { 0 } };
    memcpy(SubmitClearBatch.abCommand, abClearBatchCommand, sizeof(abClearBatchCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitClearBatch,
                   sizeof(SubmitClearBatch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    RTTestSub(g_hTest, "Venus vkCmdCopyBuffer2 serialization");
    uint8_t abCopyBuffer2Command[100] = { 0 };
    uint32_t const uCopyBuffer2Type = 207;
    uint64_t const uCopyBuffer2CommandBuffer = 43;
    uint64_t const uCopyBuffer2InfoPtr = 1;
    uint32_t const uCopyBuffer2InfoType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
    uint64_t const uCopyBuffer2Next = 0;
    uint64_t const uCopyBuffer2Src = 7;
    uint64_t const uCopyBuffer2Dst = 11;
    uint32_t const cCopyBuffer2Regions = 1;
    uint64_t const cbCopyBuffer2Array = 1;
    uint32_t const uCopyBuffer2RegionType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
    uint64_t const uCopyBuffer2RegionNext = 0;
    uint64_t const offCopyBuffer2Src = 0;
    uint64_t const offCopyBuffer2Dst = 0;
    uint64_t const cbCopyBuffer2 = sizeof(abPixels);
    memcpy(abCopyBuffer2Command + 0, &uCopyBuffer2Type, sizeof(uCopyBuffer2Type));
    memcpy(abCopyBuffer2Command + 8, &uCopyBuffer2CommandBuffer, sizeof(uCopyBuffer2CommandBuffer));
    memcpy(abCopyBuffer2Command + 16, &uCopyBuffer2InfoPtr, sizeof(uCopyBuffer2InfoPtr));
    memcpy(abCopyBuffer2Command + 24, &uCopyBuffer2InfoType, sizeof(uCopyBuffer2InfoType));
    memcpy(abCopyBuffer2Command + 28, &uCopyBuffer2Next, sizeof(uCopyBuffer2Next));
    memcpy(abCopyBuffer2Command + 36, &uCopyBuffer2Src, sizeof(uCopyBuffer2Src));
    memcpy(abCopyBuffer2Command + 44, &uCopyBuffer2Dst, sizeof(uCopyBuffer2Dst));
    memcpy(abCopyBuffer2Command + 52, &cCopyBuffer2Regions, sizeof(cCopyBuffer2Regions));
    memcpy(abCopyBuffer2Command + 56, &cbCopyBuffer2Array, sizeof(cbCopyBuffer2Array));
    memcpy(abCopyBuffer2Command + 64, &uCopyBuffer2RegionType, sizeof(uCopyBuffer2RegionType));
    memcpy(abCopyBuffer2Command + 68, &uCopyBuffer2RegionNext, sizeof(uCopyBuffer2RegionNext));
    memcpy(abCopyBuffer2Command + 76, &offCopyBuffer2Src, sizeof(offCopyBuffer2Src));
    memcpy(abCopyBuffer2Command + 84, &offCopyBuffer2Dst, sizeof(offCopyBuffer2Dst));
    memcpy(abCopyBuffer2Command + 92, &cbCopyBuffer2, sizeof(cbCopyBuffer2));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[100]; }
        SubmitCopyBuffer2 = { { 100, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitCopyBuffer2.abCommand, abCopyBuffer2Command, sizeof(abCopyBuffer2Command));
    memset(pGpu->aResources[1].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyBuffer2,
                   sizeof(SubmitCopyBuffer2), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && !memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));

    RTTestSub(g_hTest, "Venus CopyBuffer2 batch submission");
    uint8_t abCopyBuffer2BatchCommand[200] = { 0 };
    memcpy(abCopyBuffer2BatchCommand, abCopyBuffer2Command, sizeof(abCopyBuffer2Command));
    memcpy(abCopyBuffer2BatchCommand + sizeof(abCopyBuffer2Command), abCopyBuffer2Command,
           sizeof(abCopyBuffer2Command));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[200]; }
        SubmitCopyBuffer2Batch = { { 200, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitCopyBuffer2Batch.abCommand, abCopyBuffer2BatchCommand, sizeof(abCopyBuffer2BatchCommand));
    memset(pGpu->aResources[1].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyBuffer2Batch,
                   sizeof(SubmitCopyBuffer2Batch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && !memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));

    RTTestSub(g_hTest, "Venus CopyBuffer2 maximum batch");
    uint8_t abCopyBuffer2MaxSubmit[8 + 8 + 100 * 64] = { 0 };
    VIRTIOGPUSUBMIT3D *pCopyBuffer2MaxHdr = (VIRTIOGPUSUBMIT3D *)abCopyBuffer2MaxSubmit;
    pCopyBuffer2MaxHdr->cbCommand = 100 * 64;
    pCopyBuffer2MaxHdr->cResources = 2;
    uint32_t *paCopyBuffer2MaxResources = (uint32_t *)(abCopyBuffer2MaxSubmit + 8);
    paCopyBuffer2MaxResources[0] = 7;
    paCopyBuffer2MaxResources[1] = 11;
    for (unsigned i = 0; i < 64; ++i)
        memcpy(abCopyBuffer2MaxSubmit + 16 + i * 100, abCopyBuffer2Command, sizeof(abCopyBuffer2Command));
    memset(pGpu->aResources[1].pbPixels, 0, sizeof(abPixels));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommandAt(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, abCopyBuffer2MaxSubmit,
                     sizeof(abCopyBuffer2MaxSubmit), 24, 0x4000, 0x7000, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x7000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && !memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));

    memset(pGpu->aResources[0].pbPixels, 0, sizeof(abPixels));
    pGpu->aResources[0].fVulkanImageDirty = true;
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
    RTTestSub(g_hTest, "Venus CopyImage legacy batch submission");
    uint8_t abCopyImageLegacyBatchCommand[240] = { 0 };
    memcpy(abCopyImageLegacyBatchCommand, abImageCopySubmitCommand, sizeof(abImageCopySubmitCommand));
    memcpy(abCopyImageLegacyBatchCommand + sizeof(abImageCopySubmitCommand), abImageCopySubmitCommand,
           sizeof(abImageCopySubmitCommand));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[240]; }
        SubmitCopyImageLegacyBatch = { { 240, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitCopyImageLegacyBatch.abCommand, abCopyImageLegacyBatchCommand,
           sizeof(abCopyImageLegacyBatchCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyImageLegacyBatch,
                   sizeof(SubmitCopyImageLegacyBatch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    RTTestSub(g_hTest, "Venus CopyImage2 batch submission");
    uint8_t abCopyImage2BatchCommand[304] = { 0 };
    uint32_t uCopyImage2BatchType = 208;
    uint64_t uCopyImage2BatchInfoPtr = 1;
    uint32_t uCopyImage2BatchInfoType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
    uint64_t uCopyImage2BatchSrc = 7;
    uint64_t uCopyImage2BatchDst = 11;
    uint32_t uCopyImage2BatchCount = 1;
    uint64_t uCopyImage2BatchArrayCount = 1;
    uint32_t uCopyImage2BatchRegionType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
    uint64_t uCopyImage2BatchRegionNext = 0;
    memcpy(abCopyImage2BatchCommand + 0, &uCopyImage2BatchType, sizeof(uCopyImage2BatchType));
    memcpy(abCopyImage2BatchCommand + 8, &uImageCopySubmitCommandBuffer, sizeof(uImageCopySubmitCommandBuffer));
    memcpy(abCopyImage2BatchCommand + 16, &uCopyImage2BatchInfoPtr, sizeof(uCopyImage2BatchInfoPtr));
    memcpy(abCopyImage2BatchCommand + 24, &uCopyImage2BatchInfoType, sizeof(uCopyImage2BatchInfoType));
    memcpy(abCopyImage2BatchCommand + 36, &uCopyImage2BatchSrc, sizeof(uCopyImage2BatchSrc));
    memcpy(abCopyImage2BatchCommand + 44, &uImageCopySubmitSrcLayout, sizeof(uImageCopySubmitSrcLayout));
    memcpy(abCopyImage2BatchCommand + 48, &uCopyImage2BatchDst, sizeof(uCopyImage2BatchDst));
    memcpy(abCopyImage2BatchCommand + 56, &uImageCopySubmitDstLayout, sizeof(uImageCopySubmitDstLayout));
    memcpy(abCopyImage2BatchCommand + 60, &uCopyImage2BatchCount, sizeof(uCopyImage2BatchCount));
    memcpy(abCopyImage2BatchCommand + 64, &uCopyImage2BatchArrayCount, sizeof(uCopyImage2BatchArrayCount));
    memcpy(abCopyImage2BatchCommand + 72, &uCopyImage2BatchRegionType, sizeof(uCopyImage2BatchRegionType));
    memcpy(abCopyImage2BatchCommand + 76, &uCopyImage2BatchRegionNext, sizeof(uCopyImage2BatchRegionNext));
    memcpy(abCopyImage2BatchCommand + 84, auImageCopySubmitSrcSubresource, sizeof(auImageCopySubmitSrcSubresource));
    memcpy(abCopyImage2BatchCommand + 100, aiImageCopySubmitOffset, sizeof(aiImageCopySubmitOffset));
    memcpy(abCopyImage2BatchCommand + 112, auImageCopySubmitDstSubresource, sizeof(auImageCopySubmitDstSubresource));
    memcpy(abCopyImage2BatchCommand + 128, aiImageCopySubmitOffset, sizeof(aiImageCopySubmitOffset));
    memcpy(abCopyImage2BatchCommand + 140, auImageCopySubmitExtent, sizeof(auImageCopySubmitExtent));
    memcpy(abCopyImage2BatchCommand + 152, abCopyImage2BatchCommand, 152);
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[304]; }
        SubmitCopyImage2Batch = { { 304, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitCopyImage2Batch.abCommand, abCopyImage2BatchCommand, sizeof(abCopyImage2BatchCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitCopyImage2Batch,
                   sizeof(SubmitCopyImage2Batch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    RTTestSub(g_hTest, "Venus vkCmdBlitImage2 execution");
    uint8_t abBlitImage2SubmitCommand[184] = { 0 };
    uint32_t uBlitImage2SubmitType = 211;
    uint64_t uBlitImage2SubmitInfoPtr = 1;
    uint32_t uBlitImage2SubmitInfoType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    uint64_t uBlitImage2SubmitSrc = 7;
    uint64_t uBlitImage2SubmitDst = 11;
    uint32_t uBlitImage2SubmitCount = 1;
    uint64_t uBlitImage2SubmitArrayCount = 1;
    uint32_t uBlitImage2SubmitRegionType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    uint64_t uBlitImage2SubmitOffsetCount = 2;
    uint32_t uBlitImage2SubmitFilter = VK_FILTER_NEAREST;
    int32_t aiBlitImage2SubmitOffsets[2][3] = { { 0, 0, 0 }, { 2, 2, 1 } };
    memcpy(abBlitImage2SubmitCommand + 0, &uBlitImage2SubmitType, sizeof(uBlitImage2SubmitType));
    memcpy(abBlitImage2SubmitCommand + 8, &uImageCopySubmitCommandBuffer, sizeof(uImageCopySubmitCommandBuffer));
    memcpy(abBlitImage2SubmitCommand + 16, &uBlitImage2SubmitInfoPtr, sizeof(uBlitImage2SubmitInfoPtr));
    memcpy(abBlitImage2SubmitCommand + 24, &uBlitImage2SubmitInfoType, sizeof(uBlitImage2SubmitInfoType));
    memcpy(abBlitImage2SubmitCommand + 36, &uBlitImage2SubmitSrc, sizeof(uBlitImage2SubmitSrc));
    memcpy(abBlitImage2SubmitCommand + 44, &uImageCopySubmitSrcLayout, sizeof(uImageCopySubmitSrcLayout));
    memcpy(abBlitImage2SubmitCommand + 48, &uBlitImage2SubmitDst, sizeof(uBlitImage2SubmitDst));
    memcpy(abBlitImage2SubmitCommand + 56, &uImageCopySubmitDstLayout, sizeof(uImageCopySubmitDstLayout));
    memcpy(abBlitImage2SubmitCommand + 60, &uBlitImage2SubmitCount, sizeof(uBlitImage2SubmitCount));
    memcpy(abBlitImage2SubmitCommand + 64, &uBlitImage2SubmitArrayCount, sizeof(uBlitImage2SubmitArrayCount));
    memcpy(abBlitImage2SubmitCommand + 72, &uBlitImage2SubmitRegionType, sizeof(uBlitImage2SubmitRegionType));
    memcpy(abBlitImage2SubmitCommand + 84, auImageCopySubmitSrcSubresource, sizeof(auImageCopySubmitSrcSubresource));
    memcpy(abBlitImage2SubmitCommand + 100, &uBlitImage2SubmitOffsetCount, sizeof(uBlitImage2SubmitOffsetCount));
    memcpy(abBlitImage2SubmitCommand + 108, aiBlitImage2SubmitOffsets, sizeof(aiBlitImage2SubmitOffsets));
    memcpy(abBlitImage2SubmitCommand + 132, auImageCopySubmitDstSubresource, sizeof(auImageCopySubmitDstSubresource));
    memcpy(abBlitImage2SubmitCommand + 148, &uBlitImage2SubmitOffsetCount, sizeof(uBlitImage2SubmitOffsetCount));
    memcpy(abBlitImage2SubmitCommand + 156, aiBlitImage2SubmitOffsets, sizeof(aiBlitImage2SubmitOffsets));
    memcpy(abBlitImage2SubmitCommand + 180, &uBlitImage2SubmitFilter, sizeof(uBlitImage2SubmitFilter));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[184]; }
        SubmitBlitImage2 = { { 184, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitBlitImage2.abCommand, abBlitImage2SubmitCommand, sizeof(abBlitImage2SubmitCommand));
    uint64_t const tsBlitSingleStart = RTTimeNanoTS();
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitBlitImage2,
                   sizeof(SubmitBlitImage2), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    uint64_t const cBlitSingleNs = RTTimeNanoTS() - tsBlitSingleStart;
    RTTESTI_CHECK_RC(virtioGpuR3VulkanResourceReadbackImage(pGpu, &pGpu->aResources[1]), VINF_SUCCESS);
    RTTESTI_CHECK(!memcmp(pGpu->aResources[1].pbPixels, pGpu->aResources[0].pbPixels, sizeof(abPixels)));
    RTTestSub(g_hTest, "Venus BlitImage2 batch submission");
    uint8_t abBlitImage2BatchCommand[368] = { 0 };
    memcpy(abBlitImage2BatchCommand, abBlitImage2SubmitCommand, sizeof(abBlitImage2SubmitCommand));
    memcpy(abBlitImage2BatchCommand + sizeof(abBlitImage2SubmitCommand), abBlitImage2SubmitCommand,
           sizeof(abBlitImage2SubmitCommand));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t auResourceIds[2]; uint8_t abCommand[368]; }
        SubmitBlitImage2Batch = { { 368, 2 }, { 7, 11 }, { 0 } };
    memcpy(SubmitBlitImage2Batch.abCommand, abBlitImage2BatchCommand, sizeof(abBlitImage2BatchCommand));
    uint64_t const tsBlitBatchStart = RTTimeNanoTS();
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitBlitImage2Batch,
                   sizeof(SubmitBlitImage2Batch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);
    uint64_t const cBlitBatchNs = RTTimeNanoTS() - tsBlitBatchStart;
    RTTestIPrintf(RTTESTLVL_ALWAYS, "Venus BlitImage2 submit: single=%llu ns, batch2=%llu ns (%llu ns/blit)\n",
                  cBlitSingleNs, cBlitBatchNs, cBlitBatchNs / 2);
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
    pGpu->aResources[0].fVulkanImageDirty = true;
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

    RTTestSub(g_hTest, "Venus PipelineBarrier legacy batch submission");
    uint8_t abBarrierSubmitBatchCommand[256] = { 0 };
    memcpy(abBarrierSubmitBatchCommand, abBarrierSubmitCommand, sizeof(abBarrierSubmitCommand));
    memcpy(abBarrierSubmitBatchCommand + sizeof(abBarrierSubmitCommand), abBarrierSubmitCommand,
           sizeof(abBarrierSubmitCommand));
    uint32_t const uBarrierBatchOldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t const uBarrierBatchNewLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    memcpy(abBarrierSubmitBatchCommand + 84, &uBarrierBatchOldLayout, sizeof(uBarrierBatchOldLayout));
    memcpy(abBarrierSubmitBatchCommand + 88, &uBarrierBatchNewLayout, sizeof(uBarrierBatchNewLayout));
    memcpy(abBarrierSubmitBatchCommand + 128 + 84, &uBarrierBatchOldLayout, sizeof(uBarrierBatchOldLayout));
    memcpy(abBarrierSubmitBatchCommand + 128 + 88, &uBarrierBatchNewLayout, sizeof(uBarrierBatchNewLayout));
    struct { VIRTIOGPUSUBMIT3D Hdr; uint32_t uResourceId; uint8_t abCommand[256]; }
        SubmitBarrierBatch = { { 256, 1 }, 7, { 0 } };
    memcpy(SubmitBarrierBatch.abCommand, abBarrierSubmitBatchCommand, sizeof(abBarrierSubmitBatchCommand));
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SUBMIT_3D, &SubmitBarrierBatch,
                   sizeof(SubmitBarrierBatch), 24, 43);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA);

    /* A clean image must not trigger an unnecessary image-to-buffer readback. */
    struct { uint32_t x, y, w, h, id, padding; } CleanFlush = { 0, 0, 2, 2, 7, 0 };
    g_cImageToBufferCalls = 0;
    g_fCountImageToBuffer = true;
    pGpu->pfnVkGetInstanceProcAddr = tstVkCopyInstanceProc;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &CleanFlush,
                   sizeof(CleanFlush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    pGpu->pfnVkGetInstanceProcAddr = g_pfnCopyTestInstanceProc;
    g_fCountImageToBuffer = false;
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && g_cImageToBufferCalls == 0);

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
    VIRTIOGPUSETSCANOUTBLOB BlobScanoutBad = BlobScanout;
    BlobScanoutBad.auStrides[0] = 4;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB, &BlobScanoutBad,
                   sizeof(BlobScanoutBad), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER
                  && pGpu->aScanouts[0].uStride == 8);
    BlobScanoutBad = BlobScanout;
    BlobScanoutBad.auOffsets[0] = 64;
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB, &BlobScanoutBad,
                   sizeof(BlobScanoutBad), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_ERR_INVALID_PARAMETER
                  && pGpu->aScanouts[0].uOffset == 0);
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

    /* The standalone test bypasses PCI construction; provide a small shared
       window so RESOURCE_MAP_BLOB can be verified end to end. */
    pGpu->pbSharedMemory = (uint8_t *)RTMemAllocZ(64 * 1024);
    pGpu->offSharedMemoryNext = 0;
    VIRTIOGPURESOURCECREATEBLOB BlobMappable = { 14, VIRTIOGPU_BLOB_MEM_HOST3D,
                                                 VIRTIOGPU_BLOB_FLAG_USE_MAPPABLE, 0,
                                                 UINT64_C(0x9abc), 64 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB, &BlobMappable,
                   sizeof(BlobMappable), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    RTTESTI_CHECK(pGpu->aResources[1].fSharedMemory && pGpu->aResources[1].pbPixels == pGpu->pbSharedMemory);
    VIRTIOGPURESOURCEMAPBLOB MapBlob = { { VIRTIOGPU_CMD_RESOURCE_MAP_BLOB, 0, 0, 0, 0 }, 14, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_MAP_BLOB, &MapBlob,
                   sizeof(MapBlob), sizeof(VIRTIOGPURESPMAPINFO));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == sizeof(VIRTIOGPURESPMAPINFO));
    VIRTIOGPURESPMAPINFO MapResp;
    memcpy(&MapResp, &g_abRam[0x5000], sizeof(MapResp));
    RTTESTI_CHECK(MapResp.Hdr.uType == VIRTIOGPU_RESP_OK_MAP_INFO && MapResp.uMapInfo == 0
                  && pGpu->aResources[1].fMapped);
    *(uint32_t *)pGpu->pbSharedMemory = UINT32_C(0xcafebabe);
    RTTESTI_CHECK_RC(virtioGpuR3VulkanCopyBuffer(pGpu, &pGpu->aResources[1], &pGpu->aResources[0], 0, 0, 4),
                         VINF_SUCCESS);
    RTTESTI_CHECK(*(uint32_t *)pGpu->aResources[0].pvVkMapped == UINT32_C(0xcafebabe)
                  && *(uint32_t *)pGpu->aResources[0].pbPixels == UINT32_C(0xcafebabe));
    VIRTIOGPUSETSCANOUTBLOB BlobMappableScanout = { 0, 0, 1, 1, 0, 14,
                                                    VIRTIOGPU_FORMAT_B8G8R8X8_UNORM,
                                                    1, 1, { 4, 0, 0, 0 }, { 0, 0, 0, 0 } };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB, &BlobMappableScanout,
                   sizeof(BlobMappableScanout), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && pGpu->aScanouts[0].uResourceId == 14);
    *(uint32_t *)pGpu->pbSharedMemory = UINT32_C(0xfeedface);
    struct { uint32_t x, y, w, h, id, padding; } BlobMappableFlush = { 0, 0, 1, 1, 14, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_FLUSH, &BlobMappableFlush,
                   sizeof(BlobMappableFlush), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA
                  && *(uint32_t *)pGpu->aResources[1].pvVkMapped == UINT32_C(0xfeedface));
    VIRTIOGPUSETSCANOUTBLOB BlobMappableScanoutDisable = { 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_SET_SCANOUT_BLOB,
                   &BlobMappableScanoutDisable, sizeof(BlobMappableScanoutDisable), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNMAP_BLOB, &MapBlob,
                   sizeof(MapBlob), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
    memcpy(&Resp, &g_abRam[0x5000], sizeof(Resp.Hdr));
    RTTESTI_CHECK(Resp.Hdr.uType == VIRTIOGPU_RESP_OK_NODATA && !pGpu->aResources[1].fMapped);
    struct { uint32_t id, padding; } UnrefMappable = { 14, 0 };
    uBefore = pGpu->Virtio.aVirtqueues[0].uUsedIdxShadow;
    tstPostCommand(&pGpu->Virtio, 0, VIRTIOGPU_CMD_RESOURCE_UNREF, &UnrefMappable,
                   sizeof(UnrefMappable), 24);
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 0);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 0, uBefore) == 24);
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
    uint8_t abBarrier2Command[164] = { 0 };
    uint32_t uBarrier2Type = 204;
    uint64_t uBarrier2InfoPtr = 1;
    uint32_t uBarrier2InfoType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    uint32_t uBarrier2ImageCount = 1;
    uint64_t uBarrier2ImageArray = 1;
    uint32_t uBarrier2SType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    uint64_t uBarrier2SrcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    uint64_t uBarrier2SrcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    uint64_t uBarrier2DstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    uint64_t uBarrier2DstAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
    uint32_t uBarrier2OldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    uint32_t uBarrier2NewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t uBarrier2Queue = VK_QUEUE_FAMILY_IGNORED;
    uint64_t uBarrier2Image = 7;
    memcpy(abBarrier2Command + 0, &uBarrier2Type, sizeof(uBarrier2Type));
    memcpy(abBarrier2Command + 8, &uBarrierCommandBuffer, sizeof(uBarrierCommandBuffer));
    memcpy(abBarrier2Command + 16, &uBarrier2InfoPtr, sizeof(uBarrier2InfoPtr));
    memcpy(abBarrier2Command + 24, &uBarrier2InfoType, sizeof(uBarrier2InfoType));
    memcpy(abBarrier2Command + 64, &uBarrier2ImageCount, sizeof(uBarrier2ImageCount));
    memcpy(abBarrier2Command + 68, &uBarrier2ImageArray, sizeof(uBarrier2ImageArray));
    memcpy(abBarrier2Command + 76, &uBarrier2SType, sizeof(uBarrier2SType));
    memcpy(abBarrier2Command + 88, &uBarrier2SrcStage, sizeof(uBarrier2SrcStage));
    memcpy(abBarrier2Command + 96, &uBarrier2SrcAccess, sizeof(uBarrier2SrcAccess));
    memcpy(abBarrier2Command + 104, &uBarrier2DstStage, sizeof(uBarrier2DstStage));
    memcpy(abBarrier2Command + 112, &uBarrier2DstAccess, sizeof(uBarrier2DstAccess));
    memcpy(abBarrier2Command + 120, &uBarrier2OldLayout, sizeof(uBarrier2OldLayout));
    memcpy(abBarrier2Command + 124, &uBarrier2NewLayout, sizeof(uBarrier2NewLayout));
    memcpy(abBarrier2Command + 128, &uBarrier2Queue, sizeof(uBarrier2Queue));
    memcpy(abBarrier2Command + 132, &uBarrier2Queue, sizeof(uBarrier2Queue));
    memcpy(abBarrier2Command + 136, &uBarrier2Image, sizeof(uBarrier2Image));
    memcpy(abBarrier2Command + 144, auClearRange, sizeof(auClearRange));
    VIRTIOGPUPIPELINEBARRIERCMD Barrier2;
    RTTESTI_CHECK(virtioGpuR3DecodePipelineBarrier2(abBarrier2Command, sizeof(abBarrier2Command), &Barrier2)
                  && Barrier2.uCommandBuffer == uBarrierCommandBuffer && Barrier2.fModern
                  && Barrier2.cImageBarriers == 1);
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
    uint8_t abCopyImages2Command[152] = { 0 };
    uint32_t uCopyImages2Type = 208;
    uint64_t uCopyImages2InfoPtr = 1;
    uint32_t uCopyImages2InfoType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
    uint64_t uCopyImages2Src = 7;
    uint64_t uCopyImages2Dst = 11;
    uint32_t uCopyImages2Count = 1;
    uint64_t uCopyImages2ArrayCount = 1;
    uint32_t uCopyImages2RegionType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
    memcpy(abCopyImages2Command + 0, &uCopyImages2Type, sizeof(uCopyImages2Type));
    memcpy(abCopyImages2Command + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abCopyImages2Command + 16, &uCopyImages2InfoPtr, sizeof(uCopyImages2InfoPtr));
    memcpy(abCopyImages2Command + 24, &uCopyImages2InfoType, sizeof(uCopyImages2InfoType));
    memcpy(abCopyImages2Command + 36, &uCopyImages2Src, sizeof(uCopyImages2Src));
    memcpy(abCopyImages2Command + 44, &uCopyImagesSrcLayout, sizeof(uCopyImagesSrcLayout));
    memcpy(abCopyImages2Command + 48, &uCopyImages2Dst, sizeof(uCopyImages2Dst));
    memcpy(abCopyImages2Command + 56, &uCopyImagesDstLayout, sizeof(uCopyImagesDstLayout));
    memcpy(abCopyImages2Command + 60, &uCopyImages2Count, sizeof(uCopyImages2Count));
    memcpy(abCopyImages2Command + 64, &uCopyImages2ArrayCount, sizeof(uCopyImages2ArrayCount));
    memcpy(abCopyImages2Command + 72, &uCopyImages2RegionType, sizeof(uCopyImages2RegionType));
    memcpy(abCopyImages2Command + 84, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abCopyImages2Command + 112, auCopyImagesDstSubresource, sizeof(auCopyImagesDstSubresource));
    memcpy(abCopyImages2Command + 140, auCopyImagesExtent, sizeof(auCopyImagesExtent));
    VIRTIOGPUCOPYIMAGECMD CopyImages2;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyImage2(abCopyImages2Command, sizeof(abCopyImages2Command), &CopyImages2)
                  && CopyImages2.uCommandBuffer == uCopyImagesCommandBuffer
                  && CopyImages2.uSrcImage == uCopyImages2Src && CopyImages2.uDstImage == uCopyImages2Dst
                  && CopyImages2.cbRegionStride == 80);
    uint8_t abBlitImage2Command[184] = { 0 };
    uint32_t uBlitImage2Type = 211;
    uint64_t uBlitImage2InfoPtr = 1;
    uint32_t uBlitImage2InfoType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    uint64_t uBlitImage2Src = 7;
    uint64_t uBlitImage2Dst = 11;
    uint32_t uBlitImage2Count = 1;
    uint64_t uBlitImage2ArrayCount = 1;
    uint32_t uBlitImage2RegionType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    uint64_t uBlitImage2OffsetCount = 2;
    uint32_t uBlitImage2Filter = VK_FILTER_NEAREST;
    int32_t aiBlitImage2Offsets[2][3] = { { 0, 0, 0 }, { 2, 2, 1 } };
    memcpy(abBlitImage2Command + 0, &uBlitImage2Type, sizeof(uBlitImage2Type));
    memcpy(abBlitImage2Command + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abBlitImage2Command + 16, &uBlitImage2InfoPtr, sizeof(uBlitImage2InfoPtr));
    memcpy(abBlitImage2Command + 24, &uBlitImage2InfoType, sizeof(uBlitImage2InfoType));
    memcpy(abBlitImage2Command + 36, &uBlitImage2Src, sizeof(uBlitImage2Src));
    memcpy(abBlitImage2Command + 44, &uCopyImagesSrcLayout, sizeof(uCopyImagesSrcLayout));
    memcpy(abBlitImage2Command + 48, &uBlitImage2Dst, sizeof(uBlitImage2Dst));
    memcpy(abBlitImage2Command + 56, &uCopyImagesDstLayout, sizeof(uCopyImagesDstLayout));
    memcpy(abBlitImage2Command + 60, &uBlitImage2Count, sizeof(uBlitImage2Count));
    memcpy(abBlitImage2Command + 64, &uBlitImage2ArrayCount, sizeof(uBlitImage2ArrayCount));
    memcpy(abBlitImage2Command + 72, &uBlitImage2RegionType, sizeof(uBlitImage2RegionType));
    memcpy(abBlitImage2Command + 84, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abBlitImage2Command + 100, &uBlitImage2OffsetCount, sizeof(uBlitImage2OffsetCount));
    memcpy(abBlitImage2Command + 108, aiBlitImage2Offsets, sizeof(aiBlitImage2Offsets));
    memcpy(abBlitImage2Command + 132, auCopyImagesDstSubresource, sizeof(auCopyImagesDstSubresource));
    memcpy(abBlitImage2Command + 148, &uBlitImage2OffsetCount, sizeof(uBlitImage2OffsetCount));
    memcpy(abBlitImage2Command + 156, aiBlitImage2Offsets, sizeof(aiBlitImage2Offsets));
    memcpy(abBlitImage2Command + 180, &uBlitImage2Filter, sizeof(uBlitImage2Filter));
    VIRTIOGPUBLITIMAGECMD BlitImage2;
    RTTESTI_CHECK(virtioGpuR3DecodeBlitImage2(abBlitImage2Command, sizeof(abBlitImage2Command), &BlitImage2)
                  && BlitImage2.uCommandBuffer == uCopyImagesCommandBuffer
                  && BlitImage2.uSrcImage == uBlitImage2Src && BlitImage2.uDstImage == uBlitImage2Dst
                  && BlitImage2.cbRegionStride == 108 && BlitImage2.enmFilter == VK_FILTER_NEAREST);
    uint8_t abBlitImageCommand[152] = { 0 };
    uint32_t uBlitImageType = 114;
    uint64_t uBlitImageSrc = 7;
    uint64_t uBlitImageDst = 11;
    uint32_t uBlitImageCount = 1;
    uint64_t uBlitImageRegions = 96;
    uint64_t uBlitImageOffsetCount = 2;
    uint32_t uBlitImageFilter = VK_FILTER_LINEAR;
    memcpy(abBlitImageCommand + 0, &uBlitImageType, sizeof(uBlitImageType));
    memcpy(abBlitImageCommand + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abBlitImageCommand + 16, &uBlitImageSrc, sizeof(uBlitImageSrc));
    memcpy(abBlitImageCommand + 24, &uCopyImagesSrcLayout, sizeof(uCopyImagesSrcLayout));
    memcpy(abBlitImageCommand + 28, &uBlitImageDst, sizeof(uBlitImageDst));
    memcpy(abBlitImageCommand + 36, &uCopyImagesDstLayout, sizeof(uCopyImagesDstLayout));
    memcpy(abBlitImageCommand + 40, &uBlitImageCount, sizeof(uBlitImageCount));
    memcpy(abBlitImageCommand + 44, &uBlitImageRegions, sizeof(uBlitImageRegions));
    memcpy(abBlitImageCommand + 52, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abBlitImageCommand + 68, &uBlitImageOffsetCount, sizeof(uBlitImageOffsetCount));
    memcpy(abBlitImageCommand + 76, aiBlitImage2Offsets, sizeof(aiBlitImage2Offsets));
    memcpy(abBlitImageCommand + 100, auCopyImagesDstSubresource, sizeof(auCopyImagesDstSubresource));
    memcpy(abBlitImageCommand + 116, &uBlitImageOffsetCount, sizeof(uBlitImageOffsetCount));
    memcpy(abBlitImageCommand + 124, aiBlitImage2Offsets, sizeof(aiBlitImage2Offsets));
    memcpy(abBlitImageCommand + 148, &uBlitImageFilter, sizeof(uBlitImageFilter));
    VIRTIOGPUBLITIMAGECMD BlitImage;
    RTTESTI_CHECK(virtioGpuR3DecodeBlitImage(abBlitImageCommand, sizeof(abBlitImageCommand), &BlitImage)
                  && BlitImage.uCommandBuffer == uCopyImagesCommandBuffer
                  && BlitImage.uSrcImage == uBlitImageSrc && BlitImage.uDstImage == uBlitImageDst
                  && BlitImage.cbRegionStride == 96 && BlitImage.enmFilter == VK_FILTER_LINEAR);
    uint8_t abCopyBufferToImage2Command[136] = { 0 };
    uint32_t uCopyBufferToImage2Type = 209;
    uint32_t uCopyBufferToImage2InfoType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
    uint64_t uCopyBufferToImage2Src = 9;
    uint64_t uCopyBufferToImage2Dst = 7;
    uint32_t uCopyBufferToImage2Layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    uint32_t uCopyBufferToImage2Count = 1;
    uint64_t uCopyBufferToImage2ArrayCount = 1;
    uint32_t uCopyBufferToImage2RegionType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
    memcpy(abCopyBufferToImage2Command + 0, &uCopyBufferToImage2Type, sizeof(uCopyBufferToImage2Type));
    memcpy(abCopyBufferToImage2Command + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abCopyBufferToImage2Command + 16, &uCopyImages2InfoPtr, sizeof(uCopyImages2InfoPtr));
    memcpy(abCopyBufferToImage2Command + 24, &uCopyBufferToImage2InfoType, sizeof(uCopyBufferToImage2InfoType));
    memcpy(abCopyBufferToImage2Command + 36, &uCopyBufferToImage2Src, sizeof(uCopyBufferToImage2Src));
    memcpy(abCopyBufferToImage2Command + 44, &uCopyBufferToImage2Dst, sizeof(uCopyBufferToImage2Dst));
    memcpy(abCopyBufferToImage2Command + 52, &uCopyBufferToImage2Layout, sizeof(uCopyBufferToImage2Layout));
    memcpy(abCopyBufferToImage2Command + 56, &uCopyBufferToImage2Count, sizeof(uCopyBufferToImage2Count));
    memcpy(abCopyBufferToImage2Command + 60, &uCopyBufferToImage2ArrayCount, sizeof(uCopyBufferToImage2ArrayCount));
    memcpy(abCopyBufferToImage2Command + 68, &uCopyBufferToImage2RegionType, sizeof(uCopyBufferToImage2RegionType));
    memcpy(abCopyBufferToImage2Command + 96, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abCopyBufferToImage2Command + 124, auCopyImagesExtent, sizeof(auCopyImagesExtent));
    VIRTIOGPUCOPYBUFFERTOIMAGECMD CopyBufferToImage2;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyBufferToImage2(abCopyBufferToImage2Command,
                                                      sizeof(abCopyBufferToImage2Command), &CopyBufferToImage2)
                  && CopyBufferToImage2.uCommandBuffer == uCopyImagesCommandBuffer
                  && CopyBufferToImage2.uSrcBuffer == uCopyBufferToImage2Src
                  && CopyBufferToImage2.uDstImage == uCopyBufferToImage2Dst
                  && CopyBufferToImage2.cbRegionStride == 68);
    uint8_t abCopyImageToBuffer2Command[136] = { 0 };
    uint32_t uCopyImageToBuffer2Type = 210;
    uint32_t uCopyImageToBuffer2InfoType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2;
    uint64_t uCopyImageToBuffer2Src = 7;
    uint64_t uCopyImageToBuffer2Dst = 9;
    uint32_t uCopyImageToBuffer2Count = 1;
    uint64_t uCopyImageToBuffer2ArrayCount = 1;
    memcpy(abCopyImageToBuffer2Command + 0, &uCopyImageToBuffer2Type, sizeof(uCopyImageToBuffer2Type));
    memcpy(abCopyImageToBuffer2Command + 8, &uCopyImagesCommandBuffer, sizeof(uCopyImagesCommandBuffer));
    memcpy(abCopyImageToBuffer2Command + 16, &uCopyImages2InfoPtr, sizeof(uCopyImages2InfoPtr));
    memcpy(abCopyImageToBuffer2Command + 24, &uCopyImageToBuffer2InfoType, sizeof(uCopyImageToBuffer2InfoType));
    memcpy(abCopyImageToBuffer2Command + 36, &uCopyImageToBuffer2Src, sizeof(uCopyImageToBuffer2Src));
    memcpy(abCopyImageToBuffer2Command + 44, &uCopyImagesSrcLayout, sizeof(uCopyImagesSrcLayout));
    memcpy(abCopyImageToBuffer2Command + 48, &uCopyImageToBuffer2Dst, sizeof(uCopyImageToBuffer2Dst));
    memcpy(abCopyImageToBuffer2Command + 56, &uCopyImageToBuffer2Count, sizeof(uCopyImageToBuffer2Count));
    memcpy(abCopyImageToBuffer2Command + 60, &uCopyImageToBuffer2ArrayCount, sizeof(uCopyImageToBuffer2ArrayCount));
    memcpy(abCopyImageToBuffer2Command + 68, &uCopyBufferToImage2RegionType, sizeof(uCopyBufferToImage2RegionType));
    memcpy(abCopyImageToBuffer2Command + 96, auCopyImagesSrcSubresource, sizeof(auCopyImagesSrcSubresource));
    memcpy(abCopyImageToBuffer2Command + 124, auCopyImagesExtent, sizeof(auCopyImagesExtent));
    VIRTIOGPUCOPYIMAGETOBUFFERCMD CopyImageToBuffer2;
    RTTESTI_CHECK(virtioGpuR3DecodeCopyImageToBuffer2(abCopyImageToBuffer2Command,
                                                      sizeof(abCopyImageToBuffer2Command), &CopyImageToBuffer2)
                  && CopyImageToBuffer2.uCommandBuffer == uCopyImagesCommandBuffer
                  && CopyImageToBuffer2.uSrcImage == uCopyImageToBuffer2Src
                  && CopyImageToBuffer2.uDstBuffer == uCopyImageToBuffer2Dst
                  && CopyImageToBuffer2.cbRegionStride == 68);
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
    uint16_t const uCursorWrapBefore = pGpu->Virtio.aVirtqueues[1].uUsedIdxShadow;
    tstPost(&pGpu->Virtio, 1, 56, 24, UINT32_C(0x03ff));
    virtioGpuR3VirtqNotified(pDev, &pGpu->Virtio, 1);
    RTTESTI_CHECK(tstCompletion(&pGpu->Virtio, 1, uCursorWrapBefore) == sizeof(VIRTIOGPUCTRLHDR));
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
