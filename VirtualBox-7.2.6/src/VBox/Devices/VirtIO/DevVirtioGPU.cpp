/* SPDX-License-Identifier: GPL-3.0-only */
/** @file
 * VirtIO-GPU host device. Initial transport/control milestone; no renderer yet.
 */
#define LOG_GROUP LOG_GROUP_DEV_VIRTIO
#include <iprt/assert.h>
#include <iprt/errcore.h>
#include <iprt/string.h>
#include <iprt/sg.h>
#include <VBox/log.h>
#include <VBox/pci.h>
#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/ssm.h>
#include "../build/VBoxDD.h"
#include "VirtioCore.h"
#include "DevVirtioGPU.h"

#ifndef IN_RING3
# error "VirtIO-GPU currently runs entirely in ring 3."
#endif

#define VIRTIOGPU_SAVED_STATE_VERSION UINT32_C(1)

typedef struct VIRTIOGPU
{
    VIRTIOCORE      Virtio;  /* Must stay first for the common transport. */
    VIRTIOGPUCONFIG Config;
} VIRTIOGPU;
typedef VIRTIOGPU *PVIRTIOGPU;

typedef struct VIRTIOGPUCC
{
    VIRTIOCORER3 Virtio;     /* Must stay first for the common transport. */
} VIRTIOGPUCC;
typedef VIRTIOGPUCC *PVIRTIOGPUCC;

static DECLCALLBACK(int) virtioGpuR3DevCapRead(PPDMDEVINS pDevIns, uint32_t offCap, void *pvBuf, uint32_t cbRead)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    if (offCap > sizeof(pThis->Config) || cbRead > sizeof(pThis->Config) - offCap)
        return VINF_IOM_MMIO_UNUSED_00;
    memcpy(pvBuf, (const uint8_t *)&pThis->Config + offCap, cbRead);
    return VINF_SUCCESS;
}

static DECLCALLBACK(int) virtioGpuR3DevCapWrite(PPDMDEVINS pDevIns, uint32_t offCap, const void *pvBuf, uint32_t cbWrite)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    /* events_clear is write-one-to-clear; other fields are read-only. */
    if (offCap >= 4 && offCap < 8 && cbWrite <= 8 - offCap)
    {
        uint32_t fClear = 0;
        memcpy((uint8_t *)&fClear + offCap - 4, pvBuf, cbWrite);
        pThis->Config.fEventsRead &= ~fClear;
    }
    return VINF_SUCCESS;
}

/** Read a bounded request header, checking physical-memory access failures. */
static int virtioGpuR3Read(PPDMDEVINS pDevIns, PVIRTIOCORE pVirtio, PVIRTQBUF pBuf, void *pv, size_t cb)
{
    if (cb > pBuf->cbPhysSend || !pBuf->pSgPhysSend)
        return VERR_BUFFER_UNDERFLOW;
    uint8_t *pb = (uint8_t *)pv;
    while (cb)
    {
        size_t cbSeg = cb;
        RTGCPHYS GCPhys = virtioCoreGCPhysChainGetNextSeg(pBuf->pSgPhysSend, &cbSeg);
        if (!cbSeg)
            return VERR_BUFFER_UNDERFLOW;
        int rc = virtioCoreGCPhysRead(pVirtio, pDevIns, GCPhys, pb, cbSeg);
        if (RT_FAILURE(rc))
            return rc;
        pb += cbSeg;
        cb -= cbSeg;
        pBuf->cbPhysSend -= cbSeg;
    }
    return VINF_SUCCESS;
}

/** Always publish one completion, including malformed packets with no response space. */
static int virtioGpuR3Complete(PPDMDEVINS pDevIns, PVIRTIOCORE pVirtio, uint16_t uQueue, PVIRTQBUF pBuf)
{
    VIRTIOGPUDISPLAYRESP Resp;
    RT_ZERO(Resp);
    size_t cbResp = 0;
    /* Cursor commands have no response payload. Rendering/cursor state comes in the next milestone. */
    if (uQueue == VIRTIOGPU_QUEUE_CONTROL && pBuf->cbPhysReturn >= sizeof(Resp.Hdr))
    {
        VIRTIOGPUCTRLHDR Req;
        RT_ZERO(Req);
        int rc = virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Req, sizeof(Req));
        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
        cbResp = sizeof(Resp.Hdr);
        if (RT_SUCCESS(rc))
        {
            /* Context/ring-index features are not offered. Never echo reserved guest bits. */
            if (Req.uFlags & VIRTIOGPU_FLAG_FENCE)
            {
                Resp.Hdr.uFlags = VIRTIOGPU_FLAG_FENCE;
                Resp.Hdr.uFenceId = Req.uFenceId;
                Resp.Hdr.uCtxId = Req.uCtxId;
            }
            if (!(Req.uFlags & ~VIRTIOGPU_FLAG_FENCE))
            {
                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                if (Req.uType == VIRTIOGPU_CMD_GET_DISPLAY_INFO)
                {
                    if (pBuf->cbPhysReturn >= sizeof(Resp))
                    {
                        /* No scanout is connected until the software backend is implemented. */
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_DISPLAY_INFO;
                        cbResp = sizeof(Resp);
                    }
                    else
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                }
            }
        }
    }
    RTSGSEG Seg = { &Resp, cbResp };
    RTSGBUF Sg;
    RTSgBufInit(&Sg, &Seg, 1);
    /* This overload enqueues a used entry even for zero-byte completions. */
    return virtioCoreR3VirtqUsedBufPut(pDevIns, pVirtio, uQueue, cbResp ? &Sg : NULL, pBuf);
}

static DECLCALLBACK(void) virtioGpuR3VirtqNotified(PPDMDEVINS pDevIns, PVIRTIOCORE pVirtio, uint16_t uQueue)
{
    if (uQueue >= VIRTIOGPU_QUEUE_COUNT || !(pVirtio->fDeviceStatus & VIRTIO_STATUS_DRIVER_OK)
        || (pVirtio->fDeviceStatus & (VIRTIO_STATUS_DEVICE_NEEDS_RESET | VIRTIO_STATUS_FAILED))
        || !virtioCoreIsVirtqEnabled(pVirtio, uQueue))
        return;
    PVIRTQBUF pBuf = virtioCoreR3VirtqBufAlloc();
    if (!pBuf)
    {
        pVirtio->fDeviceStatus |= VIRTIO_STATUS_DEVICE_NEEDS_RESET;
        virtioCoreNotifyConfigChanged(pVirtio);
        return;
    }
    /* Snapshot the batch size: a guest adding work must not hold EMT indefinitely. */
    uint16_t cAvail = virtioCoreVirtqAvailBufCount(pDevIns, pVirtio, uQueue);
    int rc = cAvail <= pVirtio->aVirtqueues[uQueue].uQueueSize ? VINF_SUCCESS : VERR_INVALID_PARAMETER;
    for (uint16_t i = 0; RT_SUCCESS(rc) && i < cAvail; ++i)
    {
        rc = virtioCoreR3VirtqAvailBufGet(pDevIns, pVirtio, uQueue, pBuf, true);
        if (RT_SUCCESS(rc))
            rc = virtioGpuR3Complete(pDevIns, pVirtio, uQueue, pBuf);
    }
    virtioCoreVirtqUsedRingSync(pDevIns, pVirtio, uQueue);
    virtioCoreR3VirtqBufRelease(pVirtio, pBuf);
    if (RT_FAILURE(rc))
    {
        LogRelMax(64, ("virtio-gpu: queue %u failed (%Rrc), reset required\n", uQueue, rc));
        pVirtio->fDeviceStatus |= VIRTIO_STATUS_DEVICE_NEEDS_RESET;
        virtioCoreNotifyConfigChanged(pVirtio);
    }
}

static DECLCALLBACK(void) virtioGpuR3StatusChanged(PVIRTIOCORE pVirtio, PVIRTIOCORECC pVirtioCC, uint32_t fReady)
{
    RT_NOREF(pVirtioCC);
    for (uint16_t i = 0; i < VIRTIOGPU_QUEUE_COUNT; i++)
    {
        if (fReady)
        {
            virtioCoreR3VirtqAttach(pVirtio, i, i == VIRTIOGPU_QUEUE_CONTROL ? "control" : "cursor");
            if (virtioCoreIsVirtqEnabled(pVirtio, i))
                virtioCoreVirtqEnableNotify(pVirtio, i, true);
        }
        else
            virtioCoreR3VirtqDetach(pVirtio, i);
    }
    if (!fReady)
        RT_FROM_MEMBER(pVirtio, VIRTIOGPU, Virtio)->Config.fEventsRead = 0;
}

static DECLCALLBACK(int) virtioGpuR3SaveExec(PPDMDEVINS pDevIns, PSSMHANDLE pSSM)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    int rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pThis->Config.fEventsRead);
    if (RT_SUCCESS(rc))
        rc = virtioCoreR3SaveExec(&pThis->Virtio, pDevIns->pHlpR3, pSSM,
                                VIRTIOGPU_SAVED_STATE_VERSION, VIRTIOGPU_QUEUE_COUNT);
    return rc;
}

static DECLCALLBACK(int) virtioGpuR3LoadExec(PPDMDEVINS pDevIns, PSSMHANDLE pSSM, uint32_t uVersion, uint32_t uPass)
{
    if (uVersion != VIRTIOGPU_SAVED_STATE_VERSION)
        return VERR_SSM_UNSUPPORTED_DATA_UNIT_VERSION;
    if (uPass != SSM_PASS_FINAL)
        return VINF_SUCCESS;
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    int rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pThis->Config.fEventsRead);
    if (RT_SUCCESS(rc))
        rc = virtioCoreR3ModernDeviceLoadExec(&pThis->Virtio, pDevIns->pHlpR3, pSSM, uVersion,
                                            VIRTIOGPU_SAVED_STATE_VERSION, VIRTIOGPU_QUEUE_COUNT);
    if (RT_SUCCESS(rc))
    {
        for (unsigned i = 0; i < VIRTQ_MAX_COUNT; i++)
            if (pThis->Virtio.aVirtqueues[i].uQueueSize > VIRTQ_SIZE
                || (i >= VIRTIOGPU_QUEUE_COUNT && pThis->Virtio.aVirtqueues[i].uEnable))
                return VERR_SSM_LOAD_CONFIG_MISMATCH;
        PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
        virtioGpuR3StatusChanged(&pThis->Virtio, &pThisCC->Virtio, (pThis->Virtio.fDeviceStatus & VIRTIO_STATUS_DRIVER_OK));
    }
    return rc;
}

static DECLCALLBACK(int) virtioGpuR3Construct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg)
{
    RT_NOREF(pCfg);
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "", "");
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    pThis->Config.cScanouts = 1;
    pThisCC->Virtio.pfnStatusChanged = virtioGpuR3StatusChanged;
    pThisCC->Virtio.pfnVirtqNotified = virtioGpuR3VirtqNotified;
    pThisCC->Virtio.pfnDevCapRead = virtioGpuR3DevCapRead;
    pThisCC->Virtio.pfnDevCapWrite = virtioGpuR3DevCapWrite;
    VIRTIOPCIPARAMS Pci;
    RT_ZERO(Pci);
    Pci.uDeviceId = DEVICE_PCI_DEVICE_ID_VIRTIO_BASE + VIRTIO_DEVICE_TYPE_GPU;
    Pci.uClassBase = VBOX_PCI_CLASS_DISPLAY;
    Pci.uClassSub = 0x80; /* Display other: this device has no legacy VGA aperture. */
    Pci.uSubsystemId = VIRTIO_DEVICE_TYPE_GPU;
    Pci.uInterruptPin = 1;
    Pci.uDeviceType = VIRTIO_DEVICE_TYPE_GPU;
    char szName[16];
    RTStrPrintf(szName, sizeof(szName), "virtio-gpu%u", iInstance);
    int rc = virtioCoreR3Init(pDevIns, &pThis->Virtio, &pThisCC->Virtio, &Pci, szName,
                            0, 0, &pThis->Config, sizeof(pThis->Config), VIRTIOGPU_QUEUE_COUNT);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc, N_("virtio-gpu: failed to initialize VirtIO core"));
    return PDMDevHlpSSMRegister(pDevIns, VIRTIOGPU_SAVED_STATE_VERSION, sizeof(*pThis),
                               virtioGpuR3SaveExec, virtioGpuR3LoadExec);
}

static DECLCALLBACK(void) virtioGpuR3Reset(PPDMDEVINS pDevIns)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    virtioCoreR3ResetDevice(pDevIns, &pThis->Virtio, &pThisCC->Virtio);
}

static DECLCALLBACK(int) virtioGpuR3Destruct(PPDMDEVINS pDevIns)
{
    PDMDEV_CHECK_VERSIONS_RETURN_QUIET(pDevIns);
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    virtioCoreR3Term(pDevIns, &pThis->Virtio, &pThisCC->Virtio);
    return VINF_SUCCESS;
}

const PDMDEVREG g_DeviceVirtioGPU =
{
    /* .uVersion = */          PDM_DEVREG_VERSION,
    /* .uReserved0 = */        0,
    /* .szName = */            "virtio-gpu",
    /* .fFlags = */            PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE,
    /* .fClass = */            PDM_DEVREG_CLASS_GRAPHICS,
    /* .cMaxInstances = */     1,
    /* .uSharedVersion = */    1,
    /* .cbInstanceShared = */  sizeof(VIRTIOGPU),
    /* .cbInstanceCC = */      sizeof(VIRTIOGPUCC),
    /* .cbInstanceRC = */      0,
    /* .cMaxPciDevices = */    1,
    /* .cMaxMsixVectors = */   0,
    /* .pszDescription = */    "VirtIO GPU control device",
    /* .pszRCMod = */          NULL,
    /* .pszR0Mod = */          NULL,
    /* .pfnConstruct = */      virtioGpuR3Construct,
    /* .pfnDestruct = */       virtioGpuR3Destruct,
    /* .pfnRelocate = */       NULL,
    /* .pfnMemSetup = */       NULL,
    /* .pfnPowerOn = */        NULL,
    /* .pfnReset = */          virtioGpuR3Reset,
    /* .pfnSuspend = */        NULL,
    /* .pfnResume = */         NULL,
    /* .pfnAttach = */         NULL,
    /* .pfnDetach = */         NULL,
    /* .pfnQueryInterface = */ NULL,
    /* .pfnInitComplete = */   NULL,
    /* .pfnPowerOff = */       NULL,
    /* .pfnSoftReset = */      NULL,
    /* .pfnReserved0 = */      NULL,
    /* .pfnReserved1 = */      NULL,
    /* .pfnReserved2 = */      NULL,
    /* .pfnReserved3 = */      NULL,
    /* .pfnReserved4 = */      NULL,
    /* .pfnReserved5 = */      NULL,
    /* .pfnReserved6 = */      NULL,
    /* .pfnReserved7 = */      NULL,
    /* .uVersionEnd = */       PDM_DEVREG_VERSION
};
