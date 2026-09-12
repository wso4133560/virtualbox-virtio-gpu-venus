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

typedef struct VIRTIOGPUMEMENTRY
{
    uint64_t GCPhys;
    uint32_t cb;
    uint32_t uPadding;
} VIRTIOGPUMEMENTRY;

typedef struct VIRTIOGPURESOURCECREATE2D
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uResourceId;
    uint32_t uFormat;
    uint32_t uWidth;
    uint32_t uHeight;
} VIRTIOGPURESOURCECREATE2D;

typedef struct VIRTIOGPURESOURCEUNREF
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uResourceId;
    uint32_t uPadding2;
} VIRTIOGPURESOURCEUNREF;

typedef struct VIRTIOGPUSETSCANOUT
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uX;
    uint32_t uY;
    uint32_t uWidth;
    uint32_t uHeight;
    uint32_t uScanoutId;
    uint32_t uResourceId;
} VIRTIOGPUSETSCANOUT;

typedef struct VIRTIOGPURESOURCEATTACHBACKING
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uResourceId;
    uint32_t cEntries;
} VIRTIOGPURESOURCEATTACHBACKING;

typedef struct VIRTIOGPURESOURCEDETACHBACKING
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uResourceId;
    uint32_t uPadding2;
} VIRTIOGPURESOURCEDETACHBACKING;

typedef struct VIRTIOGPURESOURCECREATEBLOB
{
    uint32_t uResourceId;
    uint32_t uBlobMem;
    uint32_t fBlob;
    uint32_t cEntries;
    uint64_t uBlobId;
    uint64_t cbBlob;
} VIRTIOGPURESOURCECREATEBLOB;

typedef struct VIRTIOGPUTRANSFERTOHOST2D
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uX;
    uint32_t uY;
    uint32_t uWidth;
    uint32_t uHeight;
    uint64_t off;
    uint32_t uResourceId;
    uint32_t uPadding2;
} VIRTIOGPUTRANSFERTOHOST2D;

typedef struct VIRTIOGPURESOURCEFLUSH
{
    VIRTIOGPUCTRLHDR Hdr;
    uint32_t uX;
    uint32_t uY;
    uint32_t uWidth;
    uint32_t uHeight;
    uint32_t uResourceId;
    uint32_t uPadding2;
} VIRTIOGPURESOURCEFLUSH;

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
AssertCompileSize(VIRTIOGPUMEMENTRY, 16);
AssertCompileSize(VIRTIOGPURESOURCECREATE2D, 40);
AssertCompileSize(VIRTIOGPURESOURCEUNREF, 32);
AssertCompileSize(VIRTIOGPUSETSCANOUT, 48);
AssertCompileSize(VIRTIOGPURESOURCEATTACHBACKING, 32);
AssertCompileSize(VIRTIOGPURESOURCEDETACHBACKING, 32);
AssertCompileSize(VIRTIOGPURESOURCECREATEBLOB, 32);
AssertCompileSize(VIRTIOGPUTRANSFERTOHOST2D, 56);
AssertCompileSize(VIRTIOGPURESOURCEFLUSH, 48);
AssertCompileSize(VIRTIOGPUDISPLAYONE, 24);
AssertCompileSize(VIRTIOGPUDISPLAYRESP, 408);

/* VirtIO-GPU control queue command and response values. */
#define VIRTIOGPU_CMD_GET_DISPLAY_INFO      UINT32_C(0x0100)
#define VIRTIOGPU_RESP_OK_NODATA            UINT32_C(0x1100)
#define VIRTIOGPU_RESP_OK_DISPLAY_INFO      UINT32_C(0x1101)
#define VIRTIOGPU_RESP_ERR_UNSPEC           UINT32_C(0x1200)
#define VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY    UINT32_C(0x1201)
#define VIRTIOGPU_RESP_ERR_INVALID_SCANOUT_ID UINT32_C(0x1202)
#define VIRTIOGPU_RESP_ERR_INVALID_RESOURCE_ID UINT32_C(0x1204)
#define VIRTIOGPU_RESP_ERR_INVALID_PARAMETER UINT32_C(0x1205)
#define VIRTIOGPU_CMD_RESOURCE_CREATE_2D     UINT32_C(0x0101)
#define VIRTIOGPU_CMD_RESOURCE_UNREF         UINT32_C(0x0102)
#define VIRTIOGPU_CMD_SET_SCANOUT            UINT32_C(0x0103)
#define VIRTIOGPU_CMD_RESOURCE_FLUSH         UINT32_C(0x0104)
#define VIRTIOGPU_CMD_TRANSFER_TO_HOST_2D   UINT32_C(0x0105)
#define VIRTIOGPU_CMD_RESOURCE_ATTACH_BACKING UINT32_C(0x0106)
#define VIRTIOGPU_CMD_RESOURCE_DETACH_BACKING UINT32_C(0x0107)
#define VIRTIOGPU_CMD_GET_CAPSET_INFO          UINT32_C(0x0108)
#define VIRTIOGPU_CMD_GET_CAPSET               UINT32_C(0x0109)
#define VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB     UINT32_C(0x010a)
#define VIRTIOGPU_RESP_OK_CAPSET_INFO          UINT32_C(0x1102)
#define VIRTIOGPU_RESP_OK_CAPSET               UINT32_C(0x1103)
#define VIRTIOGPU_CMD_CTX_CREATE               UINT32_C(0x0200)
#define VIRTIOGPU_CMD_CTX_DESTROY              UINT32_C(0x0201)
#define VIRTIOGPU_FLAG_FENCE                UINT32_C(1)

#define VIRTIOGPU_QUEUE_CONTROL             0
#define VIRTIOGPU_QUEUE_CURSOR              1
#define VIRTIOGPU_QUEUE_COUNT               2
#define VIRTIOGPU_MAX_SCANOUTS              16
#define VIRTIOGPU_SCANOUT_COUNT             1
#define VIRTIOGPU_FORMAT_B8G8R8X8_UNORM     UINT32_C(2)

#endif /* !VBOX_INCLUDED_SRC_Devices_VirtIO_DevVirtioGPU_h */
