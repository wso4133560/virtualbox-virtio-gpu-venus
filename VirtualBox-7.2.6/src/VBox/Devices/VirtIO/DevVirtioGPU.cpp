/* SPDX-License-Identifier: GPL-3.0-only */
/** @file
 * VirtIO-GPU host device with the optional R3 Vulkan/Venus resource backend.
 */
#define LOG_GROUP LOG_GROUP_DEV_VIRTIO
#include <iprt/assert.h>
#include <iprt/errcore.h>
#include <iprt/string.h>
#include <iprt/sg.h>
#include <iprt/mem.h>
#include <iprt/uuid.h>
#include <iprt/ldr.h>
#include <iprt/path.h>
#ifdef RT_OS_WINDOWS
# include <iprt/win/windows.h>
#endif
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
# include <vulkan/vulkan.h>
#endif
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

#define VIRTIOGPU_SAVED_STATE_VERSION UINT32_C(5)
#define VIRTIOGPU_MAX_RESOURCES 256
#define VIRTIOGPU_MAX_CONTEXTS 64
#define VIRTIOGPU_MAX_CONTEXT_RESOURCES 64
#define VIRTIOGPU_MAX_SUBMIT_BYTES (UINT32_C(1) * _1M)
#define VIRTIOGPU_VK_CMD_FILL_BUFFER UINT32_C(118)
#define VIRTIOGPU_VK_CMD_COPY_BUFFER UINT32_C(112)
#define VIRTIOGPU_VK_CMD_UPDATE_BUFFER UINT32_C(117)
#define VIRTIOGPU_VK_CMD_CLEAR_COLOR_IMAGE UINT32_C(119)
#define VIRTIOGPU_VK_CMD_COPY_BUFFER_TO_IMAGE UINT32_C(115)
#define VIRTIOGPU_VK_CMD_COPY_IMAGE_TO_BUFFER UINT32_C(116)
#define VIRTIOGPU_VK_CMD_PIPELINE_BARRIER UINT32_C(126)
#define VIRTIOGPU_VK_CMD_COPY_IMAGE UINT32_C(113)
#define VIRTIOGPU_MAX_BACKING_ENTRIES 64
#define VIRTIOGPU_MAX_RESOURCE_BYTES (UINT64_C(256) * _1M)

typedef enum VIRTIOGPUBACKEND
{
    VIRTIOGPU_BACKEND_AUTO = 0,
    VIRTIOGPU_BACKEND_SOFTWARE,
    VIRTIOGPU_BACKEND_VENUS
} VIRTIOGPUBACKEND;

typedef struct VIRTIOGPURESOURCE
{
    bool fUsed;
    uint32_t uResourceId;
    uint32_t uFormat;
    uint32_t uWidth;
    uint32_t uHeight;
    bool fBlob;
    uint32_t cBacking;
    uint64_t cbPixels;
    uint8_t *pbPixels;
    VIRTIOGPUMEMENTRY aBacking[VIRTIOGPU_MAX_BACKING_ENTRIES];
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    VkBuffer hVkBuffer;
    VkDeviceMemory hVkMemory;
    void *pvVkMapped;
    VkImage hVkImage;
    VkDeviceMemory hVkImageMemory;
    VkImageLayout enmVkImageLayout;
    bool fVulkanBuffer;
    bool fVulkanImage;
    bool fVulkanImageDirty;
#endif
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

typedef struct VIRTIOGPUCONTEXT
{
    bool fUsed;
    uint32_t uContextId;
    uint32_t cchName;
    char szName[64];
    uint32_t cResources;
    uint32_t auResourceIds[VIRTIOGPU_MAX_CONTEXT_RESOURCES];
} VIRTIOGPUCONTEXT;
typedef VIRTIOGPUCONTEXT *PVIRTIOGPUCONTEXT;

typedef struct VIRTIOGPU
{
    VIRTIOCORE      Virtio;  /* Must stay first for the common transport. */
    VIRTIOGPUCONFIG Config;
    VIRTIOGPURESOURCE aResources[VIRTIOGPU_MAX_RESOURCES];
    VIRTIOGPUCONTEXT aContexts[VIRTIOGPU_MAX_CONTEXTS];
    VIRTIOGPUSCANOUT aScanouts[VIRTIOGPU_MAX_SCANOUTS];
    uint64_t cbAllocated;
    VIRTIOGPUBACKEND enmBackend;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    RTLDRMOD hVulkan;
    PFN_vkGetInstanceProcAddr pfnVkGetInstanceProcAddr;
    VkInstance hVkInstance;
    VkPhysicalDevice hVkPhysicalDevice;
    VkDevice hVkDevice;
    VkQueue hVkQueue;
    VkCommandPool hVkSubmitPool;
    VkCommandBuffer hVkSubmitCommandBuffer;
    VkFence hVkSubmitFence;
    uint32_t uVkQueueFamily;
    VkPhysicalDeviceProperties VkProperties;
    VkPhysicalDeviceMemoryProperties VkMemoryProperties;
    uint32_t uVkApiVersion;
    bool fVulkanLoader;
    bool fVulkanDevice;
    bool fVulkanQueue;
    bool fVulkanMemory;
    bool fVulkanSubmit;
#endif
} VIRTIOGPU;
typedef VIRTIOGPU *PVIRTIOGPU;

typedef struct VIRTIOGPUCC
{
    VIRTIOCORER3 Virtio;     /* Must stay first for the common transport. */
    PDMIBASE     IBase;
    R3PTRTYPE(PPDMIBASE) pDrvBase;
    R3PTRTYPE(PPDMIDISPLAYCONNECTOR) pDrv;
} VIRTIOGPUCC;
typedef VIRTIOGPUCC *PVIRTIOGPUCC;

#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
static int virtioGpuR3VulkanResourceCreate(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes);
static void virtioGpuR3VulkanResourceDestroy(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes);
static int virtioGpuR3VulkanResourceSync(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes);
#endif

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

static int virtioGpuR3ParseBackend(const char *pszBackend, VIRTIOGPUBACKEND *penmBackend)
{
    if (!pszBackend || !*pszBackend || !penmBackend)
        return VERR_INVALID_PARAMETER;
    if (!RTStrICmp(pszBackend, "auto"))
        *penmBackend = VIRTIOGPU_BACKEND_AUTO;
    else if (!RTStrICmp(pszBackend, "software"))
        *penmBackend = VIRTIOGPU_BACKEND_SOFTWARE;
    else if (!RTStrICmp(pszBackend, "venus"))
        *penmBackend = VIRTIOGPU_BACKEND_VENUS;
    else
        return VERR_INVALID_PARAMETER;
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

static PVIRTIOGPUCONTEXT virtioGpuR3FindContext(PVIRTIOGPU pThis, uint32_t uContextId)
{
    if (!uContextId)
        return NULL;
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aContexts); ++i)
        if (pThis->aContexts[i].fUsed && pThis->aContexts[i].uContextId == uContextId)
            return &pThis->aContexts[i];
    return NULL;
}

static bool virtioGpuR3ContextHasResource(PVIRTIOGPUCONTEXT pCtx, uint32_t uResourceId)
{
    for (uint32_t i = 0; i < pCtx->cResources; ++i)
        if (pCtx->auResourceIds[i] == uResourceId)
            return true;
    return false;
}

static void virtioGpuR3FreeResources(PVIRTIOGPU pThis)
{
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aResources); ++i)
    {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
        virtioGpuR3VulkanResourceDestroy(pThis, &pThis->aResources[i]);
#endif
        RTMemFree(pThis->aResources[i].pbPixels);
        RT_ZERO(pThis->aResources[i]);
    }
    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aScanouts); ++i)
        RT_ZERO(pThis->aScanouts[i]);
    RT_ZERO(pThis->aContexts);
    pThis->cbAllocated = 0;
}

#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
static int virtioGpuR3VulkanInit(PVIRTIOGPU pThis)
{
    pThis->hVulkan = NIL_RTLDRMOD;
    pThis->pfnVkGetInstanceProcAddr = NULL;
    pThis->hVkInstance = VK_NULL_HANDLE;
    pThis->hVkPhysicalDevice = VK_NULL_HANDLE;
    pThis->hVkDevice = VK_NULL_HANDLE;
    pThis->hVkQueue = VK_NULL_HANDLE;
    pThis->hVkSubmitPool = VK_NULL_HANDLE;
    pThis->hVkSubmitCommandBuffer = VK_NULL_HANDLE;
    pThis->hVkSubmitFence = VK_NULL_HANDLE;
    pThis->uVkQueueFamily = UINT32_MAX;
    RT_ZERO(pThis->VkMemoryProperties);
    pThis->uVkApiVersion = VK_API_VERSION_1_0;
    pThis->fVulkanLoader = false;
    pThis->fVulkanDevice = false;
    pThis->fVulkanQueue = false;
    pThis->fVulkanMemory = false;
    pThis->fVulkanSubmit = false;
    char szVulkanPath[RTPATH_MAX] = "vulkan-1.dll";
# ifdef RT_OS_WINDOWS
    char szSystemDir[RTPATH_MAX];
    UINT cchSystemDir = GetSystemDirectoryA(szSystemDir, sizeof(szSystemDir));
    if (cchSystemDir && cchSystemDir < sizeof(szSystemDir) - sizeof("\\vulkan-1.dll"))
    {
        RTStrCopy(szVulkanPath, sizeof(szVulkanPath), szSystemDir);
        RTStrCat(szVulkanPath, sizeof(szVulkanPath), "\\vulkan-1.dll");
    }
# endif
    int rc = RTLdrLoad(szVulkanPath, &pThis->hVulkan);
    if (RT_FAILURE(rc))
        return rc;
    rc = RTLdrGetSymbol(pThis->hVulkan, "vkGetInstanceProcAddr", (void **)&pThis->pfnVkGetInstanceProcAddr);
    if (RT_FAILURE(rc) || !pThis->pfnVkGetInstanceProcAddr)
        return VERR_NOT_FOUND;
    pThis->fVulkanLoader = true;
    PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
        (PFN_vkEnumerateInstanceVersion)pThis->pfnVkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
    if (pfnEnumerateInstanceVersion)
        pfnEnumerateInstanceVersion(&pThis->uVkApiVersion);
    PFN_vkCreateInstance pfnCreateInstance =
        (PFN_vkCreateInstance)pThis->pfnVkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    if (!pfnCreateInstance)
        return VERR_NOT_FOUND;
    VkApplicationInfo AppInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "VirtualBox VirtIO-GPU", 1,
                                  "VirtualBox", 1, pThis->uVkApiVersion };
    VkInstanceCreateInfo CreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &AppInfo, 0, NULL, 0, NULL };
    VkResult vkrc = pfnCreateInstance(&CreateInfo, NULL, &pThis->hVkInstance);
    if (vkrc != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevices =
        (PFN_vkEnumeratePhysicalDevices)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDeviceProperties =
        (PFN_vkGetPhysicalDeviceProperties)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetPhysicalDeviceProperties");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties pfnGetPhysicalDeviceQueueFamilyProperties =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetPhysicalDeviceQueueFamilyProperties");
    PFN_vkGetPhysicalDeviceMemoryProperties pfnGetPhysicalDeviceMemoryProperties =
        (PFN_vkGetPhysicalDeviceMemoryProperties)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetPhysicalDeviceMemoryProperties");
    if (!pfnEnumeratePhysicalDevices || !pfnGetPhysicalDeviceProperties || !pfnGetPhysicalDeviceQueueFamilyProperties
        || !pfnGetPhysicalDeviceMemoryProperties)
        return VERR_NOT_FOUND;
    uint32_t cDevices = 0;
    vkrc = pfnEnumeratePhysicalDevices(pThis->hVkInstance, &cDevices, NULL);
    if (vkrc != VK_SUCCESS || !cDevices)
        return VERR_NOT_SUPPORTED;
    VkPhysicalDevice aDevices[8];
    uint32_t cDevicesFetch = RT_MIN(cDevices, (uint32_t)RT_ELEMENTS(aDevices));
    vkrc = pfnEnumeratePhysicalDevices(pThis->hVkInstance, &cDevicesFetch, aDevices);
    if (vkrc != VK_SUCCESS || !cDevicesFetch)
        return VERR_NOT_SUPPORTED;
    pThis->hVkPhysicalDevice = aDevices[0];
    RT_ZERO(pThis->VkProperties);
    pfnGetPhysicalDeviceProperties(pThis->hVkPhysicalDevice, &pThis->VkProperties);
    pThis->fVulkanDevice = true;
    RT_ZERO(pThis->VkMemoryProperties);
    pfnGetPhysicalDeviceMemoryProperties(pThis->hVkPhysicalDevice, &pThis->VkMemoryProperties);
    pThis->fVulkanMemory = pThis->VkMemoryProperties.memoryTypeCount != 0;
    if (!pThis->fVulkanMemory)
        return VERR_NOT_SUPPORTED;
    uint32_t cQueueFamilies = 0;
    pfnGetPhysicalDeviceQueueFamilyProperties(pThis->hVkPhysicalDevice, &cQueueFamilies, NULL);
    VkQueueFamilyProperties aQueueFamilies[16];
    uint32_t cQueueFamiliesFetch = RT_MIN(cQueueFamilies, (uint32_t)RT_ELEMENTS(aQueueFamilies));
    pfnGetPhysicalDeviceQueueFamilyProperties(pThis->hVkPhysicalDevice, &cQueueFamiliesFetch, aQueueFamilies);
    for (uint32_t i = 0; i < cQueueFamiliesFetch; ++i)
        if ((aQueueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            pThis->uVkQueueFamily = i;
            break;
        }
    if (pThis->uVkQueueFamily == UINT32_MAX)
        return VERR_NOT_SUPPORTED;
    PFN_vkCreateDevice pfnCreateDevice =
        (PFN_vkCreateDevice)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkCreateDevice");
    if (!pfnCreateDevice)
        return VERR_NOT_FOUND;
    float const fQueuePriority = 1.0f;
    VkDeviceQueueCreateInfo QueueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL, 0,
                                          pThis->uVkQueueFamily, 1, &fQueuePriority };
    VkDeviceCreateInfo DeviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, NULL, 0, 1, &QueueInfo,
                                      0, NULL, 0, NULL, NULL };
    vkrc = pfnCreateDevice(pThis->hVkPhysicalDevice, &DeviceInfo, NULL, &pThis->hVkDevice);
    if (vkrc != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    PFN_vkGetDeviceQueue pfnGetDeviceQueue =
        (PFN_vkGetDeviceQueue)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceQueue");
    if (!pfnGetDeviceQueue)
        return VERR_NOT_FOUND;
    pfnGetDeviceQueue(pThis->hVkDevice, pThis->uVkQueueFamily, 0, &pThis->hVkQueue);
    pThis->fVulkanQueue = pThis->hVkQueue != VK_NULL_HANDLE;
    if (!pThis->fVulkanQueue)
        return VERR_NOT_SUPPORTED;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    PFN_vkCreateCommandPool pfnCreateCommandPool = pfnGetDeviceProcAddr
        ? (PFN_vkCreateCommandPool)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkCreateCommandPool") : NULL;
    PFN_vkAllocateCommandBuffers pfnAllocateCommandBuffers = pfnGetDeviceProcAddr
        ? (PFN_vkAllocateCommandBuffers)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkAllocateCommandBuffers") : NULL;
    PFN_vkCreateFence pfnCreateFence = pfnGetDeviceProcAddr
        ? (PFN_vkCreateFence)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkCreateFence") : NULL;
    if (!pfnCreateCommandPool || !pfnAllocateCommandBuffers || !pfnCreateFence)
        return VERR_NOT_FOUND;
    VkCommandPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, NULL,
                                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, pThis->uVkQueueFamily };
    if (pfnCreateCommandPool(pThis->hVkDevice, &PoolInfo, NULL, &pThis->hVkSubmitPool) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    VkCommandBufferAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL,
                                              pThis->hVkSubmitPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
    if (pfnAllocateCommandBuffers(pThis->hVkDevice, &AllocInfo, &pThis->hVkSubmitCommandBuffer) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    VkFenceCreateInfo FenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, 0 };
    if (pfnCreateFence(pThis->hVkDevice, &FenceInfo, NULL, &pThis->hVkSubmitFence) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pThis->fVulkanSubmit = true;
    VkDeviceSize cbDeviceLocal = 0;
    for (uint32_t i = 0; i < pThis->VkMemoryProperties.memoryHeapCount; ++i)
        if (pThis->VkMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            cbDeviceLocal += pThis->VkMemoryProperties.memoryHeaps[i].size;
    LogRel(("virtio-gpu: Vulkan host device '%s', API %u.%u.%u, memory types %u, device-local %llu MiB\n",
            pThis->VkProperties.deviceName,
            VK_VERSION_MAJOR(pThis->VkProperties.apiVersion), VK_VERSION_MINOR(pThis->VkProperties.apiVersion),
            VK_VERSION_PATCH(pThis->VkProperties.apiVersion), pThis->VkMemoryProperties.memoryTypeCount,
            (unsigned long long)(cbDeviceLocal / _1M)));
    return VINF_SUCCESS;
}

static void virtioGpuR3VulkanTerm(PVIRTIOGPU pThis)
{
    if (pThis->hVkDevice != VK_NULL_HANDLE && pThis->pfnVkGetInstanceProcAddr)
    {
        PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
            (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
        PFN_vkDestroyFence pfnDestroyFence = pfnGetDeviceProcAddr
            ? (PFN_vkDestroyFence)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkDestroyFence") : NULL;
        PFN_vkFreeCommandBuffers pfnFreeCommandBuffers = pfnGetDeviceProcAddr
            ? (PFN_vkFreeCommandBuffers)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkFreeCommandBuffers") : NULL;
        PFN_vkDestroyCommandPool pfnDestroyCommandPool = pfnGetDeviceProcAddr
            ? (PFN_vkDestroyCommandPool)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkDestroyCommandPool") : NULL;
        if (pfnDestroyFence && pThis->hVkSubmitFence != VK_NULL_HANDLE)
            pfnDestroyFence(pThis->hVkDevice, pThis->hVkSubmitFence, NULL);
        if (pfnFreeCommandBuffers && pThis->hVkSubmitCommandBuffer != VK_NULL_HANDLE
            && pThis->hVkSubmitPool != VK_NULL_HANDLE)
            pfnFreeCommandBuffers(pThis->hVkDevice, pThis->hVkSubmitPool, 1, &pThis->hVkSubmitCommandBuffer);
        if (pfnDestroyCommandPool && pThis->hVkSubmitPool != VK_NULL_HANDLE)
            pfnDestroyCommandPool(pThis->hVkDevice, pThis->hVkSubmitPool, NULL);
        pThis->hVkSubmitFence = VK_NULL_HANDLE;
        pThis->hVkSubmitCommandBuffer = VK_NULL_HANDLE;
        pThis->hVkSubmitPool = VK_NULL_HANDLE;
        pThis->fVulkanSubmit = false;
        PFN_vkDestroyDevice pfnDestroyDevice =
            (PFN_vkDestroyDevice)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkDestroyDevice");
        if (pfnDestroyDevice)
            pfnDestroyDevice(pThis->hVkDevice, NULL);
    }
    pThis->hVkDevice = VK_NULL_HANDLE;
    pThis->hVkQueue = VK_NULL_HANDLE;
    pThis->fVulkanQueue = false;
    if (pThis->hVkInstance != VK_NULL_HANDLE && pThis->pfnVkGetInstanceProcAddr)
    {
        PFN_vkDestroyInstance pfnDestroyInstance =
            (PFN_vkDestroyInstance)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkDestroyInstance");
        if (pfnDestroyInstance)
            pfnDestroyInstance(pThis->hVkInstance, NULL);
    }
    pThis->hVkInstance = VK_NULL_HANDLE;
    pThis->hVkPhysicalDevice = VK_NULL_HANDLE;
    if (pThis->hVulkan != NIL_RTLDRMOD)
        RTLdrClose(pThis->hVulkan);
    pThis->hVulkan = NIL_RTLDRMOD;
    pThis->pfnVkGetInstanceProcAddr = NULL;
    pThis->fVulkanLoader = false;
    pThis->fVulkanDevice = false;
    pThis->fVulkanMemory = false;
}

/** Executes one short command and reads it back to prove queue execution. */
static int virtioGpuR3VulkanProbeQueue(PVIRTIOGPU pThis)
{
    if (!pThis->fVulkanQueue || pThis->hVkDevice == VK_NULL_HANDLE)
        return VERR_NOT_SUPPORTED;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    PFN_vkGetPhysicalDeviceMemoryProperties pfnGetMemoryProperties =
        (PFN_vkGetPhysicalDeviceMemoryProperties)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance,
                                                                                    "vkGetPhysicalDeviceMemoryProperties");
    if (!pfnGetDeviceProcAddr || !pfnGetMemoryProperties)
        return VERR_NOT_FOUND;
# define VK_DEV_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkCreateCommandPool pfnCreateCommandPool = VK_DEV_PROC(PFN_vkCreateCommandPool, "vkCreateCommandPool");
    PFN_vkDestroyCommandPool pfnDestroyCommandPool = VK_DEV_PROC(PFN_vkDestroyCommandPool, "vkDestroyCommandPool");
    PFN_vkAllocateCommandBuffers pfnAllocateCommandBuffers = VK_DEV_PROC(PFN_vkAllocateCommandBuffers, "vkAllocateCommandBuffers");
    PFN_vkFreeCommandBuffers pfnFreeCommandBuffers = VK_DEV_PROC(PFN_vkFreeCommandBuffers, "vkFreeCommandBuffers");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_DEV_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_DEV_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdFillBuffer pfnCmdFillBuffer = VK_DEV_PROC(PFN_vkCmdFillBuffer, "vkCmdFillBuffer");
    PFN_vkCreateBuffer pfnCreateBuffer = VK_DEV_PROC(PFN_vkCreateBuffer, "vkCreateBuffer");
    PFN_vkDestroyBuffer pfnDestroyBuffer = VK_DEV_PROC(PFN_vkDestroyBuffer, "vkDestroyBuffer");
    PFN_vkGetBufferMemoryRequirements pfnGetBufferMemoryRequirements = VK_DEV_PROC(PFN_vkGetBufferMemoryRequirements, "vkGetBufferMemoryRequirements");
    PFN_vkAllocateMemory pfnAllocateMemory = VK_DEV_PROC(PFN_vkAllocateMemory, "vkAllocateMemory");
    PFN_vkFreeMemory pfnFreeMemory = VK_DEV_PROC(PFN_vkFreeMemory, "vkFreeMemory");
    PFN_vkBindBufferMemory pfnBindBufferMemory = VK_DEV_PROC(PFN_vkBindBufferMemory, "vkBindBufferMemory");
    PFN_vkMapMemory pfnMapMemory = VK_DEV_PROC(PFN_vkMapMemory, "vkMapMemory");
    PFN_vkUnmapMemory pfnUnmapMemory = VK_DEV_PROC(PFN_vkUnmapMemory, "vkUnmapMemory");
    PFN_vkCreateFence pfnCreateFence = VK_DEV_PROC(PFN_vkCreateFence, "vkCreateFence");
    PFN_vkDestroyFence pfnDestroyFence = VK_DEV_PROC(PFN_vkDestroyFence, "vkDestroyFence");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_DEV_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_DEV_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnCreateCommandPool || !pfnDestroyCommandPool || !pfnAllocateCommandBuffers || !pfnFreeCommandBuffers
        || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdFillBuffer || !pfnCreateBuffer
        || !pfnDestroyBuffer || !pfnGetBufferMemoryRequirements || !pfnAllocateMemory || !pfnFreeMemory
        || !pfnBindBufferMemory || !pfnMapMemory || !pfnUnmapMemory || !pfnCreateFence || !pfnDestroyFence
        || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandPool hPool = VK_NULL_HANDLE;
    VkCommandBuffer hCmd = VK_NULL_HANDLE;
    VkBuffer hBuffer = VK_NULL_HANDLE;
    VkDeviceMemory hMemory = VK_NULL_HANDLE;
    VkFence hFence = VK_NULL_HANDLE;
    int rc = VERR_NOT_SUPPORTED;
    VkMemoryAllocateInfo AllocInfo;
    VkCommandBufferAllocateInfo CmdAllocInfo;
    VkCommandBufferBeginInfo BeginInfo;
    VkFenceCreateInfo FenceInfo;
    VkSubmitInfo SubmitInfo;
    RT_ZERO(AllocInfo);
    RT_ZERO(CmdAllocInfo);
    RT_ZERO(BeginInfo);
    RT_ZERO(FenceInfo);
    RT_ZERO(SubmitInfo);
    VkCommandPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, NULL, 0, pThis->uVkQueueFamily };
    VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0, sizeof(uint32_t),
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, NULL };
    VkMemoryRequirements MemReq;
    VkPhysicalDeviceMemoryProperties MemProps;
    RT_ZERO(MemReq);
    RT_ZERO(MemProps);
    if (pfnCreateCommandPool(pThis->hVkDevice, &PoolInfo, NULL, &hPool) != VK_SUCCESS
        || pfnCreateBuffer(pThis->hVkDevice, &BufferInfo, NULL, &hBuffer) != VK_SUCCESS)
        goto cleanup;
    pfnGetBufferMemoryRequirements(pThis->hVkDevice, hBuffer, &MemReq);
    pfnGetMemoryProperties(pThis->hVkPhysicalDevice, &MemProps);
    uint32_t iMemoryType = UINT32_MAX;
    for (uint32_t i = 0; i < MemProps.memoryTypeCount; ++i)
        if ((MemReq.memoryTypeBits & RT_BIT_32(i))
            && (MemProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            iMemoryType = i;
            break;
        }
    if (iMemoryType == UINT32_MAX)
        goto cleanup;
    AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, MemReq.size, iMemoryType };
    if (pfnAllocateMemory(pThis->hVkDevice, &AllocInfo, NULL, &hMemory) != VK_SUCCESS
        || pfnBindBufferMemory(pThis->hVkDevice, hBuffer, hMemory, 0) != VK_SUCCESS)
        goto cleanup;
    CmdAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL, hPool,
                     VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
    if (pfnAllocateCommandBuffers(pThis->hVkDevice, &CmdAllocInfo, &hCmd) != VK_SUCCESS)
        goto cleanup;
    BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    if (pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        goto cleanup;
    pfnCmdFillBuffer(hCmd, hBuffer, 0, sizeof(uint32_t), UINT32_C(0xa5a5a5a5));
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS)
        goto cleanup;
    FenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, 0 };
    if (pfnCreateFence(pThis->hVkDevice, &FenceInfo, NULL, &hFence) != VK_SUCCESS)
        goto cleanup;
    SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        goto cleanup;
    void *pv = NULL;
    if (pfnMapMemory(pThis->hVkDevice, hMemory, 0, sizeof(uint32_t), 0, &pv) == VK_SUCCESS)
    {
        rc = *(uint32_t *)pv == UINT32_C(0xa5a5a5a5) ? VINF_SUCCESS : VERR_MISMATCH;
        pfnUnmapMemory(pThis->hVkDevice, hMemory);
    }
cleanup:
    if (hFence != VK_NULL_HANDLE) pfnDestroyFence(pThis->hVkDevice, hFence, NULL);
    if (hCmd != VK_NULL_HANDLE) pfnFreeCommandBuffers(pThis->hVkDevice, hPool, 1, &hCmd);
    if (hMemory != VK_NULL_HANDLE) pfnFreeMemory(pThis->hVkDevice, hMemory, NULL);
    if (hBuffer != VK_NULL_HANDLE) pfnDestroyBuffer(pThis->hVkDevice, hBuffer, NULL);
    if (hPool != VK_NULL_HANDLE) pfnDestroyCommandPool(pThis->hVkDevice, hPool, NULL);
    return rc;
# undef VK_DEV_PROC
}

static int virtioGpuR3VulkanImageCreate(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    if (pRes->fBlob || !pThis->fVulkanMemory || !pRes->uWidth || !pRes->uHeight)
        return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_IMAGE_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkCreateImage pfnCreateImage = VK_IMAGE_PROC(PFN_vkCreateImage, "vkCreateImage");
    PFN_vkDestroyImage pfnDestroyImage = VK_IMAGE_PROC(PFN_vkDestroyImage, "vkDestroyImage");
    PFN_vkGetImageMemoryRequirements pfnGetRequirements = VK_IMAGE_PROC(PFN_vkGetImageMemoryRequirements, "vkGetImageMemoryRequirements");
    PFN_vkAllocateMemory pfnAllocateMemory = VK_IMAGE_PROC(PFN_vkAllocateMemory, "vkAllocateMemory");
    PFN_vkFreeMemory pfnFreeMemory = VK_IMAGE_PROC(PFN_vkFreeMemory, "vkFreeMemory");
    PFN_vkBindImageMemory pfnBindImageMemory = VK_IMAGE_PROC(PFN_vkBindImageMemory, "vkBindImageMemory");
    if (!pfnCreateImage || !pfnDestroyImage || !pfnGetRequirements || !pfnAllocateMemory || !pfnFreeMemory
        || !pfnBindImageMemory)
        return VERR_NOT_FOUND;
    VkImageCreateInfo ImageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, NULL, 0, VK_IMAGE_TYPE_2D,
                                    VK_FORMAT_B8G8R8A8_UNORM, { pRes->uWidth, pRes->uHeight, 1 }, 1, 1,
                                    VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                    | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    VK_SHARING_MODE_EXCLUSIVE, 0, NULL, VK_IMAGE_LAYOUT_UNDEFINED };
    if (pfnCreateImage(pThis->hVkDevice, &ImageInfo, NULL, &pRes->hVkImage) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    VkMemoryRequirements MemReq;
    RT_ZERO(MemReq);
    pfnGetRequirements(pThis->hVkDevice, pRes->hVkImage, &MemReq);
    uint32_t iMemoryType = UINT32_MAX;
    uint32_t iFallbackMemoryType = UINT32_MAX;
    for (uint32_t i = 0; i < pThis->VkMemoryProperties.memoryTypeCount; ++i)
        if (MemReq.memoryTypeBits & RT_BIT_32(i))
        {
            if (iFallbackMemoryType == UINT32_MAX)
                iFallbackMemoryType = i;
            if (pThis->VkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                iMemoryType = i;
                break;
            }
        }
    if (iMemoryType == UINT32_MAX)
        iMemoryType = iFallbackMemoryType;
    if (iMemoryType == UINT32_MAX)
    {
        pfnDestroyImage(pThis->hVkDevice, pRes->hVkImage, NULL);
        pRes->hVkImage = VK_NULL_HANDLE;
        return VERR_NOT_SUPPORTED;
    }
    VkMemoryAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, MemReq.size, iMemoryType };
    if (pfnAllocateMemory(pThis->hVkDevice, &AllocInfo, NULL, &pRes->hVkImageMemory) != VK_SUCCESS
        || pfnBindImageMemory(pThis->hVkDevice, pRes->hVkImage, pRes->hVkImageMemory, 0) != VK_SUCCESS)
    {
        if (pRes->hVkImageMemory != VK_NULL_HANDLE)
            pfnFreeMemory(pThis->hVkDevice, pRes->hVkImageMemory, NULL);
        pfnDestroyImage(pThis->hVkDevice, pRes->hVkImage, NULL);
        pRes->hVkImageMemory = VK_NULL_HANDLE;
        pRes->hVkImage = VK_NULL_HANDLE;
        return VERR_NOT_SUPPORTED;
    }
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pRes->fVulkanImage = true;
    return VINF_SUCCESS;
# undef VK_IMAGE_PROC
}

static int virtioGpuR3VulkanResourceCreate(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    pRes->hVkBuffer = VK_NULL_HANDLE;
    pRes->hVkMemory = VK_NULL_HANDLE;
    pRes->pvVkMapped = NULL;
    pRes->hVkImage = VK_NULL_HANDLE;
    pRes->hVkImageMemory = VK_NULL_HANDLE;
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pRes->fVulkanBuffer = false;
    pRes->fVulkanImage = false;
    pRes->fVulkanImageDirty = false;
    if (!pThis->fVulkanMemory || pThis->hVkDevice == VK_NULL_HANDLE)
        return VINF_SUCCESS;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_RES_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkCreateBuffer pfnCreateBuffer = VK_RES_PROC(PFN_vkCreateBuffer, "vkCreateBuffer");
    PFN_vkDestroyBuffer pfnDestroyBuffer = VK_RES_PROC(PFN_vkDestroyBuffer, "vkDestroyBuffer");
    PFN_vkGetBufferMemoryRequirements pfnGetRequirements = VK_RES_PROC(PFN_vkGetBufferMemoryRequirements, "vkGetBufferMemoryRequirements");
    PFN_vkAllocateMemory pfnAllocateMemory = VK_RES_PROC(PFN_vkAllocateMemory, "vkAllocateMemory");
    PFN_vkFreeMemory pfnFreeMemory = VK_RES_PROC(PFN_vkFreeMemory, "vkFreeMemory");
    PFN_vkBindBufferMemory pfnBindBufferMemory = VK_RES_PROC(PFN_vkBindBufferMemory, "vkBindBufferMemory");
    PFN_vkMapMemory pfnMapMemory = VK_RES_PROC(PFN_vkMapMemory, "vkMapMemory");
    PFN_vkUnmapMemory pfnUnmapMemory = VK_RES_PROC(PFN_vkUnmapMemory, "vkUnmapMemory");
    if (!pfnCreateBuffer || !pfnDestroyBuffer || !pfnGetRequirements || !pfnAllocateMemory || !pfnFreeMemory
        || !pfnBindBufferMemory || !pfnMapMemory || !pfnUnmapMemory)
        return VERR_NOT_FOUND;
    VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0, pRes->cbPixels,
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                      | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                      | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                      VK_SHARING_MODE_EXCLUSIVE, 0, NULL };
    VkMemoryRequirements MemReq;
    RT_ZERO(MemReq);
    VkMemoryAllocateInfo AllocInfo;
    RT_ZERO(AllocInfo);
    if (pfnCreateBuffer(pThis->hVkDevice, &BufferInfo, NULL, &pRes->hVkBuffer) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnGetRequirements(pThis->hVkDevice, pRes->hVkBuffer, &MemReq);
    uint32_t iMemoryType = UINT32_MAX;
    uint32_t iFallbackMemoryType = UINT32_MAX;
    for (uint32_t i = 0; i < pThis->VkMemoryProperties.memoryTypeCount; ++i)
        if ((MemReq.memoryTypeBits & RT_BIT_32(i))
            && (pThis->VkMemoryProperties.memoryTypes[i].propertyFlags
                & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            if (iFallbackMemoryType == UINT32_MAX)
                iFallbackMemoryType = i;
            if (pThis->VkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                iMemoryType = i;
                break;
            }
        }
    if (iMemoryType == UINT32_MAX)
        iMemoryType = iFallbackMemoryType;
    if (iMemoryType == UINT32_MAX)
        goto resource_cleanup;
    AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, MemReq.size, iMemoryType };
    if (pfnAllocateMemory(pThis->hVkDevice, &AllocInfo, NULL, &pRes->hVkMemory) != VK_SUCCESS
        || pfnBindBufferMemory(pThis->hVkDevice, pRes->hVkBuffer, pRes->hVkMemory, 0) != VK_SUCCESS)
        goto resource_cleanup;
    if (pfnMapMemory(pThis->hVkDevice, pRes->hVkMemory, 0, VK_WHOLE_SIZE, 0, &pRes->pvVkMapped) != VK_SUCCESS)
        goto resource_cleanup;
    pRes->fVulkanBuffer = true;
    memcpy(pRes->pvVkMapped, pRes->pbPixels, (size_t)pRes->cbPixels);
    if (!pRes->fBlob)
    {
        int const rcImage = virtioGpuR3VulkanImageCreate(pThis, pRes);
        if (RT_FAILURE(rcImage))
            LogRel2(("virtio-gpu: image backing unavailable for resource %u (%Rrc); using buffer backing\n",
                     pRes->uResourceId, rcImage));
        else
        {
            int const rcSync = virtioGpuR3VulkanResourceSync(pThis, pRes);
            if (RT_FAILURE(rcSync))
            {
                LogRel2(("virtio-gpu: image initialization failed for resource %u (%Rrc); using buffer backing\n",
                         pRes->uResourceId, rcSync));
                virtioGpuR3VulkanResourceDestroy(pThis, pRes);
            }
        }
    }
    return VINF_SUCCESS;
resource_cleanup:
    if (pRes->pvVkMapped != NULL && pfnUnmapMemory)
        pfnUnmapMemory(pThis->hVkDevice, pRes->hVkMemory);
    if (pRes->hVkMemory != VK_NULL_HANDLE)
        pfnFreeMemory(pThis->hVkDevice, pRes->hVkMemory, NULL);
    if (pRes->hVkBuffer != VK_NULL_HANDLE)
        pfnDestroyBuffer(pThis->hVkDevice, pRes->hVkBuffer, NULL);
    pRes->hVkMemory = VK_NULL_HANDLE;
    pRes->hVkBuffer = VK_NULL_HANDLE;
    pRes->pvVkMapped = NULL;
    return VERR_NOT_SUPPORTED;
# undef VK_RES_PROC
}

static void virtioGpuR3VulkanResourceDestroy(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    if (!pRes->fVulkanBuffer || pThis->hVkDevice == VK_NULL_HANDLE || !pThis->pfnVkGetInstanceProcAddr)
        return;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (pfnGetDeviceProcAddr)
    {
        PFN_vkDestroyBuffer pfnDestroyBuffer = (PFN_vkDestroyBuffer)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkDestroyBuffer");
        PFN_vkDestroyImage pfnDestroyImage = (PFN_vkDestroyImage)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkDestroyImage");
        PFN_vkFreeMemory pfnFreeMemory = (PFN_vkFreeMemory)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkFreeMemory");
        PFN_vkUnmapMemory pfnUnmapMemory = (PFN_vkUnmapMemory)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkUnmapMemory");
        if (pRes->pvVkMapped != NULL && pfnUnmapMemory)
            pfnUnmapMemory(pThis->hVkDevice, pRes->hVkMemory);
        if (pRes->hVkBuffer != VK_NULL_HANDLE && pfnDestroyBuffer)
            pfnDestroyBuffer(pThis->hVkDevice, pRes->hVkBuffer, NULL);
        if (pRes->hVkImage != VK_NULL_HANDLE && pfnDestroyImage)
            pfnDestroyImage(pThis->hVkDevice, pRes->hVkImage, NULL);
        if (pRes->hVkMemory != VK_NULL_HANDLE && pfnFreeMemory)
            pfnFreeMemory(pThis->hVkDevice, pRes->hVkMemory, NULL);
        if (pRes->hVkImageMemory != VK_NULL_HANDLE && pfnFreeMemory)
            pfnFreeMemory(pThis->hVkDevice, pRes->hVkImageMemory, NULL);
    }
    pRes->hVkBuffer = VK_NULL_HANDLE;
    pRes->hVkMemory = VK_NULL_HANDLE;
    pRes->pvVkMapped = NULL;
    pRes->hVkImage = VK_NULL_HANDLE;
    pRes->hVkImageMemory = VK_NULL_HANDLE;
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pRes->fVulkanBuffer = false;
    pRes->fVulkanImage = false;
}

static int virtioGpuR3VulkanResourceSyncImage(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    if (!pRes->fVulkanImage || !pRes->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit)
        return VINF_SUCCESS;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_IMAGE_SYNC_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_IMAGE_SYNC_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_IMAGE_SYNC_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_IMAGE_SYNC_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_IMAGE_SYNC_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdCopyBufferToImage pfnCmdCopyBufferToImage = VK_IMAGE_SYNC_PROC(PFN_vkCmdCopyBufferToImage, "vkCmdCopyBufferToImage");
    PFN_vkResetFences pfnResetFences = VK_IMAGE_SYNC_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_IMAGE_SYNC_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_IMAGE_SYNC_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdCopyBufferToImage || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkAccessFlags fSrcAccess = pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED ? (VkAccessFlags)0
                             : pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                             ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags fSrcStage = pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                   ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkImageMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                     fSrcAccess,
                                     VK_ACCESS_TRANSFER_WRITE_BIT, pRes->enmVkImageLayout,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED, pRes->hVkImage,
                                     { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkBufferImageCopy Region = { 0, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                                 { 0, 0, 0 }, { pRes->uWidth, pRes->uHeight, 1 } };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, fSrcStage,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &Barrier);
    pfnCmdCopyBufferToImage(hCmd, pRes->hVkBuffer, pRes->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    pRes->fVulkanImageDirty = false;
    return VINF_SUCCESS;
# undef VK_IMAGE_SYNC_PROC
}

static int virtioGpuR3VulkanResourceReadbackImage(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    if (!pRes->fVulkanImage || !pRes->fVulkanBuffer || pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        return VINF_SUCCESS;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_IMAGE_READBACK_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_IMAGE_READBACK_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_IMAGE_READBACK_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_IMAGE_READBACK_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_IMAGE_READBACK_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdCopyImageToBuffer pfnCmdCopyImageToBuffer = VK_IMAGE_READBACK_PROC(PFN_vkCmdCopyImageToBuffer, "vkCmdCopyImageToBuffer");
    PFN_vkResetFences pfnResetFences = VK_IMAGE_READBACK_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_IMAGE_READBACK_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_IMAGE_READBACK_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdCopyImageToBuffer || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkAccessFlags fSrcAccess = pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                             ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
    VkImageMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                     fSrcAccess, VK_ACCESS_TRANSFER_READ_BIT,
                                     pRes->enmVkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, pRes->hVkImage,
                                     { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkBufferImageCopy Region = { 0, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                                 { 0, 0, 0 }, { pRes->uWidth, pRes->uHeight, 1 } };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                          0, NULL, 0, NULL, 1, &Barrier);
    pfnCmdCopyImageToBuffer(hCmd, pRes->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            pRes->hVkBuffer, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    memcpy(pRes->pbPixels, pRes->pvVkMapped, (size_t)pRes->cbPixels);
    pRes->fVulkanImageDirty = false;
    return VINF_SUCCESS;
# undef VK_IMAGE_READBACK_PROC
}

static int virtioGpuR3VulkanResourceEnsureBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    return pRes->fVulkanImageDirty ? virtioGpuR3VulkanResourceReadbackImage(pThis, pRes) : VINF_SUCCESS;
}

static int virtioGpuR3VulkanResourceSync(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    if (!pRes->fVulkanBuffer || !pRes->pbPixels || !pRes->pvVkMapped)
        return VINF_SUCCESS;
    memcpy(pRes->pvVkMapped, pRes->pbPixels, (size_t)pRes->cbPixels);
    return virtioGpuR3VulkanResourceSyncImage(pThis, pRes);
}

static int virtioGpuR3VulkanFillBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                       uint64_t off, uint64_t cb, uint32_t uData)
{
    if (!pRes->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit || off > pRes->cbPixels
        || cb > pRes->cbPixels - off || !cb || (off & 3) || (cb & 3))
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pRes);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_FILL_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_FILL_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_FILL_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_FILL_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdFillBuffer pfnCmdFillBuffer = VK_FILL_PROC(PFN_vkCmdFillBuffer, "vkCmdFillBuffer");
    PFN_vkResetFences pfnResetFences = VK_FILL_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_FILL_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_FILL_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdFillBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    int rc = VERR_NOT_SUPPORTED;
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS)
        goto cleanup;
    if (pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        goto cleanup;
    pfnCmdFillBuffer(hCmd, pRes->hVkBuffer, off, cb, uData);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        goto cleanup;
    if (*(uint32_t *)((uint8_t *)pRes->pvVkMapped + off) != uData)
    {
        goto cleanup;
    }
    memcpy(pRes->pbPixels + off, (uint8_t *)pRes->pvVkMapped + off, (size_t)cb);
    rc = virtioGpuR3VulkanResourceSyncImage(pThis, pRes);
cleanup:
    return rc;
# undef VK_FILL_PROC
}

static int virtioGpuR3VulkanCopyBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                       PVIRTIOGPURESOURCE pDst, uint64_t offSrc,
                                       uint64_t offDst, uint64_t cbCopy)
{
    if (!pSrc->fVulkanBuffer || !pDst->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit
        || offSrc > pSrc->cbPixels || cbCopy > pSrc->cbPixels - offSrc
        || offDst > pDst->cbPixels || cbCopy > pDst->cbPixels - offDst || !cbCopy)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pSrc);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pDst);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_COPY_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_COPY_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_COPY_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_COPY_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdCopyBuffer pfnCmdCopyBuffer = VK_COPY_PROC(PFN_vkCmdCopyBuffer, "vkCmdCopyBuffer");
    PFN_vkResetFences pfnResetFences = VK_COPY_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_COPY_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_COPY_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdCopyBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkBufferCopy Region = { offSrc, offDst, cbCopy };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdCopyBuffer(hCmd, pSrc->hVkBuffer, pDst->hVkBuffer, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    memcpy((uint8_t *)pDst->pvVkMapped + offDst, (uint8_t *)pSrc->pvVkMapped + offSrc, (size_t)cbCopy);
    memcpy(pDst->pbPixels + offDst, (uint8_t *)pDst->pvVkMapped + offDst, (size_t)cbCopy);
    return virtioGpuR3VulkanResourceSyncImage(pThis, pDst);
# undef VK_COPY_PROC
}

typedef struct VIRTIOGPUFILLCMD
{
    uint64_t uCommandBuffer;
    uint64_t uBuffer;
    uint64_t offBuffer;
    uint64_t cbBuffer;
    uint32_t uData;
} VIRTIOGPUFILLCMD;

typedef struct VIRTIOGPUCOPYCMD
{
    uint64_t uCommandBuffer;
    uint64_t uSrcBuffer;
    uint64_t uDstBuffer;
    uint32_t cRegions;
    uint64_t cbRegions;
    const uint8_t *pbRegions;
    uint64_t offSrc;
    uint64_t offDst;
    uint64_t cbCopy;
} VIRTIOGPUCOPYCMD;

typedef struct VIRTIOGPUUPDATECMD
{
    uint64_t uCommandBuffer;
    uint64_t uBuffer;
    uint64_t offBuffer;
    uint64_t cbData;
    const uint8_t *pbData;
} VIRTIOGPUUPDATECMD;

typedef struct VIRTIOGPUCLEARCOLORCMD
{
    uint64_t uCommandBuffer;
    uint64_t uImage;
    uint32_t enmImageLayout;
    uint32_t auColor[4];
    uint32_t cRanges;
    uint64_t cbRanges;
    const uint8_t *pbRanges;
} VIRTIOGPUCLEARCOLORCMD;

typedef struct VIRTIOGPUCOPYBUFFERTOIMAGECMD
{
    uint64_t uCommandBuffer;
    uint64_t uSrcBuffer;
    uint64_t uDstImage;
    uint32_t enmDstImageLayout;
    uint32_t cRegions;
    uint64_t cbRegions;
    const uint8_t *pbRegions;
} VIRTIOGPUCOPYBUFFERTOIMAGECMD;

typedef struct VIRTIOGPUCOPYIMAGETOBUFFERCMD
{
    uint64_t uCommandBuffer;
    uint64_t uSrcImage;
    uint32_t enmSrcImageLayout;
    uint64_t uDstBuffer;
    uint32_t cRegions;
    uint64_t cbRegions;
    const uint8_t *pbRegions;
} VIRTIOGPUCOPYIMAGETOBUFFERCMD;

typedef struct VIRTIOGPUPIPELINEBARRIERCMD
{
    uint64_t uCommandBuffer;
    uint32_t fSrcStage;
    uint32_t fDstStage;
    uint32_t fDependency;
    uint32_t cMemoryBarriers;
    uint32_t cBufferBarriers;
    uint32_t cImageBarriers;
    const uint8_t *pbImageBarrier;
} VIRTIOGPUPIPELINEBARRIERCMD;

typedef struct VIRTIOGPUCOPYIMAGECMD
{
    uint64_t uCommandBuffer;
    uint64_t uSrcImage;
    uint32_t enmSrcImageLayout;
    uint64_t uDstImage;
    uint32_t enmDstImageLayout;
    uint32_t cRegions;
    uint64_t cbRegions;
    const uint8_t *pbRegions;
} VIRTIOGPUCOPYIMAGECMD;

static bool virtioGpuR3DecodeFillBuffer(const uint8_t *pbCommand, size_t cbCommand, VIRTIOGPUFILLCMD *pFill)
{
    if (!pbCommand || !pFill || cbCommand != 44)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_FILL_BUFFER || fCommand)
        return false;
    memcpy(&pFill->uCommandBuffer, pbCommand + 8, sizeof(pFill->uCommandBuffer));
    memcpy(&pFill->uBuffer, pbCommand + 16, sizeof(pFill->uBuffer));
    memcpy(&pFill->offBuffer, pbCommand + 24, sizeof(pFill->offBuffer));
    memcpy(&pFill->cbBuffer, pbCommand + 32, sizeof(pFill->cbBuffer));
    memcpy(&pFill->uData, pbCommand + 40, sizeof(pFill->uData));
    return pFill->uCommandBuffer != 0 && pFill->uBuffer != 0;
}

static bool virtioGpuR3DecodeCopyBuffer(const uint8_t *pbCommand, size_t cbCommand, VIRTIOGPUCOPYCMD *pCopy)
{
    if (!pbCommand || !pCopy || cbCommand < 68)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_COPY_BUFFER || fCommand)
        return false;
    memcpy(&pCopy->uCommandBuffer, pbCommand + 8, sizeof(pCopy->uCommandBuffer));
    memcpy(&pCopy->uSrcBuffer, pbCommand + 16, sizeof(pCopy->uSrcBuffer));
    memcpy(&pCopy->uDstBuffer, pbCommand + 24, sizeof(pCopy->uDstBuffer));
    memcpy(&pCopy->cRegions, pbCommand + 32, sizeof(pCopy->cRegions));
    memcpy(&pCopy->cbRegions, pbCommand + 36, sizeof(pCopy->cbRegions));
    if (!pCopy->cRegions || pCopy->cRegions > 256 || pCopy->cbRegions != (uint64_t)pCopy->cRegions * 24
        || cbCommand != 44 + (size_t)pCopy->cbRegions)
        return false;
    pCopy->pbRegions = pbCommand + 44;
    memcpy(&pCopy->offSrc, pCopy->pbRegions + 0, sizeof(pCopy->offSrc));
    memcpy(&pCopy->offDst, pCopy->pbRegions + 8, sizeof(pCopy->offDst));
    memcpy(&pCopy->cbCopy, pCopy->pbRegions + 16, sizeof(pCopy->cbCopy));
    return pCopy->uCommandBuffer != 0 && pCopy->uSrcBuffer != 0 && pCopy->uDstBuffer != 0
        && pCopy->cbCopy != 0;
}

static bool virtioGpuR3DecodeUpdateBuffer(const uint8_t *pbCommand, size_t cbCommand, VIRTIOGPUUPDATECMD *pUpdate)
{
    if (!pbCommand || !pUpdate || cbCommand < 48)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    uint64_t cbArray = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_UPDATE_BUFFER || fCommand)
        return false;
    memcpy(&pUpdate->uCommandBuffer, pbCommand + 8, sizeof(pUpdate->uCommandBuffer));
    memcpy(&pUpdate->uBuffer, pbCommand + 16, sizeof(pUpdate->uBuffer));
    memcpy(&pUpdate->offBuffer, pbCommand + 24, sizeof(pUpdate->offBuffer));
    memcpy(&pUpdate->cbData, pbCommand + 32, sizeof(pUpdate->cbData));
    memcpy(&cbArray, pbCommand + 40, sizeof(cbArray));
    if (!pUpdate->uCommandBuffer || !pUpdate->uBuffer || !pUpdate->cbData || pUpdate->cbData > UINT32_C(65536)
        || (pUpdate->cbData & 3) || cbArray != pUpdate->cbData
        || cbCommand != 48 + (size_t)((pUpdate->cbData + 3) & ~UINT64_C(3)))
        return false;
    pUpdate->pbData = pbCommand + 48;
    return true;
}

static bool virtioGpuR3DecodeClearColorImage(const uint8_t *pbCommand, size_t cbCommand,
                                             VIRTIOGPUCLEARCOLORCMD *pClear)
{
    if (!pbCommand || !pClear || cbCommand < 84)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    uint64_t cbColor = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_CLEAR_COLOR_IMAGE || fCommand)
        return false;
    memcpy(&pClear->uCommandBuffer, pbCommand + 8, sizeof(pClear->uCommandBuffer));
    memcpy(&pClear->uImage, pbCommand + 16, sizeof(pClear->uImage));
    memcpy(&pClear->enmImageLayout, pbCommand + 24, sizeof(pClear->enmImageLayout));
    memcpy(&cbColor, pbCommand + 28, sizeof(cbColor));
    if (cbColor != 1)
        return false;
    memcpy(pClear->auColor, pbCommand + 36, sizeof(pClear->auColor));
    memcpy(&pClear->cRanges, pbCommand + 52, sizeof(pClear->cRanges));
    memcpy(&pClear->cbRanges, pbCommand + 56, sizeof(pClear->cbRanges));
    if (!pClear->uCommandBuffer || !pClear->uImage || pClear->cRanges != 1
        || pClear->cbRanges != 20 || cbCommand != 84)
        return false;
    pClear->pbRanges = pbCommand + 64;
    return true;
}

static bool virtioGpuR3DecodeCopyBufferToImage(const uint8_t *pbCommand, size_t cbCommand,
                                               VIRTIOGPUCOPYBUFFERTOIMAGECMD *pCopy)
{
    if (!pbCommand || !pCopy || cbCommand < 104)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_COPY_BUFFER_TO_IMAGE || fCommand)
        return false;
    memcpy(&pCopy->uCommandBuffer, pbCommand + 8, sizeof(pCopy->uCommandBuffer));
    memcpy(&pCopy->uSrcBuffer, pbCommand + 16, sizeof(pCopy->uSrcBuffer));
    memcpy(&pCopy->uDstImage, pbCommand + 24, sizeof(pCopy->uDstImage));
    memcpy(&pCopy->enmDstImageLayout, pbCommand + 32, sizeof(pCopy->enmDstImageLayout));
    memcpy(&pCopy->cRegions, pbCommand + 36, sizeof(pCopy->cRegions));
    memcpy(&pCopy->cbRegions, pbCommand + 40, sizeof(pCopy->cbRegions));
    if (!pCopy->uCommandBuffer || !pCopy->uSrcBuffer || !pCopy->uDstImage || pCopy->cRegions != 1
        || pCopy->cbRegions != 56 || cbCommand != 48 + (size_t)pCopy->cbRegions)
        return false;
    pCopy->pbRegions = pbCommand + 48;
    return true;
}

static bool virtioGpuR3DecodeCopyImageToBuffer(const uint8_t *pbCommand, size_t cbCommand,
                                               VIRTIOGPUCOPYIMAGETOBUFFERCMD *pCopy)
{
    if (!pbCommand || !pCopy || cbCommand < 104)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_COPY_IMAGE_TO_BUFFER || fCommand)
        return false;
    memcpy(&pCopy->uCommandBuffer, pbCommand + 8, sizeof(pCopy->uCommandBuffer));
    memcpy(&pCopy->uSrcImage, pbCommand + 16, sizeof(pCopy->uSrcImage));
    memcpy(&pCopy->enmSrcImageLayout, pbCommand + 24, sizeof(pCopy->enmSrcImageLayout));
    memcpy(&pCopy->uDstBuffer, pbCommand + 28, sizeof(pCopy->uDstBuffer));
    memcpy(&pCopy->cRegions, pbCommand + 36, sizeof(pCopy->cRegions));
    memcpy(&pCopy->cbRegions, pbCommand + 40, sizeof(pCopy->cbRegions));
    if (!pCopy->uCommandBuffer || !pCopy->uSrcImage || !pCopy->uDstBuffer || pCopy->cRegions != 1
        || pCopy->cbRegions != 56 || cbCommand != 48 + (size_t)pCopy->cbRegions)
        return false;
    pCopy->pbRegions = pbCommand + 48;
    return true;
}

static bool virtioGpuR3DecodePipelineBarrier(const uint8_t *pbCommand, size_t cbCommand,
                                             VIRTIOGPUPIPELINEBARRIERCMD *pBarrier)
{
    if (!pbCommand || !pBarrier || cbCommand != 128)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    uint64_t cbMemory = 0;
    uint64_t cbBuffer = 0;
    uint64_t cbImage = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_PIPELINE_BARRIER || fCommand)
        return false;
    memcpy(&pBarrier->uCommandBuffer, pbCommand + 8, sizeof(pBarrier->uCommandBuffer));
    memcpy(&pBarrier->fSrcStage, pbCommand + 16, sizeof(pBarrier->fSrcStage));
    memcpy(&pBarrier->fDstStage, pbCommand + 20, sizeof(pBarrier->fDstStage));
    memcpy(&pBarrier->fDependency, pbCommand + 24, sizeof(pBarrier->fDependency));
    memcpy(&pBarrier->cMemoryBarriers, pbCommand + 28, sizeof(pBarrier->cMemoryBarriers));
    memcpy(&cbMemory, pbCommand + 32, sizeof(cbMemory));
    memcpy(&pBarrier->cBufferBarriers, pbCommand + 40, sizeof(pBarrier->cBufferBarriers));
    memcpy(&cbBuffer, pbCommand + 44, sizeof(cbBuffer));
    memcpy(&pBarrier->cImageBarriers, pbCommand + 52, sizeof(pBarrier->cImageBarriers));
    memcpy(&cbImage, pbCommand + 56, sizeof(cbImage));
    if (!pBarrier->uCommandBuffer || pBarrier->cMemoryBarriers || cbMemory || pBarrier->cBufferBarriers
        || cbBuffer || pBarrier->cImageBarriers != 1 || cbImage != 64)
        return false;
    pBarrier->pbImageBarrier = pbCommand + 64;
    return true;
}

static bool virtioGpuR3DecodeCopyImage(const uint8_t *pbCommand, size_t cbCommand,
                                       VIRTIOGPUCOPYIMAGECMD *pCopy)
{
    if (!pbCommand || !pCopy || cbCommand != 120)
        return false;
    uint32_t uCommandType = 0;
    uint32_t fCommand = 0;
    memcpy(&uCommandType, pbCommand, sizeof(uCommandType));
    memcpy(&fCommand, pbCommand + 4, sizeof(fCommand));
    if (uCommandType != VIRTIOGPU_VK_CMD_COPY_IMAGE || fCommand)
        return false;
    memcpy(&pCopy->uCommandBuffer, pbCommand + 8, sizeof(pCopy->uCommandBuffer));
    memcpy(&pCopy->uSrcImage, pbCommand + 16, sizeof(pCopy->uSrcImage));
    memcpy(&pCopy->enmSrcImageLayout, pbCommand + 24, sizeof(pCopy->enmSrcImageLayout));
    memcpy(&pCopy->uDstImage, pbCommand + 28, sizeof(pCopy->uDstImage));
    memcpy(&pCopy->enmDstImageLayout, pbCommand + 36, sizeof(pCopy->enmDstImageLayout));
    memcpy(&pCopy->cRegions, pbCommand + 40, sizeof(pCopy->cRegions));
    memcpy(&pCopy->cbRegions, pbCommand + 44, sizeof(pCopy->cbRegions));
    if (!pCopy->uCommandBuffer || !pCopy->uSrcImage || !pCopy->uDstImage || pCopy->cRegions != 1
        || pCopy->cbRegions != 68)
        return false;
    pCopy->pbRegions = pbCommand + 52;
    return true;
}

#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
static int virtioGpuR3VulkanClearColorImage(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                             const VIRTIOGPUCLEARCOLORCMD *pClear)
{
    if (!pRes->fVulkanImage || !pThis->fVulkanQueue || !pThis->fVulkanSubmit || !pClear
        || pClear->enmImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || !pClear->pbRanges)
        return VERR_INVALID_PARAMETER;
    VkImageSubresourceRange Range;
    memcpy(&Range.aspectMask, pClear->pbRanges + 0, sizeof(Range.aspectMask));
    memcpy(&Range.baseMipLevel, pClear->pbRanges + 4, sizeof(Range.baseMipLevel));
    memcpy(&Range.levelCount, pClear->pbRanges + 8, sizeof(Range.levelCount));
    memcpy(&Range.baseArrayLayer, pClear->pbRanges + 12, sizeof(Range.baseArrayLayer));
    memcpy(&Range.layerCount, pClear->pbRanges + 16, sizeof(Range.layerCount));
    if (Range.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT || Range.baseMipLevel != 0
        || Range.levelCount != 1 || Range.baseArrayLayer != 0 || Range.layerCount != 1)
        return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_CLEAR_IMAGE_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_CLEAR_IMAGE_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_CLEAR_IMAGE_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_CLEAR_IMAGE_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_CLEAR_IMAGE_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdClearColorImage pfnCmdClearColorImage = VK_CLEAR_IMAGE_PROC(PFN_vkCmdClearColorImage, "vkCmdClearColorImage");
    PFN_vkResetFences pfnResetFences = VK_CLEAR_IMAGE_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_CLEAR_IMAGE_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_CLEAR_IMAGE_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdClearColorImage || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkAccessFlags fSrcAccess = pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                             ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags fSrcStage = pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                   ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkImageMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                     pRes->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED ? (VkAccessFlags)0 : fSrcAccess,
                                     VK_ACCESS_TRANSFER_WRITE_BIT, pRes->enmVkImageLayout,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED, pRes->hVkImage,
                                     { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkClearColorValue Color;
    memcpy(&Color, pClear->auColor, sizeof(Color));
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, fSrcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                          0, NULL, 0, NULL, 1, &Barrier);
    pfnCmdClearColorImage(hCmd, pRes->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          &Color, 1, &Range);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pRes->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    pRes->fVulkanImageDirty = true;
    return VINF_SUCCESS;
# undef VK_CLEAR_IMAGE_PROC
}

static int virtioGpuR3VulkanCopyBufferToImage(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                              PVIRTIOGPURESOURCE pDst,
                                              const VIRTIOGPUCOPYBUFFERTOIMAGECMD *pCopy)
{
    if (!pSrc->fVulkanBuffer || !pDst->fVulkanImage || !pThis->fVulkanQueue || !pThis->fVulkanSubmit
        || !pCopy || pCopy->enmDstImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        return VERR_INVALID_PARAMETER;
    uint64_t offBuffer = 0;
    uint32_t bufferRowLength = 0;
    uint32_t bufferImageHeight = 0;
    uint32_t aspectMask = 0;
    uint32_t mipLevel = 0;
    uint32_t baseArrayLayer = 0;
    uint32_t layerCount = 0;
    int32_t imageOffset[3] = { 0, 0, 0 };
    uint32_t imageExtent[3] = { 0, 0, 0 };
    memcpy(&offBuffer, pCopy->pbRegions + 0, sizeof(offBuffer));
    memcpy(&bufferRowLength, pCopy->pbRegions + 8, sizeof(bufferRowLength));
    memcpy(&bufferImageHeight, pCopy->pbRegions + 12, sizeof(bufferImageHeight));
    memcpy(&aspectMask, pCopy->pbRegions + 16, sizeof(aspectMask));
    memcpy(&mipLevel, pCopy->pbRegions + 20, sizeof(mipLevel));
    memcpy(&baseArrayLayer, pCopy->pbRegions + 24, sizeof(baseArrayLayer));
    memcpy(&layerCount, pCopy->pbRegions + 28, sizeof(layerCount));
    memcpy(imageOffset, pCopy->pbRegions + 32, sizeof(imageOffset));
    memcpy(imageExtent, pCopy->pbRegions + 44, sizeof(imageExtent));
    if (offBuffer || bufferRowLength || bufferImageHeight || aspectMask != VK_IMAGE_ASPECT_COLOR_BIT
        || mipLevel || baseArrayLayer || layerCount != 1 || imageOffset[0] || imageOffset[1] || imageOffset[2]
        || imageExtent[0] != pDst->uWidth || imageExtent[1] != pDst->uHeight || imageExtent[2] != 1
        || pDst->cbPixels > pSrc->cbPixels)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pSrc);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_COPY_IMAGE_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_COPY_IMAGE_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_COPY_IMAGE_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_COPY_IMAGE_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_COPY_IMAGE_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdCopyBufferToImage pfnCmdCopyBufferToImage = VK_COPY_IMAGE_PROC(PFN_vkCmdCopyBufferToImage, "vkCmdCopyBufferToImage");
    PFN_vkResetFences pfnResetFences = VK_COPY_IMAGE_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_COPY_IMAGE_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_COPY_IMAGE_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdCopyBufferToImage || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkAccessFlags fSrcAccess = pDst->enmVkImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                             ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags fSrcStage = pDst->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                   ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkImageMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                     pDst->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED ? (VkAccessFlags)0 : fSrcAccess,
                                     VK_ACCESS_TRANSFER_WRITE_BIT, pDst->enmVkImageLayout,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED, pDst->hVkImage,
                                     { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkBufferImageCopy Region = { offBuffer, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                                 { 0, 0, 0 }, { pDst->uWidth, pDst->uHeight, 1 } };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, fSrcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                          0, NULL, 0, NULL, 1, &Barrier);
    pfnCmdCopyBufferToImage(hCmd, pSrc->hVkBuffer, pDst->hVkImage,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    memcpy(pDst->pvVkMapped, pSrc->pvVkMapped, (size_t)pDst->cbPixels);
    memcpy(pDst->pbPixels, pDst->pvVkMapped, (size_t)pDst->cbPixels);
    pDst->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    pDst->fVulkanImageDirty = false;
    return VINF_SUCCESS;
# undef VK_COPY_IMAGE_PROC
}

static int virtioGpuR3VulkanCopyImageToBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                              PVIRTIOGPURESOURCE pDst,
                                              const VIRTIOGPUCOPYIMAGETOBUFFERCMD *pCopy)
{
    if (!pSrc->fVulkanImage || !pDst->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit
        || !pCopy || pCopy->enmSrcImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        return VERR_INVALID_PARAMETER;
    uint64_t offBuffer = 0;
    uint32_t bufferRowLength = 0;
    uint32_t bufferImageHeight = 0;
    uint32_t aspectMask = 0;
    uint32_t mipLevel = 0;
    uint32_t baseArrayLayer = 0;
    uint32_t layerCount = 0;
    int32_t imageOffset[3] = { 0, 0, 0 };
    uint32_t imageExtent[3] = { 0, 0, 0 };
    memcpy(&offBuffer, pCopy->pbRegions + 0, sizeof(offBuffer));
    memcpy(&bufferRowLength, pCopy->pbRegions + 8, sizeof(bufferRowLength));
    memcpy(&bufferImageHeight, pCopy->pbRegions + 12, sizeof(bufferImageHeight));
    memcpy(&aspectMask, pCopy->pbRegions + 16, sizeof(aspectMask));
    memcpy(&mipLevel, pCopy->pbRegions + 20, sizeof(mipLevel));
    memcpy(&baseArrayLayer, pCopy->pbRegions + 24, sizeof(baseArrayLayer));
    memcpy(&layerCount, pCopy->pbRegions + 28, sizeof(layerCount));
    memcpy(imageOffset, pCopy->pbRegions + 32, sizeof(imageOffset));
    memcpy(imageExtent, pCopy->pbRegions + 44, sizeof(imageExtent));
    if (offBuffer || bufferRowLength || bufferImageHeight || aspectMask != VK_IMAGE_ASPECT_COLOR_BIT
        || mipLevel || baseArrayLayer || layerCount != 1 || imageOffset[0] || imageOffset[1] || imageOffset[2]
        || imageExtent[0] != pSrc->uWidth || imageExtent[1] != pSrc->uHeight || imageExtent[2] != 1
        || pSrc->cbPixels > pDst->cbPixels)
        return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_COPY_IMAGE_TO_BUFFER_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdCopyImageToBuffer pfnCmdCopyImageToBuffer = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkCmdCopyImageToBuffer, "vkCmdCopyImageToBuffer");
    PFN_vkResetFences pfnResetFences = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_COPY_IMAGE_TO_BUFFER_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdCopyImageToBuffer || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkAccessFlags fSrcAccess = pSrc->enmVkImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                             ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags fSrcStage = pSrc->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED
                                   ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkImageMemoryBarrier Barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                     pSrc->enmVkImageLayout == VK_IMAGE_LAYOUT_UNDEFINED ? (VkAccessFlags)0 : fSrcAccess,
                                     VK_ACCESS_TRANSFER_READ_BIT, pSrc->enmVkImageLayout,
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
                                     VK_QUEUE_FAMILY_IGNORED, pSrc->hVkImage,
                                     { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkBufferImageCopy Region = { offBuffer, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                                 { 0, 0, 0 }, { pSrc->uWidth, pSrc->uHeight, 1 } };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, fSrcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                          0, NULL, 0, NULL, 1, &Barrier);
    pfnCmdCopyImageToBuffer(hCmd, pSrc->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            pDst->hVkBuffer, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    memcpy(pDst->pbPixels, pDst->pvVkMapped, (size_t)pSrc->cbPixels);
    if (pSrc == pDst)
        pSrc->fVulkanImageDirty = false;
    pSrc->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    return VINF_SUCCESS;
# undef VK_COPY_IMAGE_TO_BUFFER_PROC
}

static int virtioGpuR3VulkanPipelineBarrier(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                            const VIRTIOGPUPIPELINEBARRIERCMD *pBarrier)
{
    if (!pRes->fVulkanImage || !pThis->fVulkanQueue || !pThis->fVulkanSubmit || !pBarrier
        || !pBarrier->pbImageBarrier)
        return VERR_INVALID_PARAMETER;
    uint32_t sType = 0;
    uint64_t cbNext = 0;
    uint32_t fSrcAccess = 0;
    uint32_t fDstAccess = 0;
    uint32_t enmOldLayout = 0;
    uint32_t enmNewLayout = 0;
    uint32_t uSrcQueue = 0;
    uint32_t uDstQueue = 0;
    uint64_t uImage = 0;
    uint32_t auRange[5];
    memcpy(&sType, pBarrier->pbImageBarrier + 0, sizeof(sType));
    memcpy(&cbNext, pBarrier->pbImageBarrier + 4, sizeof(cbNext));
    memcpy(&fSrcAccess, pBarrier->pbImageBarrier + 12, sizeof(fSrcAccess));
    memcpy(&fDstAccess, pBarrier->pbImageBarrier + 16, sizeof(fDstAccess));
    memcpy(&enmOldLayout, pBarrier->pbImageBarrier + 20, sizeof(enmOldLayout));
    memcpy(&enmNewLayout, pBarrier->pbImageBarrier + 24, sizeof(enmNewLayout));
    memcpy(&uSrcQueue, pBarrier->pbImageBarrier + 28, sizeof(uSrcQueue));
    memcpy(&uDstQueue, pBarrier->pbImageBarrier + 32, sizeof(uDstQueue));
    memcpy(&uImage, pBarrier->pbImageBarrier + 36, sizeof(uImage));
    memcpy(auRange, pBarrier->pbImageBarrier + 44, sizeof(auRange));
    if (sType != VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER || cbNext != 0 || uImage == 0
        || uImage > UINT32_MAX || (uint32_t)uImage != pRes->uResourceId
        || uSrcQueue != VK_QUEUE_FAMILY_IGNORED || uDstQueue != VK_QUEUE_FAMILY_IGNORED
        || auRange[0] != VK_IMAGE_ASPECT_COLOR_BIT || auRange[1] || auRange[2] != 1
        || auRange[3] || auRange[4] != 1)
        return VERR_INVALID_PARAMETER;
    if (pRes->enmVkImageLayout != VK_IMAGE_LAYOUT_UNDEFINED
        && enmOldLayout != (uint32_t)pRes->enmVkImageLayout)
        return VERR_INVALID_PARAMETER;
    if (enmNewLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && enmNewLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        return VERR_NOT_SUPPORTED;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_BARRIER_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_BARRIER_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_BARRIER_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_BARRIER_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_BARRIER_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkResetFences pfnResetFences = VK_BARRIER_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_BARRIER_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_BARRIER_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkImageMemoryBarrier HostBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
                                         fSrcAccess, fDstAccess, (VkImageLayout)enmOldLayout,
                                         (VkImageLayout)enmNewLayout, VK_QUEUE_FAMILY_IGNORED,
                                         VK_QUEUE_FAMILY_IGNORED, pRes->hVkImage,
                                         { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
    VkPipelineStageFlags fSrcStage = pBarrier->fSrcStage ? pBarrier->fSrcStage : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags fDstStage = pBarrier->fDstStage ? pBarrier->fDstStage : VK_PIPELINE_STAGE_TRANSFER_BIT;
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, fSrcStage, fDstStage, pBarrier->fDependency,
                          0, NULL, 0, NULL, 1, &HostBarrier);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pRes->enmVkImageLayout = (VkImageLayout)enmNewLayout;
    return VINF_SUCCESS;
# undef VK_BARRIER_PROC
}

static int virtioGpuR3VulkanCopyImage(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                      PVIRTIOGPURESOURCE pDst, const VIRTIOGPUCOPYIMAGECMD *pCopy)
{
    if (!pSrc->fVulkanImage || !pDst->fVulkanImage || !pThis->fVulkanQueue || !pThis->fVulkanSubmit
        || !pCopy || pCopy->enmSrcImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        || pCopy->enmDstImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        return VERR_INVALID_PARAMETER;
    uint32_t auSrcSubresource[4];
    int32_t aiSrcOffset[3];
    uint32_t auDstSubresource[4];
    int32_t aiDstOffset[3];
    uint32_t auExtent[3];
    memcpy(auSrcSubresource, pCopy->pbRegions + 0, sizeof(auSrcSubresource));
    memcpy(aiSrcOffset, pCopy->pbRegions + 16, sizeof(aiSrcOffset));
    memcpy(auDstSubresource, pCopy->pbRegions + 28, sizeof(auDstSubresource));
    memcpy(aiDstOffset, pCopy->pbRegions + 44, sizeof(aiDstOffset));
    memcpy(auExtent, pCopy->pbRegions + 56, sizeof(auExtent));
    if (auSrcSubresource[0] != VK_IMAGE_ASPECT_COLOR_BIT || auSrcSubresource[1]
        || auSrcSubresource[2] || auSrcSubresource[3] != 1 || auDstSubresource[0] != VK_IMAGE_ASPECT_COLOR_BIT
        || auDstSubresource[1] || auDstSubresource[2] || auDstSubresource[3] != 1
        || aiSrcOffset[0] || aiSrcOffset[1] || aiSrcOffset[2] || aiDstOffset[0] || aiDstOffset[1]
        || aiDstOffset[2] || auExtent[0] != pSrc->uWidth || auExtent[1] != pSrc->uHeight
        || auExtent[2] != 1 || pSrc->uWidth != pDst->uWidth || pSrc->uHeight != pDst->uHeight)
        return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_COPY_IMAGE_CMD_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_COPY_IMAGE_CMD_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_COPY_IMAGE_CMD_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_COPY_IMAGE_CMD_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdPipelineBarrier pfnCmdPipelineBarrier = VK_COPY_IMAGE_CMD_PROC(PFN_vkCmdPipelineBarrier, "vkCmdPipelineBarrier");
    PFN_vkCmdCopyImage pfnCmdCopyImage = VK_COPY_IMAGE_CMD_PROC(PFN_vkCmdCopyImage, "vkCmdCopyImage");
    PFN_vkResetFences pfnResetFences = VK_COPY_IMAGE_CMD_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_COPY_IMAGE_CMD_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_COPY_IMAGE_CMD_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdPipelineBarrier
        || !pfnCmdCopyImage || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    VkImageMemoryBarrier aBarriers[2] = {
        { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
          pSrc->enmVkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
          VK_QUEUE_FAMILY_IGNORED, pSrc->hVkImage, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } },
        { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
          pDst->enmVkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
          VK_QUEUE_FAMILY_IGNORED, pDst->hVkImage, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } }
    };
    VkImageCopy Region = { { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, { 0, 0, 0 },
                           { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, { 0, 0, 0 },
                           { pSrc->uWidth, pSrc->uHeight, 1 } };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdPipelineBarrier(hCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                          0, NULL, 0, NULL, 2, aBarriers);
    pfnCmdCopyImage(hCmd, pSrc->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    pDst->hVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pDst->enmVkImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    pDst->fVulkanImageDirty = true;
    return VINF_SUCCESS;
# undef VK_COPY_IMAGE_CMD_PROC
}
#endif

static int virtioGpuR3VulkanUpdateBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                          uint64_t off, const uint8_t *pbData, uint64_t cbData)
{
    if (!pRes->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit || !pbData || !cbData
        || off > pRes->cbPixels || cbData > pRes->cbPixels - off)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pRes);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_UPDATE_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_UPDATE_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_UPDATE_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_UPDATE_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdUpdateBuffer pfnCmdUpdateBuffer = VK_UPDATE_PROC(PFN_vkCmdUpdateBuffer, "vkCmdUpdateBuffer");
    PFN_vkResetFences pfnResetFences = VK_UPDATE_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_UPDATE_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_UPDATE_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdUpdateBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdUpdateBuffer(hCmd, pRes->hVkBuffer, off, cbData, pbData);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    if (memcmp((uint8_t *)pRes->pvVkMapped + off, pbData, (size_t)cbData) != 0)
        return VERR_MISMATCH;
    memcpy(pRes->pbPixels + off, pbData, (size_t)cbData);
    return virtioGpuR3VulkanResourceSyncImage(pThis, pRes);
# undef VK_UPDATE_PROC
}

static int virtioGpuR3VulkanCopyBufferBatch(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                             PVIRTIOGPURESOURCE pDst, const VIRTIOGPUCOPYCMD *paCopy,
                                             uint32_t cCopy)
{
    if (!cCopy || !paCopy || !pSrc->fVulkanBuffer || !pDst->fVulkanBuffer || !pThis->fVulkanQueue
        || !pThis->fVulkanSubmit)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pSrc);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pDst);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    for (uint32_t i = 0; i < cCopy; ++i)
        if (paCopy[i].offSrc > pSrc->cbPixels || paCopy[i].cbCopy > pSrc->cbPixels - paCopy[i].offSrc
            || paCopy[i].offDst > pDst->cbPixels || paCopy[i].cbCopy > pDst->cbPixels - paCopy[i].offDst
            || !paCopy[i].cbCopy)
            return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_BATCH_COPY_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_BATCH_COPY_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_BATCH_COPY_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_BATCH_COPY_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdCopyBuffer pfnCmdCopyBuffer = VK_BATCH_COPY_PROC(PFN_vkCmdCopyBuffer, "vkCmdCopyBuffer");
    PFN_vkResetFences pfnResetFences = VK_BATCH_COPY_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_BATCH_COPY_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_BATCH_COPY_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdCopyBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    for (uint32_t i = 0; i < cCopy; ++i)
    {
        VkBufferCopy Region = { paCopy[i].offSrc, paCopy[i].offDst, paCopy[i].cbCopy };
        pfnCmdCopyBuffer(hCmd, pSrc->hVkBuffer, pDst->hVkBuffer, 1, &Region);
    }
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    for (uint32_t i = 0; i < cCopy; ++i)
    {
        memcpy((uint8_t *)pDst->pvVkMapped + paCopy[i].offDst,
               (uint8_t *)pSrc->pvVkMapped + paCopy[i].offSrc, (size_t)paCopy[i].cbCopy);
        memcpy(pDst->pbPixels + paCopy[i].offDst,
               (uint8_t *)pDst->pvVkMapped + paCopy[i].offDst, (size_t)paCopy[i].cbCopy);
    }
    return virtioGpuR3VulkanResourceSyncImage(pThis, pDst);
# undef VK_BATCH_COPY_PROC
}

static int virtioGpuR3VulkanCopyBufferRegions(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pSrc,
                                               PVIRTIOGPURESOURCE pDst, const VIRTIOGPUCOPYCMD *pCopy)
{
    if (!pCopy || !pCopy->pbRegions || !pSrc->fVulkanBuffer || !pDst->fVulkanBuffer
        || !pThis->fVulkanQueue || !pThis->fVulkanSubmit)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pSrc);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pDst);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    VkBufferCopy aRegions[256];
    for (uint32_t i = 0; i < pCopy->cRegions; ++i)
    {
        memcpy(&aRegions[i].srcOffset, pCopy->pbRegions + i * 24 + 0, sizeof(aRegions[i].srcOffset));
        memcpy(&aRegions[i].dstOffset, pCopy->pbRegions + i * 24 + 8, sizeof(aRegions[i].dstOffset));
        memcpy(&aRegions[i].size, pCopy->pbRegions + i * 24 + 16, sizeof(aRegions[i].size));
        if (aRegions[i].srcOffset > pSrc->cbPixels || aRegions[i].size > pSrc->cbPixels - aRegions[i].srcOffset
            || aRegions[i].dstOffset > pDst->cbPixels || aRegions[i].size > pDst->cbPixels - aRegions[i].dstOffset
            || !aRegions[i].size)
            return VERR_INVALID_PARAMETER;
    }
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_COPY_REGIONS_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_COPY_REGIONS_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_COPY_REGIONS_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_COPY_REGIONS_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdCopyBuffer pfnCmdCopyBuffer = VK_COPY_REGIONS_PROC(PFN_vkCmdCopyBuffer, "vkCmdCopyBuffer");
    PFN_vkResetFences pfnResetFences = VK_COPY_REGIONS_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_COPY_REGIONS_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_COPY_REGIONS_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdCopyBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnCmdCopyBuffer(hCmd, pSrc->hVkBuffer, pDst->hVkBuffer, pCopy->cRegions, aRegions);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    for (uint32_t i = 0; i < pCopy->cRegions; ++i)
    {
        memcpy((uint8_t *)pDst->pvVkMapped + aRegions[i].dstOffset,
               (uint8_t *)pSrc->pvVkMapped + aRegions[i].srcOffset, (size_t)aRegions[i].size);
        memcpy(pDst->pbPixels + aRegions[i].dstOffset,
               (uint8_t *)pDst->pvVkMapped + aRegions[i].dstOffset, (size_t)aRegions[i].size);
    }
    return virtioGpuR3VulkanResourceSyncImage(pThis, pDst);
# undef VK_COPY_REGIONS_PROC
}

static int virtioGpuR3VulkanFillBufferBatch(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                             const VIRTIOGPUFILLCMD *paFill, uint32_t cFill)
{
    if (!cFill || !paFill || !pRes->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit)
        return VERR_INVALID_PARAMETER;
    int rcEnsure = virtioGpuR3VulkanResourceEnsureBuffer(pThis, pRes);
    if (RT_FAILURE(rcEnsure))
        return rcEnsure;
    for (uint32_t i = 0; i < cFill; ++i)
        if (paFill[i].offBuffer > pRes->cbPixels || paFill[i].cbBuffer > pRes->cbPixels - paFill[i].offBuffer
            || !paFill[i].cbBuffer || (paFill[i].offBuffer & 3) || (paFill[i].cbBuffer & 3))
            return VERR_INVALID_PARAMETER;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr =
        (PFN_vkGetDeviceProcAddr)pThis->pfnVkGetInstanceProcAddr(pThis->hVkInstance, "vkGetDeviceProcAddr");
    if (!pfnGetDeviceProcAddr)
        return VERR_NOT_FOUND;
# define VK_BATCH_PROC(type, name) (type)pfnGetDeviceProcAddr(pThis->hVkDevice, name)
    PFN_vkResetCommandBuffer pfnResetCommandBuffer = VK_BATCH_PROC(PFN_vkResetCommandBuffer, "vkResetCommandBuffer");
    PFN_vkBeginCommandBuffer pfnBeginCommandBuffer = VK_BATCH_PROC(PFN_vkBeginCommandBuffer, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pfnEndCommandBuffer = VK_BATCH_PROC(PFN_vkEndCommandBuffer, "vkEndCommandBuffer");
    PFN_vkCmdFillBuffer pfnCmdFillBuffer = VK_BATCH_PROC(PFN_vkCmdFillBuffer, "vkCmdFillBuffer");
    PFN_vkResetFences pfnResetFences = VK_BATCH_PROC(PFN_vkResetFences, "vkResetFences");
    PFN_vkQueueSubmit pfnQueueSubmit = VK_BATCH_PROC(PFN_vkQueueSubmit, "vkQueueSubmit");
    PFN_vkWaitForFences pfnWaitForFences = VK_BATCH_PROC(PFN_vkWaitForFences, "vkWaitForFences");
    if (!pfnResetCommandBuffer || !pfnBeginCommandBuffer || !pfnEndCommandBuffer || !pfnCmdFillBuffer
        || !pfnResetFences || !pfnQueueSubmit || !pfnWaitForFences)
        return VERR_NOT_FOUND;
    VkCommandBuffer hCmd = pThis->hVkSubmitCommandBuffer;
    VkFence hFence = pThis->hVkSubmitFence;
    VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL };
    VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, NULL, NULL, 1, &hCmd, 0, NULL };
    if (pfnResetFences(pThis->hVkDevice, 1, &hFence) != VK_SUCCESS
        || pfnResetCommandBuffer(hCmd, 0) != VK_SUCCESS
        || pfnBeginCommandBuffer(hCmd, &BeginInfo) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    for (uint32_t i = 0; i < cFill; ++i)
        pfnCmdFillBuffer(hCmd, pRes->hVkBuffer, paFill[i].offBuffer, paFill[i].cbBuffer, paFill[i].uData);
    if (pfnEndCommandBuffer(hCmd) != VK_SUCCESS
        || pfnQueueSubmit(pThis->hVkQueue, 1, &SubmitInfo, hFence) != VK_SUCCESS
        || pfnWaitForFences(pThis->hVkDevice, 1, &hFence, VK_TRUE, UINT64_C(1000000000)) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    for (uint32_t i = 0; i < cFill; ++i)
    {
        if (*(uint32_t *)((uint8_t *)pRes->pvVkMapped + paFill[i].offBuffer) != paFill[i].uData)
            return VERR_MISMATCH;
        memcpy(pRes->pbPixels + paFill[i].offBuffer,
               (uint8_t *)pRes->pvVkMapped + paFill[i].offBuffer, (size_t)paFill[i].cbBuffer);
    }
    return virtioGpuR3VulkanResourceSyncImage(pThis, pRes);
# undef VK_BATCH_PROC
}
#endif

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
    bool fPreserveResponse = false;
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
            case VIRTIOGPU_CMD_GET_CAPSET_INFO:
            {
                VIRTIOGPUCAPSETINFO Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd)))
                    || Cmd.uCapsetIndex >= pThis->Config.cCapsets
                    || pBuf->cbPhysReturn < sizeof(VIRTIOGPUCAPSETINFORESP))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    VIRTIOGPUCAPSETINFORESP CapResp;
                    RT_ZERO(CapResp);
                    CapResp.Hdr = Resp.Hdr;
                    CapResp.Hdr.uType = VIRTIOGPU_RESP_OK_CAPSET_INFO;
                    memcpy(&Resp, &CapResp, sizeof(CapResp));
                    cbResp = sizeof(CapResp);
                    fPreserveResponse = true;
                }
                break;
            }
            case VIRTIOGPU_CMD_GET_CAPSET:
            {
                VIRTIOGPUGETCAPSET Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd))))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                break;
            }
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
                        int rcBackend = VINF_SUCCESS;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                        rcBackend = virtioGpuR3VulkanResourceCreate(pThis, pRes);
#endif
                        if (RT_FAILURE(rcBackend))
                        {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                            virtioGpuR3VulkanResourceDestroy(pThis, pRes);
#endif
                            RTMemFree(pRes->pbPixels);
                            pThis->cbAllocated -= pRes->cbPixels;
                            RT_ZERO(*pRes);
                            Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY;
                        }
                        else
                            Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_CREATE_BLOB:
            {
                VIRTIOGPURESOURCECREATEBLOB Cmd;
                RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd) || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd)))
                    || !Cmd.uResourceId || virtioGpuR3FindResource(pThis, Cmd.uResourceId)
                    || !Cmd.cbBlob || Cmd.cbBlob > VIRTIOGPU_MAX_RESOURCE_BYTES || (Cmd.cbBlob & 3)
                    || Cmd.cEntries > VIRTIOGPU_MAX_BACKING_ENTRIES
                    || pBuf->cbPhysSend < sizeof(Cmd) + (size_t)Cmd.cEntries * sizeof(VIRTIOGPUMEMENTRY))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = NULL;
                    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aResources); ++i)
                        if (!pThis->aResources[i].fUsed) { pRes = &pThis->aResources[i]; break; }
                    if (!pRes || pThis->cbAllocated > VIRTIOGPU_MAX_RESOURCE_BYTES - Cmd.cbBlob
                        || Cmd.cbBlob / 4 > UINT32_MAX)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY;
                    else
                    {
                        pRes->pbPixels = (uint8_t *)RTMemAllocZ((size_t)Cmd.cbBlob);
                        if (!pRes->pbPixels)
                            Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY;
                        else
                        {
                            pRes->fUsed = true;
                            pRes->fBlob = true;
                            pRes->uResourceId = Cmd.uResourceId;
                            pRes->uWidth = (uint32_t)(Cmd.cbBlob / 4);
                            pRes->uHeight = 1;
                            pRes->cbPixels = (size_t)Cmd.cbBlob;
                            pRes->cBacking = Cmd.cEntries;
                            if (Cmd.cEntries)
                            {
                                rcReq = virtioGpuR3Read(pDevIns, pVirtio, pBuf, pRes->aBacking,
                                                        (size_t)Cmd.cEntries * sizeof(VIRTIOGPUMEMENTRY));
                                uint64_t cbBacking = 0;
                                for (uint32_t i = 0; RT_SUCCESS(rcReq) && i < Cmd.cEntries; ++i)
                                    if (pRes->aBacking[i].GCPhys > UINT64_MAX - pRes->aBacking[i].cb
                                        || cbBacking > UINT64_MAX - pRes->aBacking[i].cb)
                                        rcReq = VERR_OUT_OF_RANGE;
                                    else
                                        cbBacking += pRes->aBacking[i].cb;
                                if (RT_FAILURE(rcReq) || cbBacking < Cmd.cbBlob)
                                    rcReq = VERR_INVALID_PARAMETER;
                            }
                            if (RT_SUCCESS(rcReq))
                            {
                                pThis->cbAllocated += pRes->cbPixels;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                rcReq = virtioGpuR3VulkanResourceCreate(pThis, pRes);
#endif
                            }
                            if (RT_FAILURE(rcReq))
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                virtioGpuR3VulkanResourceDestroy(pThis, pRes);
#endif
                                RTMemFree(pRes->pbPixels);
                                RT_ZERO(*pRes);
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                            }
                            else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                        }
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
                        RTMemFree(pRes->pbPixels);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                        virtioGpuR3VulkanResourceDestroy(pThis, pRes);
#endif
                        for (unsigned i = 0; i < RT_ELEMENTS(pThis->aContexts); ++i)
                        {
                            PVIRTIOGPUCONTEXT pCtx = &pThis->aContexts[i];
                            for (uint32_t j = 0; j < pCtx->cResources; )
                                if (pCtx->auResourceIds[j] == Cmd.id)
                                {
                                    pCtx->auResourceIds[j] = pCtx->auResourceIds[--pCtx->cResources];
                                    pCtx->auResourceIds[pCtx->cResources] = 0;
                                }
                                else
                                    ++j;
                        }
                        pThis->cbAllocated -= pRes->cbPixels; RT_ZERO(*pRes);
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_CTX_CREATE:
            {
                struct { uint32_t cchName; uint32_t fInit; char szName[64]; } Cmd;
                RT_ZERO(Cmd);
                if (!Req.uCtxId || virtioGpuR3FindContext(pThis, Req.uCtxId)
                    || pBuf->cbPhysSend < sizeof(Cmd)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd)))
                    || Cmd.cchName > sizeof(Cmd.szName) || Cmd.fInit != 0)
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPUCONTEXT pCtx = NULL;
                    for (unsigned i = 0; i < RT_ELEMENTS(pThis->aContexts); ++i)
                        if (!pThis->aContexts[i].fUsed) { pCtx = &pThis->aContexts[i]; break; }
                    if (!pCtx)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_OUT_OF_MEMORY;
                    else
                    {
                        pCtx->fUsed = true;
                        pCtx->uContextId = Req.uCtxId;
                        pCtx->cchName = Cmd.cchName;
                        memcpy(pCtx->szName, Cmd.szName, sizeof(pCtx->szName));
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_CTX_DESTROY:
            {
                if (!Req.uCtxId)
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPUCONTEXT pCtx = virtioGpuR3FindContext(pThis, Req.uCtxId);
                    if (!pCtx)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        RT_ZERO(*pCtx);
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE:
            case VIRTIOGPU_CMD_CTX_DETACH_RESOURCE:
            {
                uint32_t uResourceId = 0;
                PVIRTIOGPUCONTEXT pCtx = virtioGpuR3FindContext(pThis, Req.uCtxId);
                if (!pCtx || pBuf->cbPhysSend < sizeof(uResourceId)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &uResourceId, sizeof(uResourceId))))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else if (!virtioGpuR3FindResource(pThis, uResourceId))
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_RESOURCE_ID;
                else if (Req.uType == VIRTIOGPU_CMD_CTX_ATTACH_RESOURCE)
                {
                    if (virtioGpuR3ContextHasResource(pCtx, uResourceId)
                        || pCtx->cResources >= VIRTIOGPU_MAX_CONTEXT_RESOURCES)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        pCtx->auResourceIds[pCtx->cResources++] = uResourceId;
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                else
                {
                    uint32_t iResource = 0;
                    while (iResource < pCtx->cResources && pCtx->auResourceIds[iResource] != uResourceId)
                        ++iResource;
                    if (iResource == pCtx->cResources)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        pCtx->auResourceIds[iResource] = pCtx->auResourceIds[--pCtx->cResources];
                        pCtx->auResourceIds[pCtx->cResources] = 0;
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_SUBMIT_3D:
            {
                VIRTIOGPUSUBMIT3D Cmd;
                RT_ZERO(Cmd);
                PVIRTIOGPUCONTEXT pCtx = virtioGpuR3FindContext(pThis, Req.uCtxId);
                uint8_t *pbCommand = NULL;
                if (!pCtx || pBuf->cbPhysSend < sizeof(Cmd)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf, &Cmd, sizeof(Cmd)))
                    || Cmd.cbCommand > VIRTIOGPU_MAX_SUBMIT_BYTES || Cmd.cResources > VIRTIOGPU_MAX_CONTEXT_RESOURCES
                    || pBuf->cbPhysSend < sizeof(Cmd) + (size_t)Cmd.cResources * sizeof(uint32_t) + Cmd.cbCommand)
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    uint32_t auResourceIds[VIRTIOGPU_MAX_CONTEXT_RESOURCES];
                    RT_ZERO(auResourceIds);
                    rcReq = Cmd.cResources
                          ? virtioGpuR3Read(pDevIns, pVirtio, pBuf, auResourceIds,
                                            (size_t)Cmd.cResources * sizeof(uint32_t)) : VINF_SUCCESS;
                    for (uint32_t i = 0; RT_SUCCESS(rcReq) && i < Cmd.cResources; ++i)
                        if (!virtioGpuR3ContextHasResource(pCtx, auResourceIds[i]))
                            rcReq = VERR_INVALID_PARAMETER;
                    if (RT_SUCCESS(rcReq) && Cmd.cbCommand)
                    {
                        pbCommand = (uint8_t *)RTMemAlloc(Cmd.cbCommand);
                        if (!pbCommand)
                            rcReq = VERR_NO_MEMORY;
                        else
                        {
                            rcReq = virtioGpuR3Read(pDevIns, pVirtio, pBuf, pbCommand, Cmd.cbCommand);
                        }
                    }
                    if (RT_FAILURE(rcReq))
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else if (Cmd.cbCommand == 0)
                        Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                    else
                    {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                        VIRTIOGPUUPDATECMD Update;
                        VIRTIOGPUCLEARCOLORCMD Clear;
                        VIRTIOGPUCOPYBUFFERTOIMAGECMD CopyBufferToImage;
                        VIRTIOGPUCOPYIMAGETOBUFFERCMD CopyImageToBuffer;
                        VIRTIOGPUPIPELINEBARRIERCMD PipelineBarrier;
                        VIRTIOGPUCOPYIMAGECMD CopyImage;
                        PVIRTIOGPURESOURCE pUpdateRes = NULL;
                        if (virtioGpuR3DecodeUpdateBuffer(pbCommand, Cmd.cbCommand, &Update))
                        {
                            if (Update.uBuffer > UINT32_MAX
                                || !(pUpdateRes = virtioGpuR3FindResource(pThis, (uint32_t)Update.uBuffer))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)Update.uBuffer))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanUpdateBuffer(pThis, pUpdateRes,
                                                                                             Update.offBuffer,
                                                                                             Update.pbData,
                                                                                             Update.cbData))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (virtioGpuR3DecodeClearColorImage(pbCommand, Cmd.cbCommand, &Clear))
                        {
                            PVIRTIOGPURESOURCE pImageRes = NULL;
                            if (Clear.uImage > UINT32_MAX
                                || !(pImageRes = virtioGpuR3FindResource(pThis, (uint32_t)Clear.uImage))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)Clear.uImage))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanClearColorImage(pThis, pImageRes,
                                                                                               &Clear))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (virtioGpuR3DecodeCopyBufferToImage(pbCommand, Cmd.cbCommand, &CopyBufferToImage))
                        {
                            PVIRTIOGPURESOURCE pSrcRes = NULL;
                            PVIRTIOGPURESOURCE pDstRes = NULL;
                            if (CopyBufferToImage.uSrcBuffer > UINT32_MAX || CopyBufferToImage.uDstImage > UINT32_MAX
                                || !(pSrcRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyBufferToImage.uSrcBuffer))
                                || !(pDstRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyBufferToImage.uDstImage))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyBufferToImage.uSrcBuffer)
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyBufferToImage.uDstImage))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanCopyBufferToImage(pThis, pSrcRes,
                                                                                                  pDstRes,
                                                                                                  &CopyBufferToImage))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (virtioGpuR3DecodeCopyImageToBuffer(pbCommand, Cmd.cbCommand, &CopyImageToBuffer))
                        {
                            PVIRTIOGPURESOURCE pSrcRes = NULL;
                            PVIRTIOGPURESOURCE pDstRes = NULL;
                            if (CopyImageToBuffer.uSrcImage > UINT32_MAX || CopyImageToBuffer.uDstBuffer > UINT32_MAX
                                || !(pSrcRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyImageToBuffer.uSrcImage))
                                || !(pDstRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyImageToBuffer.uDstBuffer))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyImageToBuffer.uSrcImage)
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyImageToBuffer.uDstBuffer))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanCopyImageToBuffer(pThis, pSrcRes,
                                                                                                  pDstRes,
                                                                                                  &CopyImageToBuffer))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (virtioGpuR3DecodePipelineBarrier(pbCommand, Cmd.cbCommand, &PipelineBarrier))
                        {
                            PVIRTIOGPURESOURCE pImageRes = NULL;
                            uint64_t uImage = 0;
                            memcpy(&uImage, PipelineBarrier.pbImageBarrier + 36, sizeof(uImage));
                            if (uImage > UINT32_MAX
                                || !(pImageRes = virtioGpuR3FindResource(pThis, (uint32_t)uImage))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)uImage))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanPipelineBarrier(pThis, pImageRes,
                                                                                              &PipelineBarrier))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (virtioGpuR3DecodeCopyImage(pbCommand, Cmd.cbCommand, &CopyImage))
                        {
                            PVIRTIOGPURESOURCE pSrcRes = NULL;
                            PVIRTIOGPURESOURCE pDstRes = NULL;
                            if (CopyImage.uSrcImage > UINT32_MAX || CopyImage.uDstImage > UINT32_MAX
                                || !(pSrcRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyImage.uSrcImage))
                                || !(pDstRes = virtioGpuR3FindResource(pThis, (uint32_t)CopyImage.uDstImage))
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyImage.uSrcImage)
                                || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)CopyImage.uDstImage))
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                            else
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanCopyImage(pThis, pSrcRes, pDstRes,
                                                                                          &CopyImage))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
                        else if (Cmd.cbCommand > sizeof(uint32_t) * 11 && Cmd.cbCommand % 44 == 0
                            && Cmd.cbCommand / 44 <= 256)
                        {
                            uint32_t const cFill = Cmd.cbCommand / 44;
                            VIRTIOGPUFILLCMD aFill[256];
                            PVIRTIOGPURESOURCE pBatchRes = NULL;
                            bool fBatchValid = true;
                            for (uint32_t i = 0; i < cFill; ++i)
                            {
                                if (!virtioGpuR3DecodeFillBuffer(pbCommand + i * 44, 44, &aFill[i])
                                    || aFill[i].uBuffer > UINT32_MAX
                                    || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)aFill[i].uBuffer))
                                {
                                    fBatchValid = false;
                                    break;
                                }
                                PVIRTIOGPURESOURCE pCurRes = virtioGpuR3FindResource(pThis, (uint32_t)aFill[i].uBuffer);
                                if (!pCurRes || (pBatchRes && pBatchRes != pCurRes))
                                {
                                    fBatchValid = false;
                                    break;
                                }
                                pBatchRes = pCurRes;
                            }
                            if (fBatchValid)
                            {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanFillBufferBatch(pThis, pBatchRes,
                                                                                               aFill, cFill))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                                break;
                            }
                        }
                        else
                        {
                            VIRTIOGPUFILLCMD Fill;
                            PVIRTIOGPURESOURCE pRes = NULL;
                            if (virtioGpuR3DecodeFillBuffer(pbCommand, Cmd.cbCommand, &Fill))
                            {
                                if (Fill.uBuffer > UINT32_MAX
                                    || !(pRes = virtioGpuR3FindResource(pThis, (uint32_t)Fill.uBuffer))
                                    || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)Fill.uBuffer))
                                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                                else
                                {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                    Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanFillBuffer(pThis, pRes,
                                                                                             Fill.offBuffer, Fill.cbBuffer,
                                                                                             Fill.uData))
                                                   ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                                }
                            }
                            else
                            {
                                VIRTIOGPUCOPYCMD Copy;
                                PVIRTIOGPURESOURCE pSrc = NULL;
                                PVIRTIOGPURESOURCE pDst = NULL;
                                if (!virtioGpuR3DecodeCopyBuffer(pbCommand, Cmd.cbCommand, &Copy)
                                    || Copy.uSrcBuffer > UINT32_MAX || Copy.uDstBuffer > UINT32_MAX
                                    || !(pSrc = virtioGpuR3FindResource(pThis, (uint32_t)Copy.uSrcBuffer))
                                    || !(pDst = virtioGpuR3FindResource(pThis, (uint32_t)Copy.uDstBuffer))
                                    || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)Copy.uSrcBuffer)
                                    || !virtioGpuR3ContextHasResource(pCtx, (uint32_t)Copy.uDstBuffer))
                                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                                else
                                {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                                    Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanCopyBufferRegions(pThis, pSrc, pDst,
                                                                                                     &Copy))
                                                   ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                                }
                            }
                        }
#else
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                    }
                    RTMemFree(pbCommand);
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
                        if (RT_SUCCESS(rcReq))
                        {
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                            rcReq = virtioGpuR3VulkanResourceSync(pThis, pRes);
#endif
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
                else
                {
                    PVIRTIOGPURESOURCE pRes=virtioGpuR3FindResource(pThis,Cmd.uResourceId);
                    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
                    if(!pRes || !virtioGpuR3RectValid(pRes,Cmd.uX,Cmd.uY,Cmd.uWidth,Cmd.uHeight))
                        Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        int rcResize = VINF_SUCCESS;
                        if (pThisCC->pDrv && pThisCC->pDrv->pfnResize)
                            rcResize = pThisCC->pDrv->pfnResize(pThisCC->pDrv, 32, pRes->pbPixels,
                                                               pRes->uWidth * 4, Cmd.uWidth, Cmd.uHeight);
                        if (RT_FAILURE(rcResize))
                            Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                        else
                        {
                            pThis->aScanouts[Cmd.uScanoutId].uResourceId=Cmd.uResourceId;
                            pThis->aScanouts[Cmd.uScanoutId].uX=Cmd.uX;
                            pThis->aScanouts[Cmd.uScanoutId].uY=Cmd.uY;
                            pThis->aScanouts[Cmd.uScanoutId].uWidth=Cmd.uWidth;
                            pThis->aScanouts[Cmd.uScanoutId].uHeight=Cmd.uHeight;
                            Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA;
                        }
                    }
                }
                break;
            }
            case VIRTIOGPU_CMD_RESOURCE_FLUSH:
            {
                VIRTIOGPURESOURCEFLUSH Cmd; RT_ZERO(Cmd);
                if (pBuf->cbPhysSend < sizeof(Cmd) - sizeof(Cmd.Hdr)
                    || RT_FAILURE(virtioGpuR3Read(pDevIns, pVirtio, pBuf,
                                                   (uint8_t *)&Cmd + sizeof(Cmd.Hdr),
                                                   sizeof(Cmd) - sizeof(Cmd.Hdr))) )
                    Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else
                {
                    PVIRTIOGPURESOURCE pRes = virtioGpuR3FindResource(pThis, Cmd.uResourceId);
                    if (!pRes || !virtioGpuR3RectValid(pRes, Cmd.uX, Cmd.uY, Cmd.uWidth, Cmd.uHeight))
                        Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                    else
                    {
                        PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
                        bool fScanout = false;
                        for (unsigned i = 0; i < VIRTIOGPU_MAX_SCANOUTS; ++i)
                            if (pThis->aScanouts[i].uResourceId == Cmd.uResourceId)
                                fScanout = true;
                        int rcReadback = VINF_SUCCESS;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                        if (fScanout)
                            rcReadback = virtioGpuR3VulkanResourceReadbackImage(pThis, pRes);
#endif
                        if (RT_FAILURE(rcReadback))
                            Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
                        else
                        {
                            for (unsigned i = 0; i < VIRTIOGPU_MAX_SCANOUTS; ++i)
                                if (pThis->aScanouts[i].uResourceId == Cmd.uResourceId)
                                {
                                    pThis->aScanouts[i].uFlushSequence++;
                                    if (pThisCC->pDrv && pThisCC->pDrv->pfnUpdateRect)
                                        pThisCC->pDrv->pfnUpdateRect(pThisCC->pDrv, Cmd.uX, Cmd.uY,
                                                                      Cmd.uWidth, Cmd.uHeight);
                                }
                            Resp.Hdr.uType = VIRTIOGPU_RESP_OK_NODATA;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    if (pBuf->cbPhysReturn >= sizeof(Resp.Hdr) && !fPreserveResponse)
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

static DECLCALLBACK(void *) virtioGpuR3QueryInterface(PPDMIBASE pInterface, const char *pszIID)
{
    PVIRTIOGPUCC pThisCC = RT_FROM_MEMBER(pInterface, VIRTIOGPUCC, IBase);
    PDMIBASE_RETURN_INTERFACE(pszIID, PDMIBASE, &pThisCC->IBase);
    return NULL;
}

static DECLCALLBACK(int) virtioGpuR3Attach(PPDMDEVINS pDevIns, unsigned iLUN, uint32_t fFlags)
{
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    if (!(fFlags & PDM_TACH_FLAGS_NOT_HOT_PLUG) || iLUN != 0)
        return VERR_INVALID_PARAMETER;
    int rc = PDMDevHlpDriverAttach(pDevIns, iLUN, &pThisCC->IBase, &pThisCC->pDrvBase, "Display Port");
    if (RT_SUCCESS(rc))
    {
        pThisCC->pDrv = PDMIBASE_QUERY_INTERFACE(pThisCC->pDrvBase, PDMIDISPLAYCONNECTOR);
        if (!pThisCC->pDrv || !pThisCC->pDrv->pfnResize || !pThisCC->pDrv->pfnUpdateRect)
        {
            pThisCC->pDrv = NULL;
            pThisCC->pDrvBase = NULL;
            rc = VERR_PDM_MISSING_INTERFACE;
        }
    }
    return rc;
}

static DECLCALLBACK(int) virtioGpuR3SaveExec(PPDMDEVINS pDevIns, PSSMHANDLE pSSM)
{
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    int rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pThis->Config.fEventsRead);
    if (RT_SUCCESS(rc))
        rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, (uint32_t)pThis->enmBackend);
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aResources); ++i)
    {
        rc = pDevIns->pHlpR3->pfnSSMPutBool(pSSM, pThis->aResources[i].fUsed);
        if (RT_SUCCESS(rc) && pThis->aResources[i].fUsed)
        {
            PVIRTIOGPURESOURCE pRes = &pThis->aResources[i];
            rc = pDevIns->pHlpR3->pfnSSMPutBool(pSSM, pRes->fBlob);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uResourceId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uFormat);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uWidth);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->uHeight);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pRes->cBacking);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pRes->aBacking, sizeof(pRes->aBacking));
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pRes->pbPixels, pRes->cbPixels);
        }
    }
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aContexts); ++i)
    {
        rc = pDevIns->pHlpR3->pfnSSMPutBool(pSSM, pThis->aContexts[i].fUsed);
        if (RT_SUCCESS(rc) && pThis->aContexts[i].fUsed)
        {
            PVIRTIOGPUCONTEXT pCtx = &pThis->aContexts[i];
            rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pCtx->uContextId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pCtx->cchName);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pCtx->szName, sizeof(pCtx->szName));
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutU32(pSSM, pCtx->cResources);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMPutMem(pSSM, pCtx->auResourceIds, sizeof(pCtx->auResourceIds));
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
    uint32_t uSavedBackend = UINT32_MAX;
    if (RT_SUCCESS(rc))
        rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &uSavedBackend);
    if (RT_SUCCESS(rc) && (uSavedBackend > VIRTIOGPU_BACKEND_VENUS
                           || uSavedBackend != (uint32_t)pThis->enmBackend))
        rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
    virtioGpuR3FreeResources(pThis);
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aResources); ++i)
    {
        bool fUsed = false;
        rc = pDevIns->pHlpR3->pfnSSMGetBool(pSSM, &fUsed);
        if (RT_SUCCESS(rc) && fUsed)
        {
            PVIRTIOGPURESOURCE pRes = &pThis->aResources[i];
            pRes->fUsed = true;
            rc = pDevIns->pHlpR3->pfnSSMGetBool(pSSM, &pRes->fBlob);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uResourceId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uFormat);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uWidth);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->uHeight);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pRes->cBacking);
            if (RT_SUCCESS(rc) && (pRes->cBacking > VIRTIOGPU_MAX_BACKING_ENTRIES || !pRes->uResourceId
                                   || (!pRes->fBlob && pRes->uFormat != VIRTIOGPU_FORMAT_B8G8R8X8_UNORM)
                                   || (pRes->fBlob && (pRes->uFormat != 0 || pRes->uHeight != 1))
                                   || !pRes->uWidth
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
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
                    if (RT_SUCCESS(rc))
                        rc = virtioGpuR3VulkanResourceCreate(pThis, pRes);
#endif
                }
            }
            if (RT_FAILURE(rc))
            {
                RTMemFree(pRes->pbPixels);
                RT_ZERO(*pRes);
            }
        }
    }
    for (unsigned i = 0; RT_SUCCESS(rc) && i < RT_ELEMENTS(pThis->aContexts); ++i)
    {
        bool fUsed = false;
        rc = pDevIns->pHlpR3->pfnSSMGetBool(pSSM, &fUsed);
        if (RT_SUCCESS(rc) && fUsed)
        {
            PVIRTIOGPUCONTEXT pCtx = &pThis->aContexts[i];
            pCtx->fUsed = true;
            rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pCtx->uContextId);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pCtx->cchName);
            if (RT_SUCCESS(rc) && (pCtx->cchName > sizeof(pCtx->szName) || !pCtx->uContextId
                                   || virtioGpuR3FindContext(pThis, pCtx->uContextId) != pCtx))
                rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetMem(pSSM, pCtx->szName, sizeof(pCtx->szName));
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetU32(pSSM, &pCtx->cResources);
            if (RT_SUCCESS(rc)) rc = pDevIns->pHlpR3->pfnSSMGetMem(pSSM, pCtx->auResourceIds, sizeof(pCtx->auResourceIds));
            if (RT_SUCCESS(rc) && pCtx->cResources > VIRTIOGPU_MAX_CONTEXT_RESOURCES)
                rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
            for (uint32_t j = 0; RT_SUCCESS(rc) && j < pCtx->cResources; ++j)
                if (!virtioGpuR3FindResource(pThis, pCtx->auResourceIds[j]))
                    rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
                else
                    for (uint32_t k = 0; k < j; ++k)
                        if (pCtx->auResourceIds[k] == pCtx->auResourceIds[j])
                            rc = VERR_SSM_LOAD_CONFIG_MISMATCH;
            if (RT_FAILURE(rc))
                RT_ZERO(*pCtx);
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
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "", "");
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    char szBackend[16];
    int rc = pDevIns->pHlpR3->pfnCFGMQueryStringDef(pCfg, "Backend", szBackend, sizeof(szBackend), "auto");
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc, N_("virtio-gpu: failed to read Backend configuration"));
    rc = virtioGpuR3ParseBackend(szBackend, &pThis->enmBackend);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, VERR_INVALID_PARAMETER,
                                N_("virtio-gpu: Backend must be auto, software or venus"));
#ifndef VBOX_WITH_VIRTIO_GPU_VENUS
    if (pThis->enmBackend == VIRTIOGPU_BACKEND_VENUS)
        return PDMDEV_SET_ERROR(pDevIns, VERR_NOT_SUPPORTED,
                                N_("virtio-gpu: Venus backend is not included in this build"));
#endif
    pThis->Config.cScanouts = 1;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    if (pThis->enmBackend != VIRTIOGPU_BACKEND_SOFTWARE)
    {
        int rcVulkan = virtioGpuR3VulkanInit(pThis);
        if (RT_FAILURE(rcVulkan))
        {
            if (pThis->enmBackend == VIRTIOGPU_BACKEND_VENUS)
            {
                virtioGpuR3VulkanTerm(pThis);
                return PDMDEV_SET_ERROR(pDevIns, rcVulkan,
                                         N_("virtio-gpu: Venus backend requested but host Vulkan is unavailable"));
            }
            LogRel(("virtio-gpu: Vulkan host probe failed (%Rrc), using software path\n", rcVulkan));
            virtioGpuR3VulkanTerm(pThis);
        }
    }
#endif
    pThisCC->Virtio.pfnStatusChanged = virtioGpuR3StatusChanged;
    pThisCC->Virtio.pfnVirtqNotified = virtioGpuR3VirtqNotified;
    pThisCC->Virtio.pfnDevCapRead = virtioGpuR3DevCapRead;
    pThisCC->Virtio.pfnDevCapWrite = virtioGpuR3DevCapWrite;
    pThisCC->IBase.pfnQueryInterface = virtioGpuR3QueryInterface;
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
    rc = virtioCoreR3Init(pDevIns, &pThis->Virtio, &pThisCC->Virtio, &Pci, szName,
                            VIRTIOGPU_F_RESOURCE_BLOB | VIRTIOGPU_F_CONTEXT_INIT,
                            0, &pThis->Config, sizeof(pThis->Config), VIRTIOGPU_QUEUE_COUNT);
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
    virtioGpuR3FreeResources(pThis);
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    virtioGpuR3VulkanTerm(pThis);
#endif
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
    /* .pfnAttach = */         virtioGpuR3Attach,
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
