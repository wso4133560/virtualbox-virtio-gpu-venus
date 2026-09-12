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

#define VIRTIOGPU_SAVED_STATE_VERSION UINT32_C(4)
#define VIRTIOGPU_MAX_RESOURCES 256
#define VIRTIOGPU_MAX_CONTEXTS 64
#define VIRTIOGPU_MAX_CONTEXT_RESOURCES 64
#define VIRTIOGPU_MAX_SUBMIT_BYTES (UINT32_C(1) * _1M)
#define VIRTIOGPU_VK_CMD_FILL_BUFFER UINT32_C(118)
#define VIRTIOGPU_VK_CMD_COPY_BUFFER UINT32_C(112)
#define VIRTIOGPU_MAX_BACKING_ENTRIES 64
#define VIRTIOGPU_MAX_RESOURCE_BYTES (UINT64_C(256) * _1M)

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
    bool fVulkanBuffer;
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

static int virtioGpuR3VulkanResourceCreate(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    pRes->hVkBuffer = VK_NULL_HANDLE;
    pRes->hVkMemory = VK_NULL_HANDLE;
    pRes->pvVkMapped = NULL;
    pRes->fVulkanBuffer = false;
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
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_SHARING_MODE_EXCLUSIVE, 0, NULL };
    VkMemoryRequirements MemReq;
    RT_ZERO(MemReq);
    VkMemoryAllocateInfo AllocInfo;
    RT_ZERO(AllocInfo);
    if (pfnCreateBuffer(pThis->hVkDevice, &BufferInfo, NULL, &pRes->hVkBuffer) != VK_SUCCESS)
        return VERR_NOT_SUPPORTED;
    pfnGetRequirements(pThis->hVkDevice, pRes->hVkBuffer, &MemReq);
    uint32_t iMemoryType = UINT32_MAX;
    for (uint32_t i = 0; i < pThis->VkMemoryProperties.memoryTypeCount; ++i)
        if ((MemReq.memoryTypeBits & RT_BIT_32(i))
            && (pThis->VkMemoryProperties.memoryTypes[i].propertyFlags
                & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            iMemoryType = i;
            break;
        }
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
        PFN_vkFreeMemory pfnFreeMemory = (PFN_vkFreeMemory)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkFreeMemory");
        PFN_vkUnmapMemory pfnUnmapMemory = (PFN_vkUnmapMemory)pfnGetDeviceProcAddr(pThis->hVkDevice, "vkUnmapMemory");
        if (pRes->pvVkMapped != NULL && pfnUnmapMemory)
            pfnUnmapMemory(pThis->hVkDevice, pRes->hVkMemory);
        if (pRes->hVkBuffer != VK_NULL_HANDLE && pfnDestroyBuffer)
            pfnDestroyBuffer(pThis->hVkDevice, pRes->hVkBuffer, NULL);
        if (pRes->hVkMemory != VK_NULL_HANDLE && pfnFreeMemory)
            pfnFreeMemory(pThis->hVkDevice, pRes->hVkMemory, NULL);
    }
    pRes->hVkBuffer = VK_NULL_HANDLE;
    pRes->hVkMemory = VK_NULL_HANDLE;
    pRes->pvVkMapped = NULL;
    pRes->fVulkanBuffer = false;
}

static int virtioGpuR3VulkanResourceSync(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes)
{
    RT_NOREF(pThis);
    if (!pRes->fVulkanBuffer || !pRes->pbPixels || !pRes->pvVkMapped)
        return VINF_SUCCESS;
    memcpy(pRes->pvVkMapped, pRes->pbPixels, (size_t)pRes->cbPixels);
    return VINF_SUCCESS;
}

static int virtioGpuR3VulkanFillBuffer(PVIRTIOGPU pThis, PVIRTIOGPURESOURCE pRes,
                                       uint64_t off, uint64_t cb, uint32_t uData)
{
    if (!pRes->fVulkanBuffer || !pThis->fVulkanQueue || !pThis->fVulkanSubmit || off > pRes->cbPixels
        || cb > pRes->cbPixels - off || !cb || (off & 3) || (cb & 3))
        return VERR_INVALID_PARAMETER;
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
    rc = VINF_SUCCESS;
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
    return VINF_SUCCESS;
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
    uint32_t cbRegions;
    uint64_t offSrc;
    uint64_t offDst;
    uint64_t cbCopy;
} VIRTIOGPUCOPYCMD;

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
    if (!pbCommand || !pCopy || cbCommand != 64)
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
    memcpy(&pCopy->offSrc, pbCommand + 40, sizeof(pCopy->offSrc));
    memcpy(&pCopy->offDst, pbCommand + 48, sizeof(pCopy->offDst));
    memcpy(&pCopy->cbCopy, pbCommand + 56, sizeof(pCopy->cbCopy));
    return pCopy->uCommandBuffer != 0 && pCopy->uSrcBuffer != 0 && pCopy->uDstBuffer != 0
        && pCopy->cRegions == 1 && pCopy->cbRegions == 24;
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
                                Resp.Hdr.uType = RT_SUCCESS(virtioGpuR3VulkanCopyBuffer(pThis, pSrc, pDst,
                                                                                          Copy.offSrc, Copy.offDst,
                                                                                          Copy.cbCopy))
                                               ? VIRTIOGPU_RESP_OK_NODATA : VIRTIOGPU_RESP_ERR_UNSPEC;
#else
                                Resp.Hdr.uType = VIRTIOGPU_RESP_ERR_UNSPEC;
#endif
                            }
                        }
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
                if(pBuf->cbPhysSend<sizeof(Cmd)-sizeof(Cmd.Hdr) || RT_FAILURE(virtioGpuR3Read(pDevIns,pVirtio,pBuf,(uint8_t *)&Cmd+sizeof(Cmd.Hdr),sizeof(Cmd)-sizeof(Cmd.Hdr)))) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER;
                else { PVIRTIOGPURESOURCE pRes=virtioGpuR3FindResource(pThis,Cmd.uResourceId); if(!pRes || !virtioGpuR3RectValid(pRes,Cmd.uX,Cmd.uY,Cmd.uWidth,Cmd.uHeight)) Resp.Hdr.uType=VIRTIOGPU_RESP_ERR_INVALID_PARAMETER; else { PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC); for(unsigned i=0;i<VIRTIOGPU_MAX_SCANOUTS;++i) if(pThis->aScanouts[i].uResourceId==Cmd.uResourceId) { pThis->aScanouts[i].uFlushSequence++; if (pThisCC->pDrv && pThisCC->pDrv->pfnUpdateRect) pThisCC->pDrv->pfnUpdateRect(pThisCC->pDrv, Cmd.uX, Cmd.uY, Cmd.uWidth, Cmd.uHeight); } Resp.Hdr.uType=VIRTIOGPU_RESP_OK_NODATA; } }
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
    RT_NOREF(pCfg);
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "", "");
    PVIRTIOGPU pThis = PDMDEVINS_2_DATA(pDevIns, PVIRTIOGPU);
    PVIRTIOGPUCC pThisCC = PDMDEVINS_2_DATA_CC(pDevIns, PVIRTIOGPUCC);
    pThis->Config.cScanouts = 1;
#ifdef VBOX_WITH_VIRTIO_GPU_VENUS
    int rcVulkan = virtioGpuR3VulkanInit(pThis);
    if (RT_FAILURE(rcVulkan))
    {
        LogRel(("virtio-gpu: Vulkan host probe failed (%Rrc), using software path\n", rcVulkan));
        virtioGpuR3VulkanTerm(pThis);
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
