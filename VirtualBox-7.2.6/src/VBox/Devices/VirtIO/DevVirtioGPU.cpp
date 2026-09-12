/* SPDX-License-Identifier: GPL-3.0-only */
/** @file
 * VirtIO-GPU host device. Initial transport/control milestone; no renderer yet.
 */
#define LOG_GROUP LOG_GROUP_DEV_VIRTIO
#include <iprt/assert.h>
#include <iprt/errcore.h>
#include <iprt/string.h>
#include <iprt/sg.h>
#include <iprt/mem.h>
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

#define VIRTIOGPU_SAVED_STATE_VERSION UINT32_C(2)
#define VIRTIOGPU_MAX_RESOURCES 256
#define VIRTIOGPU_MAX_BACKING_ENTRIES 64
#define VIRTIOGPU_MAX_RESOURCE_BYTES (UINT64_C(256) * _1M)

typedef struct VIRTIOGPURESOURCE
{
    bool fUsed;
    uint32_t uResourceId;
    uint32_t uFormat;
    uint32_t uWidth;
    uint32_t uHeight;
    uint32_t cBacking;
    uint64_t cbPixels;
    uint8_t *pbPixels;
    VIRTIOGPUMEMENTRY aBacking[VIRTIOGPU_MAX_BACKING_ENTRIES];
} VIRTIOGPURESOURCE;
typedef VIRTIOGPURESOURCE *PVIRTIOGPURESOURCE;

typedef struct VIRTIOGPUSCANOUT
{
    uint32_t uResourceId;
    uint32_t uX;
    uint32_t uY;
    uint32_t uWidth;
    uint32_t uHeight;
    uint64_t uFlushSequence;
} VIRTIOGPUSCANOUT;

typedef struct VIRTIOGPU
{
    VIRTIOCORE      Virtio;  /* Must stay first for the common transport. */
    VIRTIOGPUCONFIG Config;
    VIRTIOGPURESOURCE aResources[VIRTIOGPU_MAX_RESOURCES];
    VIRTIOGPUSCANOUT aScanouts[VIRTIOGPU_MAX_SCANOUTS];
    uint64_t cbAllocated;
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
    size_t cbRemaining = cb;
    while (cb)
    {
        size_t cbSeg = cbRemaining;
        RTGCPHYS GCPhys = virtioCoreGCPhysChainGetNextSeg(pBuf->pSgPhysSend, &cbSeg);
        if (!cbSeg)
            return VERR_BUFFER_UNDERFLOW;
        int rc = virtioCoreGCPhysRead(pVirtio, pDevIns, GCPhys, pb, cbSeg);
        if (RT_FAILURE(rc))
            return rc;
        pb += cbSeg;
        cb -= cbSeg;
        cbRemaining -= cbSeg;
    }
    return VINF_SUCCESS;
}

static PVIRTIOGPURESOURCE virtioGpuR3FindResource(PVIRTIOGPU pThis, uint32_t uResourceId)
{
    if (!uResourceId)
        return NULL;
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aResources); ++i)
        if (pThis->aResources[i].fUsed && pThis->aResources[i].uResourceId == uResourceId)
            return &pThis->aResources[i];
    return NULL;
}

static void virtioGpuR3FreeResources(PVIRTIOGPU pThis)
{
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aResources); ++i)
    {
        RTMemFree(pThis->aResources[i].pbPixels);
        RT_ZERO(pThis->aResources[i]);
    }
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aScanouts); ++i)
        RT_ZERO(pThis->aScanouts[i]);
    pThis->cbAllocated = 0;
}

static bool virtioGpuR3RectValid(PVIRTIOGPURESOURCE pRes, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    return x <= pRes->uWidth && y <= pRes->uHeight && w <= pRes->uWidth - x && h <= pRes->uHeight - y;
}

static int virtioGpuR3ReadGuest(PPDMDEVINS pDevIns, PVIRTIOCORE pVirtio, PVIRTIOGPURESOURCE pRes,
                                uint64_t off, void *pvDst, size_t cb)
{
    uint8_t *pbDst = (uint8_t *)pvDst;
    uint64_t cbTotal = 0;
    for (unsigned i = 0; i < pRes->cBacking; ++i)
        cbTotal += pRes->aBacking[i].cb;
    if (off > cbTotal || cb > cbTotal - off)
        return VERR_BUFFER_OVERFLOW;
    for (unsigned i = 0; i < pRes->cBacking && cb; ++i)
    {
        const VIRTIOGPUMEMENTRY *pEntry = &pRes->aBacking[i];
        if (off >= pEntry->cb)
        {
            off -= pEntry->cb;
            continue;
        }
        size_t cbChunk = RT_MIN(cb, (size_t)pEntry->cb - (size_t)off);
        if (pEntry->GCPhys > UINT64_MAX - off)
            return VERR_OUT_OF_RANGE;
        int rc = virtioCoreGCPhysRead(pVirtio, pDevIns, pEntry->GCPhys + off, pbDst, cbChunk);
        if (RT_FAILURE(rc))
            return rc;
        pbDst += cbChunk;
        cb -= cbChunk;
        off = 0;
    }
    return cb ? VERR_BUFFER_UNDERFLOW : VINF_SUCCESS;
}

static void virtioGpuR3Response(VIRTIOGPUDISPLAYRESP *pResp, const VIRTIOGPUCTRLHDR *pReq, uint32_t uType)
{
    RT_ZERO(*pResp);
    pResp->Hdr.uType = uType;
    if (pReq->uFlags & VIRTIOGPU_FLAG_FENCE)
    {
        pResp->Hdr.uFlags = VIRTIOGPU_FLAG_FENCE;
        pResp->Hdr.uFenceId = pReq->uFenceId;
        pResp->Hdr.uCtxId = pReq->uCtxId;
    }
}

/** Process one control/cursor command and enqueue one used entry. */
static int virtioGpuR3Complete(PPDMDEVINS pDevIns, PVIRTIOCORE pVirtio, uint16_t uQueue, PVIRTQBUF pBuf)
{
    PVIRTIOGPU pThis = RT_FROM_MEMBER(pVirtio, VIRTIOGPU, Virtio);
    VIRTIOGPUDISPLAYRESP Resp;
    VIRTIOGPUCTRLHDR Req;
    RT_ZERO(Req);
    RT_ZERO(Resp);
    size_t cbResp = 0;
    int rcReq = virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Req, sizeof(Req));
    uint32_t uResponse = RT_FAILURE(rcReq) || (RT_SUCCESS(rcReq) && (Req.uFlags & ~VIRTIOGPU_FLAG_FENCE))
                        ? VIRTIOGPU_RESP_ERR_INVALID_PARAMETER : VIRTIOGPU_RESP_ERR_UNSPEC;
    if (uQueue == VIRTIOGPU_QUEUE_CONTROL && pBuf->cbPhysReturn >= sizeof(Resp.Hdr))
    {
        virtioGpuR3Response(&Resp, &Req, uResponse);
        cbResp = sizeof(Resp.Hdr);
    }
    if (RT_SUCCESS(rcReq) && !(Req.uFlags & ~VIRTIOGPU_FLAG_FENCE) && uQueue == VIRTIOGPU_QUEUE_CONTROL)
    {
        switch (Req.uType)
        {
            case VIRTIOGPU_CMD_GET_DISPLAY_INFO:
                if (pBuf->cbPhysReturn >= sizeof(Resp))
                {
                    Resp.Hdr.uType = VIRTIOGPU_RESP_OK_DISPLAY_INFO;
                    for (unsigned i = 0; i < VIRTIOGPU_MAX_SCANOUTS; ++i)
                    {
                        Resp.aScanouts[i].uX = pThis->aScanouts[i].uX;
                        Resp.aScanouts[i].uY = pThis->aScanouts[i].uY;
                        Resp.aScanouts[i].uWidth = pThis->aScanouts[i].uWidth;
                        Resp.aScanouts[i].uHeight = pThis->aScanouts[i].uHeight;
                        Resp.aScanouts[i].fEnabled = pThis->aScanouts[i].uResourceId != 0;
                    }
                    cbResp = sizeof(Resp);
                }
                else
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                break;
            case VIRTIOGPU_CMD_RESOURCE_CREATE_2D:
            {
                struct { uint32_t id, format, width, height; } Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd) || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd))))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else if (Cmd.id == 0 || virtioGpuR3FindResource(pThis, Cmd.id)
                         || Cmd.format != VIRTIOGPU_FORMAT_B8G8R8X8_UNORM
                         || Cmd.width == 0 || Cmd.height == 0 || Cmd.width > 16384 || Cmd.height > 16384
                         || (uint64_t)Cmd.width * Cmd.height * 4 > VIRTIOGPU_MAX_RESOURCE_BYTES
                         || pThis->cbAllocated + (uint64_t)Cmd.width * Cmd.height * 4 > VIRTIOGPU_MAX_RESOURCE_BYTES)
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = NULL;
                    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aResources); ++i)
                        if (!pThis->aResources[i].fUsed) { pRes = &pThis->aResources[i]; break; }
                    size_t cbPixels = (size_t)Cmd.width * Cmd.height * 4;
                    if (!pRes || !(pRes->pbPixels = (uint8_t *)RTMemAllocZ(cbPixels)))
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY;
                    else
                    {
                        pRes->fUsed = true; pRes->uResourceId = Cmd.id; pRes->uFormat = Cmd.format;
                        pRes->uWidth = Cmd.width; pRes->uHeight = Cmd.height; pRes->cbPixels = cbPixels;
                        pThis->cbAllocated += cbPixels;
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_UNREF:
            {
                struct { uint32_t id, padding; } Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd) || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd))))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = virtioGpuR3FindResource(pThis, Cmd.id);
                    if (!pRes) Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_RESOURCE_ID;
                    else
                    {
                        for (unsigned i=0; i<VIRTIOGPU_MAX_SCANOUTS; ++i)
                            if (pThis->aScanouts[i].uResourceId == Cmd.id) RT_ZERO(pThis->aScanouts[i]);
                        RTMemFree(pRes->pbPixels); pThis->cbAllocated -= pRes->cbPixels; RT_ZERO(*pRes);
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_ATTACH_BACKING:
            {
                struct { uint32_t id, count; } Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd) || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd)))
                    || Cmd.count == 0 || Cmd.count > VIRTIOGPU_MAX_BACKING_ENTRIES
                    || pBuf->cbPhysSend < (size_t)Cmd.count * sizeof(VIRTIOGPUMEMENTRY))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = virtioGpuR3FindResource(pThis, Cmd.id);
                    if (!pRes) Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_RESOURCE_ID;
                    else
                    {
                        VIRTIOGPUMEMENTRY aEntries[VIRTIOGPU_MAX_BACKING_ENTRIES];
                        rcReq = virtioGpuR3Read(pDevIns,pVirtio,pBuf,aEntries,Cmd.count*sizeof(*aEntries));
                        uint64_t cbBacking=0;
                        for (uint32_t i=0; RT_SUCCESS(rcReq) && i<Cmd.count; ++i)
                            if (aEntries[i].GCPhys > UINT64_MAX-aEntries[i].cb || cbBacking > UINT64_MAX-aEntries[i].cb) rcReq=VERR_OUT_OF_RANGE; else cbBacking+=aEntries[i].cb;
                        if (RT_FAILURE(rcReq) || cbBacking < pRes->cbPixels) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                        else { memcpy(pRes->aBacking,aEntries,Cmd.count*sizeof(*aEntries)); pRes->cBacking=Cmd.count; Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA; }
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_DETACH_BACKING:
            {
                uint32_t id=0;
                if (pBuf->cbPhysSend<sizeof(id) || RT_FAILURE(virtioGpuR3Read(pDevIns,pVirtio,pBuf,&id,sizeof(id)))) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = virtioGpuR3FindResource(pThis, id);
                    if (!pRes)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_RESOURCE_ID;
                    else
                    {
                        pRes->cBacking = 0;
                        for (unsigned i = 0; i < RT_ELEMENTS(pRes->aBacking); ++i)
                        {
                            pRes->aBacking[i].GCPhys = 0;
                            pRes->aBacking[i].cb = 0;
                            pRes->aBacking[i].uPadding = 0;
                        }
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_TRANSFER_TO_HOST_2D:
            {
                struct { uint32_t x,y,w,h; uint64_t off; uint32_t id,padding; } Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend<sizeof(Cmd) || RT_FAILURE(virtioGpuR3Read(pDevIns,pVirtio,pBuf,&Cmd,sizeof(Cmd)))) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = virtioGpuR3FindResource(pThis, Cmd.id);
                    uint64_t const stride = pRes ? (uint64_t)pRes->uWidth * 4 : 0;
                    bool fValid = pRes && pRes->cBacking && virtioGpuR3RectValid(pRes, Cmd.x, Cmd.y, Cmd.w, Cmd.h);
                    uint64_t const cbRow = (uint64_t)Cmd.w * 4;
                    if (fValid && (Cmd.off > UINT64_MAX - ((uint64_t)Cmd.y + Cmd.h) * stride
                                   || Cmd.off + (uint64_t)Cmd.y * stride > UINT64_MAX - (uint64_t)Cmd.x * 4))
                        fValid = false;
                    if (!fValid)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        for (uint32_t y = 0; y < Cmd.h && RT_SUCCESS(rcReq); ++y)
                        {
                            uint64_t const src = Cmd.off + (uint64_t)(Cmd.y + y) * stride + (uint64_t)Cmd.x * 4;
                            uint8_t * const pbDst = pRes->pbPixels + ((uint64_t)(Cmd.y + y) * pRes->uWidth + Cmd.x) * 4;
                            rcReq = virtioGpuR3ReadGuest(pDevIns, pVirtio, pRes, src, pbDst, (size_t)cbRow);
                        }
                        Resp.Hdr.uType = RT_SUCCESS(rcReq) ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_SET_SCANOUT:
            {
                VIRTIOGPUSETSCANOUT Cmd; RT_ZERO(Cmd);
                if(pBuf->cbPhysSend<sizeof(Cmd)-sizeof(Cmd.Hdr) || RT_FAILURE(virtioGpuR3Read(pDevIns,pVirtio,pBuf,(uint8_t *)&Cmd+sizeof(Cmd.Hdr),sizeof(Cmd)-sizeof(Cmd.Hdr)))) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else if(Cmd.uScanoutId>=VIRTIOGPU_MAX_SCANOUTS) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_SCANOUT_ID;
                else if(!Cmd.uResourceId) { RT_ZERO(pThis->aScanouts[Cmd.uScanoutId]); Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA; }
                else { PVIRTIOGPURESOURCE pRes=virtioGpuR3FindResource(pThis,Cmd.uResourceId); if(!pRes || !virtioGpuR3RectValid(pRes,Cmd.uX,Cmd.uY,Cmd.uWidth,Cmd.uHeight)) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER; else { pThis->aScanouts[Cmd.uScanoutId].uResourceId=Cmd.uResourceId; pThis->aScanouts[Cmd.uScanoutId].uX=Cmd.uX; pThis->aScanouts[Cmd.uScanoutId].uY=Cmd.uY; pThis->aScanouts[Cmd.uScanoutId].uWidth=Cmd.uWidth; pThis->aScanouts[Cmd.uScanoutId].uHeight=Cmd.uHeight; Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA; } }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_FLUSH:
            {
                VIRTIOGPURESOURCEFLUSH Cmd; RT_ZERO(Cmd);
                if(pBuf->cbPhysSend<sizeof(Cmd)-sizeof(Cmd.Hdr) || RT_FAILURE(virtioGpuR3Read(pDevIns,pVirtio,pBuf,(uint8_t *)&Cmd+sizeof(Cmd.Hdr),sizeof(Cmd)-sizeof(Cmd.Hdr)))) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else { PVIRTIOGPURESOURCE pRes=virtioGpuR3FindResource(pThis,Cmd.uResourceId); if(!pRes || !virtioGpuR3RectValid(pRes,Cmd.uX,Cmd.uY,Cmd.uWidth,Cmd.uHeight)) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER; else { for(unsigned i=0;i<VIRTIOGPU_MAX_SCANOUTS;++i) if(pThis->aScanouts[i].uResourceId==Cmd.uResourceId) pThis->aScanouts[i].uFlushSequence++; Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA; } }
                break;
            }
            default:
                break;
        }
    }
    if (pBuf->cbPhysReturn >= sizeof(Resp.Hdr))
    {
        uint32_t uFinal=Resp.Hdr.uType;
        virtioGpuR3Response(&Resp,&Req,uFinal);
    }
    RTSGSEG Seg={&Resp,cbResp}; RTSGBUF Sg; RTSgBufInit(&Sg,&Seg,1);
    return virtioCoreR3VirtqUsedBufPut(pDevIns,pVirtio,uQueue,cbResp?&Sg:NULL,pBuf);
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
    {
        PVIRTIOGPU pThis = RT_FROM_MEMBER(pVirtio, VIRTIOGPU, Virtio);
        pThis->Config.fEventsRead = 0;
        virtioGpuR3FreeResources(pThis);
    }
}

static DECLCALLBACK(int) virtioGpuR3SaveExec(PPDMDEVINS pDevIns, PSSMHANDLE pSSM)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    int rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pThis->Config.fEventsRead);
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aResources); ++i)
    {
        rc = pDevIns->pHlpR3->pfnSSMPutBool(pSSM, pThis->aResources[i].fUsed);
        if (RT_SUCCESS(rc) && pThis->aResources[i].fUsed)
        {
            PVIRTIOGPURESOURCE pRes = &pThis->aResources[i];
            rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uResourceId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uFormat);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uWidth);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uHeight);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->cBacking);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pRes->aBacking, sizeof(pRes->aBacking));
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pRes->pbPixels, pRes->cbPixels);
        }
    }
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aScanouts); ++i)
        rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, &pThis->aScanouts[i], sizeof(pThis->aScanouts[i]));
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
    virtioGpuR3FreeResources(pThis);
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aResources); ++i)
    {
        bool fUsed = false;
        rc = pDevIns->pHlpR3->pfnSSMGetBool(pSSM, &fUsed);
        if (RT_SUCCESS(rc) && fUsed)
        {
            PVIRTIOGPURESOURCE pRes = &pThis->aResources[i];
            pRes->fUsed = true;
            rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uResourceId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uFormat);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uWidth);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uHeight);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->cBacking);
            if (RT_SUCCESS(rc) && (pRes->cBacking > VIRTIOGPU_MAX_BACKING_ENTRIES || !pRes->uResourceId
                                   || pRes->uFormat != VIRTIOGPU_FORMAT_B8G8R8X8_UNORM
                                   || !pRes->uWidth || !pRes->uHeight
                                   || (uint64_t)pRes->uWidth * pRes->uHeight * 4 > VIRTIOGPU_MAX_RESOURCE_BYTES))
                rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetMem(pSSM, pRes->aBacking, sizeof(pRes->aBacking));
            if (RT_SUCCESS(rc))
            {
                uint64_t cbBacking = 0;
                for (uint32_t j = 0; j < pRes->cBacking; ++j)
                {
                    if (pRes->aBacking[j].GCPhys > UINT64_MAX - pRes->aBacking[j].cb
                        || cbBacking > UINT64_MAX - pRes->aBacking[j].cb)
                    {
                        rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
                        break;
                    }
                    cbBacking += pRes->aBacking[j].cb;
                }
                if (RT_SUCCESS(rc) && cbBacking < (uint64_t)pRes->uWidth * pRes->uHeight * 4)
                    rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
            }
            if (RT_SUCCESS(rc))
            {
                pRes->cbPixels = (size_t)pRes->uWidth * pRes->uHeight * 4;
                pRes->pbPixels = (uint8_t *)RTMemAlloc(pRes->cbPixels);
                if (!pRes->pbPixels) rc = VERR_NO_MEMORY;
                else
                {
                    pThis->cbAllocated += pRes->cbPixels;
                    rc = pDevIns->pHlpR3->pfnSSMGetMem(pSSM, pRes->pbPixels, pRes->cbPixels);
                }
            }
            if (RT_FAILURE(rc))
            {
                RTMemFree(pRes->pbPixels);
                RT_ZERO(*pRes);
            }
        }
    }
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aScanouts); ++i)
        rc = pDevIns->pHlpR3->pfnSSMGetMem(pSSM, &pThis->aScanouts[i], sizeof(pThis->aScanouts[i]));
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
