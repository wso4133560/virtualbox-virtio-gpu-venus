/* SPDX-License-Identifier: GPL-3.0-only */
/*
 * Minimal VirtIO-GPU control protocol definitions used by the host device.
 * The wire format is kept local so the device does not depend on firmware
 * headers or guest implementation details.
 */
#ifndef VBOX_INCLUDED_SRC_Devices_VirtIO_DevVirtioGPU_h
#define VBOX_INCLUDED_SRC_Devices_VirtIO_DevVirtioGPU_h

#include <iprt/types.h>
#include <iprt/assertcompile.h>

#pragma pack(1)
typedef struct VIRTIOGPUCTRLHDR
{
    uint32_t uType;
    uint32_t uFlags;
    uint64_t uFenceId;
    uint32_t uCtxId;
    uint32_t uPadding;
} VIRTIOGPUCTRLHDR;

typedef struct VIRTIOGPUCONFIG
{
    uint32_t fEventsRead;
    uint32_t fEventsClear;
    uint32_t cScanouts;
    uint32_t cCapsets;
} VIRTIOGPUCONFIG;

typedef struct VIRTIOGPUDISPLAYONE
{
    uint32_t uX;
    uint32_t uY;
    uint32_t uWidth;
    uint32_t uHeight;
    uint32_t fEnabled;
    uint32_t uFlags;
} VIRTIOGPUDISPLAYONE;

typedef struct VIRTIOGPUDISPLAYRESP
{
    VIRTIOGPUCTRLHDR Hdr;
    VIRTIOGPUDISPLAYONE aScanouts[16];
} VIRTIOGPUDISPLAYRESP;
#pragma pack()

AssertCompileSize(VIRTIOGPUCTRLHDR, 24);
AssertCompileSize(VIRTIOGPUCONFIG, 16);
AssertCompileSize(VIRTIOGPUDISPLAYONE, 24);
AssertCompileSize(VIRTIOGPUDISPLAYRESP, 408);

/* VirtIO-GPU control queue command and response values. */
#define VIRTIOGPU_CMD_GET_DISPLAY_INFO      UINT32_C(0x0100)
#define VIRTIOGPU_RESP_OK_NODATA            UINT32_C(0x1100)
#define VIRTIOGPU_RESP_OK_DISPLAY_INFO      UINT32_C(0x1101)
#define VIRTIOGPU_RESP_ERR_UNSPEC           UINT32_C(0x1200)
#define VIRTIOGPU_RESP_ERR_INVALID_PARAMETER UINT32_C(0x1205)
#define VIRTIOGPU_FLAG_FENCE                UINT32_C(1)

#define VIRTIOGPU_QUEUE_CONTROL             0
#define VIRTIOGPU_QUEUE_CURSOR              1
#define VIRTIOGPU_QUEUE_COUNT               2
#define VIRTIOGPU_SCANOUT_COUNT             16

#endif /* !VBOX_INCLUDED_SRC_Devices_VirtIO_DevVirtioGPU_h */
