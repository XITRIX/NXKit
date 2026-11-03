#include <platforms/switch/SkiaCtx_switch.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_VI_NN
#include <vulkan/vulkan.h>
#include <switch.h>

#include "SwitchPipelineCache.hpp"
#include "include/core/SkGraphics.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "include/gpu/vk/VulkanPreferredFeatures.h"
#include "include/ports/SkFontMgr_data.h"
// Private implementation header stays inside this platform translation unit.
// Use Skia's matching VMA implementation rather than compiling a second one.
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorWrapper.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

extern "C" void nxkitEnsureSwitchRuntimeLinked();
extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance, const char*);

using namespace NXKit;

namespace {

bool checkVk(VkResult result, const char* operation) {
    if (result == VK_SUCCESS) return true;
    SkDebugf("Switch Vulkan %s failed: %d\n", operation, static_cast<int>(result));
    return false;
}

bool appendSharedFont(PlSharedFontType type, const char* name,
                      std::vector<sk_sp<SkData>>& data) {
    PlFontData font{};
    const Result result = plGetSharedFontByType(&font, type);
    if (R_FAILED(result) || !font.address || !font.size) {
        SkDebugf("Switch %s shared font unavailable: 0x%x\n", name, result);
        return false;
    }
    data.push_back(SkData::MakeWithoutCopy(font.address, font.size));
    return true;
}

bool hasExtension(const std::vector<VkExtensionProperties>& extensions, const char* name) {
    return std::any_of(extensions.begin(), extensions.end(), [name](const auto& value) {
        return std::strcmp(value.extensionName, name) == 0;
    });
}

// The Horizon driver is linked statically and exports vkGetInstanceProcAddr.
// Resolve every other entry point explicitly; no desktop Vulkan loader/dlopen.
#define INSTANCE_PROCS(X) \
    X(DestroyInstance) X(EnumeratePhysicalDevices) X(GetPhysicalDeviceProperties) \
    X(GetPhysicalDeviceMemoryProperties) \
    X(GetPhysicalDeviceQueueFamilyProperties) X(GetPhysicalDeviceSurfaceSupportKHR) \
    X(GetPhysicalDeviceSurfaceCapabilitiesKHR) X(GetPhysicalDeviceSurfaceFormatsKHR) \
    X(EnumerateDeviceExtensionProperties) X(GetPhysicalDeviceFeatures2) \
    X(CreateDevice) X(CreateViSurfaceNN) X(DestroySurfaceKHR) X(GetDeviceProcAddr)
#define DEVICE_PROCS(X) \
    X(DestroyDevice) X(GetDeviceQueue) X(DeviceWaitIdle) \
    X(CreateSwapchainKHR) X(DestroySwapchainKHR) X(GetSwapchainImagesKHR) \
    X(AcquireNextImageKHR) X(QueuePresentKHR) X(CreateSemaphore) X(DestroySemaphore)

struct VulkanFunctions {
#define DECLARE(name) PFN_vk##name name = nullptr;
    INSTANCE_PROCS(DECLARE)
    DEVICE_PROCS(DECLARE)
#undef DECLARE
    bool loadInstance(VkInstance instance) {
#define LOAD(name) \
        name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk" #name)); \
        if (!name) { SkDebugf("Missing Vulkan instance function vk%s\n", #name); return false; }
        INSTANCE_PROCS(LOAD)
#undef LOAD
        return true;
    }
    bool loadDevice(VkDevice device) {
#define LOAD(name) \
        name = reinterpret_cast<PFN_vk##name>(GetDeviceProcAddr(device, "vk" #name)); \
        if (!name) { SkDebugf("Missing Vulkan device function vk%s\n", #name); return false; }
        DEVICE_PROCS(LOAD)
#undef LOAD
        return true;
    }
};
#undef INSTANCE_PROCS
#undef DEVICE_PROCS

// Uses the same VMA implementation already linked into Skia. Allocation
// metadata and mapping follow Skia's VulkanAMDMemoryAllocator (Copyright 2018
// Google LLC; BSD-3-Clause, see licenses/Skia-Vulkan-Allocator-LICENSE).
// Horizon's NVK memory type is unified and GPU-accessible but may omit
// DEVICE_LOCAL. Treat locality as a preference, not an allocation requirement.
class SwitchAllocator final : public skgpu::VulkanMemoryAllocator {
public:
    static sk_sp<SwitchAllocator> Make(const skgpu::VulkanBackendContext& backend) {
        VmaVulkanFunctions functions{};
#define INSTANCE(name) functions.vk##name = reinterpret_cast<PFN_vk##name>( \
        backend.fGetProc("vk" #name, backend.fInstance, VK_NULL_HANDLE));
#define DEVICE(name) functions.vk##name = reinterpret_cast<PFN_vk##name>( \
        backend.fGetProc("vk" #name, backend.fInstance, backend.fDevice));
        INSTANCE(GetPhysicalDeviceProperties)
        INSTANCE(GetPhysicalDeviceMemoryProperties)
        DEVICE(AllocateMemory) DEVICE(FreeMemory) DEVICE(MapMemory) DEVICE(UnmapMemory)
        DEVICE(FlushMappedMemoryRanges) DEVICE(InvalidateMappedMemoryRanges)
        DEVICE(BindBufferMemory) DEVICE(BindImageMemory)
        DEVICE(GetBufferMemoryRequirements) DEVICE(GetImageMemoryRequirements)
        DEVICE(CreateBuffer) DEVICE(DestroyBuffer) DEVICE(CreateImage) DEVICE(DestroyImage)
        DEVICE(CmdCopyBuffer)
#undef INSTANCE
#undef DEVICE
#define ALIAS(field, name) functions.field = reinterpret_cast<PFN_##field>( \
        backend.fGetProc(#name, backend.fInstance, backend.fDevice));
        ALIAS(vkGetBufferMemoryRequirements2KHR, vkGetBufferMemoryRequirements2)
        ALIAS(vkGetImageMemoryRequirements2KHR, vkGetImageMemoryRequirements2)
        ALIAS(vkBindBufferMemory2KHR, vkBindBufferMemory2)
        ALIAS(vkBindImageMemory2KHR, vkBindImageMemory2)
#undef ALIAS
        functions.vkGetPhysicalDeviceMemoryProperties2KHR =
            reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(
                backend.fGetProc("vkGetPhysicalDeviceMemoryProperties2",
                                 backend.fInstance, VK_NULL_HANDLE));
        functions.vkGetPhysicalDeviceProperties2KHR =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
                backend.fGetProc("vkGetPhysicalDeviceProperties2",
                                 backend.fInstance, VK_NULL_HANDLE));
        VmaAllocatorCreateInfo info{};
        info.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT |
                     VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        info.instance = backend.fInstance;
        info.physicalDevice = backend.fPhysicalDevice;
        info.device = backend.fDevice;
        info.preferredLargeHeapBlockSize = 4 * 1024 * 1024;
        info.pVulkanFunctions = &functions;
        info.vulkanApiVersion = VK_API_VERSION_1_1;
        VmaAllocator allocator = VK_NULL_HANDLE;
        if (!checkVk(vmaCreateAllocator(&info, &allocator), "create VMA allocator")) return nullptr;
        return sk_sp<SwitchAllocator>(new SwitchAllocator(allocator));
    }
    ~SwitchAllocator() override { vmaDestroyAllocator(allocator); }

    VkResult allocateImageMemory(VkImage image, uint32_t flags,
                                  skgpu::VulkanBackendMemory* memory) override {
        auto info = allocationInfo(flags);
        info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (flags & kLazyAllocation_AllocationPropertyFlag)
            info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        VmaAllocation allocation = VK_NULL_HANDLE;
        const auto result = vmaAllocateMemoryForImage(allocator, image, &info, &allocation, nullptr);
        if (result == VK_SUCCESS) *memory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
        checkVk(result, "allocate image memory");
        return result;
    }
    VkResult allocateBufferMemory(VkBuffer buffer, BufferUsage usage, uint32_t flags,
                                   skgpu::VulkanBackendMemory* memory) override {
        auto info = allocationInfo(flags);
        info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (usage != BufferUsage::kGpuOnly) {
            info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            info.preferredFlags |= usage == BufferUsage::kTransfersFromGpuToCpu
                ? VK_MEMORY_PROPERTY_HOST_CACHED_BIT : VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        VmaAllocation allocation = VK_NULL_HANDLE;
        const auto result = vmaAllocateMemoryForBuffer(allocator, buffer, &info, &allocation, nullptr);
        if (result == VK_SUCCESS) *memory = reinterpret_cast<skgpu::VulkanBackendMemory>(allocation);
        if (result != VK_SUCCESS)
            SkDebugf("Switch Vulkan buffer allocation failed: %d (usage %d, flags %u).\n",
                     static_cast<int>(result), static_cast<int>(usage), flags);
        return result;
    }
    void getAllocInfo(const skgpu::VulkanBackendMemory& memory, skgpu::VulkanAlloc* info) const override {
        VmaAllocationInfo allocation{};
        vmaGetAllocationInfo(allocator, reinterpret_cast<VmaAllocation>(memory), &allocation);
        VkMemoryPropertyFlags properties = 0;
        vmaGetMemoryTypeProperties(allocator, allocation.memoryType, &properties);
        info->fMemory = allocation.deviceMemory;
        info->fOffset = allocation.offset;
        info->fSize = allocation.size;
        info->fBackendMemory = memory;
        info->fFlags = 0;
        if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            info->fFlags |= skgpu::VulkanAlloc::kMappable_Flag;
        if (!(properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            info->fFlags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
        if (properties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            info->fFlags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;
    }
    VkResult mapMemory(const skgpu::VulkanBackendMemory& memory, void** data) override {
        const auto result = vmaMapMemory(allocator, reinterpret_cast<VmaAllocation>(memory), data);
        checkVk(result, "map memory");
        return result;
    }
    void unmapMemory(const skgpu::VulkanBackendMemory& memory) override {
        vmaUnmapMemory(allocator, reinterpret_cast<VmaAllocation>(memory));
    }
    VkResult flushMemory(const skgpu::VulkanBackendMemory& memory, VkDeviceSize offset,
                           VkDeviceSize size) override {
        return vmaFlushAllocation(allocator, reinterpret_cast<VmaAllocation>(memory), offset, size);
    }
    VkResult invalidateMemory(const skgpu::VulkanBackendMemory& memory, VkDeviceSize offset,
                                VkDeviceSize size) override {
        return vmaInvalidateAllocation(allocator, reinterpret_cast<VmaAllocation>(memory), offset, size);
    }
    void freeMemory(const skgpu::VulkanBackendMemory& memory) override {
        vmaFreeMemory(allocator, reinterpret_cast<VmaAllocation>(memory));
    }
    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override {
        VmaTotalStatistics stats{};
        vmaCalculateStatistics(allocator, &stats);
        return {stats.total.statistics.blockBytes, stats.total.statistics.allocationBytes};
    }
private:
    explicit SwitchAllocator(VmaAllocator value) : allocator(value) {}
    VmaAllocator allocator;
    static VmaAllocationCreateInfo allocationInfo(uint32_t flags) {
        VmaAllocationCreateInfo info{};
        if (flags & kDedicatedAllocation_AllocationPropertyFlag)
            info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        if (flags & kPersistentlyMapped_AllocationPropertyFlag)
            info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if (flags & kProtected_AllocationPropertyFlag)
            info.requiredFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        return info;
    }
};
} // namespace

struct SkiaCtx_switch::GraphiteState {
    VulkanFunctions vk;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;
    VkSurfaceKHR nativeSurface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};
    VkImageUsageFlags imageUsage = 0;
    struct Image {
        VkImage image = VK_NULL_HANDLE;
        // Presentation consumes this semaphore before the same image can be
        // acquired again. Keep one per image, not one per CPU frame.
        VkSemaphore rendered = VK_NULL_HANDLE;
        sk_sp<SkSurface> surface;
    };
    std::vector<Image> images;
    VkSemaphore acquired = VK_NULL_HANDLE;
    uint32_t imageIndex = 0;
    bool frameAcquired = false;
    bool frameSubmitted = false;
    bool failed = false;
    uint64_t frames = 0;
    sk_sp<skgpu::VulkanMemoryAllocator> allocator;
    std::unique_ptr<Internal::SwitchPipelineCache> pipelineCache;
    std::unique_ptr<skgpu::graphite::Context> context;
    std::unique_ptr<skgpu::graphite::Recorder> recorder;
};

SkiaCtx_switch::SkiaCtx_switch() {
    nxkitEnsureSwitchRuntimeLinked();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());

    // SDL owns lifecycle/input/window sizing. Vulkan presentation uses libnx's
    // NWindow directly because SDL's Switch driver has no Vulkan surface hook.
    window = SDL_CreateWindow("NXKit", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL window creation failed: ") + SDL_GetError());
    }

    SkGraphics::Init();
    std::vector<sk_sp<SkData>> fonts;
    const bool standard = appendSharedFont(PlSharedFontType_Standard, "standard", fonts);
    appendSharedFont(PlSharedFontType_NintendoExt, "Nintendo extended", fonts);
    if (standard) {
        if (auto manager = SkFontMgr_New_Custom_Data(SkSpan(fonts))) fontMgr = std::move(manager);
    }

    if (!initContext()) {
        // Keep partially initialized state owned so normal destruction can
        // release the Vulkan objects. The run loop fails cleanly, before UI
        // rendering can dereference a missing context.
        graphite->failed = true;
        SkDebugf("Switch Graphite/Vulkan initialization failed.\n");
    }
}

SkiaCtx_switch::~SkiaCtx_switch() {
    surface.reset();
    if (graphite) {
        auto& g = *graphite;
        finishGraphiteWork(g.context.get(), g.recorder.get());
        if (g.device && g.vk.DeviceWaitIdle) g.vk.DeviceWaitIdle(g.device);
        destroySwapchain();
        if (g.recorder) g.recorder->freeGpuResources();
        g.recorder.reset();
        if (g.context) {
            g.context->freeGpuResources();
            g.context->submit(skgpu::graphite::SyncToCpu::kYes);
            g.context->syncPipelineData();
        }
        if (g.pipelineCache) g.pipelineCache->saveKeys();
        g.context.reset();
        g.pipelineCache.reset();
        g.allocator.reset();
        if (g.device && g.vk.DestroyDevice) g.vk.DestroyDevice(g.device, nullptr);
        if (g.nativeSurface && g.vk.DestroySurfaceKHR)
            g.vk.DestroySurfaceKHR(g.instance, g.nativeSurface, nullptr);
        if (g.instance && g.vk.DestroyInstance) g.vk.DestroyInstance(g.instance, nullptr);
        graphite.reset();
    }
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

bool SkiaCtx_switch::initContext() {
    graphite = std::make_unique<GraphiteState>();
    auto& g = *graphite;
    auto enumerateExtensions = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));
    auto createInstance = reinterpret_cast<PFN_vkCreateInstance>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
    if (!enumerateExtensions || !createInstance) return false;

    uint32_t count = 0;
    if (!checkVk(enumerateExtensions(nullptr, &count, nullptr), "enumerate instance extensions"))
        return false;
    std::vector<VkExtensionProperties> available(count);
    if (!checkVk(enumerateExtensions(nullptr, &count, available.data()), "read instance extensions"))
        return false;
    for (auto name : {VK_KHR_SURFACE_EXTENSION_NAME, VK_NN_VI_SURFACE_EXTENSION_NAME}) {
        if (!hasExtension(available, name)) {
            SkDebugf("NVK is missing required instance extension %s\n", name);
            return false;
        }
    }

    constexpr uint32_t apiVersion = VK_API_VERSION_1_1;
    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(apiVersion);
    std::vector<const char*> instanceExtensions{
        VK_KHR_SURFACE_EXTENSION_NAME, VK_NN_VI_SURFACE_EXTENSION_NAME};
    preferred.addToInstanceExtensions(available.data(), available.size(), instanceExtensions);
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "NXKit";
    app.apiVersion = apiVersion;
    VkInstanceCreateInfo instanceInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceInfo.pApplicationInfo = &app;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
    if (!checkVk(createInstance(&instanceInfo, nullptr, &g.instance), "create instance") ||
        !g.vk.loadInstance(g.instance)) return false;

    VkViSurfaceCreateInfoNN surfaceInfo{VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN};
    surfaceInfo.window = nwindowGetDefault();
    if (!checkVk(g.vk.CreateViSurfaceNN(g.instance, &surfaceInfo, nullptr, &g.nativeSurface),
                 "create VI surface")) return false;

    if (!checkVk(g.vk.EnumeratePhysicalDevices(g.instance, &count, nullptr),
                 "enumerate physical devices") || !count) return false;
    std::vector<VkPhysicalDevice> devices(count);
    if (!checkVk(g.vk.EnumeratePhysicalDevices(g.instance, &count, devices.data()),
                 "read physical devices")) return false;
    for (auto device : devices) {
        VkPhysicalDeviceProperties properties{};
        g.vk.GetPhysicalDeviceProperties(device, &properties);
        if (properties.apiVersion < apiVersion) continue;
        uint32_t familyCount = 0;
        g.vk.GetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        g.vk.GetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
        for (uint32_t family = 0; family < familyCount; ++family) {
            VkBool32 present = VK_FALSE;
            if (families[family].queueCount &&
                (families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                g.vk.GetPhysicalDeviceSurfaceSupportKHR(device, family, g.nativeSurface, &present)
                    == VK_SUCCESS && present) {
                g.physicalDevice = device;
                g.queueFamily = family;
                SkDebugf("Switch Vulkan GPU: %s (API %u.%u.%u)\n", properties.deviceName,
                         VK_VERSION_MAJOR(properties.apiVersion),
                         VK_VERSION_MINOR(properties.apiVersion),
                         VK_VERSION_PATCH(properties.apiVersion));
                break;
            }
        }
        if (g.physicalDevice) break;
    }
    if (!g.physicalDevice) return false;
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    g.vk.GetPhysicalDeviceMemoryProperties(g.physicalDevice, &memoryProperties);
    for (uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index)
        SkDebugf("Switch Vulkan memory type %u: flags 0x%x heap %u\n", index,
                 memoryProperties.memoryTypes[index].propertyFlags,
                 memoryProperties.memoryTypes[index].heapIndex);

    if (!checkVk(g.vk.EnumerateDeviceExtensionProperties(g.physicalDevice, nullptr, &count, nullptr),
                 "enumerate device extensions")) return false;
    available.resize(count);
    if (!checkVk(g.vk.EnumerateDeviceExtensionProperties(
            g.physicalDevice, nullptr, &count, available.data()), "read device extensions") ||
        !hasExtension(available, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) return false;
    VkPhysicalDeviceFeatures2 supported{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    preferred.addFeaturesToQuery(available.data(), available.size(), supported);
    g.vk.GetPhysicalDeviceFeatures2(g.physicalDevice, &supported);
    VkPhysicalDeviceFeatures2 enabled{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    preferred.addFeaturesToEnable(deviceExtensions, enabled);
    // Core features used by Graphite; never enable an unsupported feature.
    enabled.features.dualSrcBlend = supported.features.dualSrcBlend;
    enabled.features.sampleRateShading = supported.features.sampleRateShading;
    enabled.features.drawIndirectFirstInstance = supported.features.drawIndirectFirstInstance;

    const float priority = 1;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueInfo.queueFamilyIndex = g.queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;
    VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceInfo.pNext = &enabled;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (!checkVk(g.vk.CreateDevice(g.physicalDevice, &deviceInfo, nullptr, &g.device),
                 "create device") || !g.vk.loadDevice(g.device)) return false;
    g.vk.GetDeviceQueue(g.device, g.queueFamily, 0, &g.queue);

    const skgpu::VulkanGetProc getProc = [&g](const char* name, VkInstance instance, VkDevice device) {
        return device ? g.vk.GetDeviceProcAddr(device, name)
                      : vkGetInstanceProcAddr(instance, name);
    };
    skgpu::VulkanExtensions extensions;
    extensions.init(getProc, g.instance, g.physicalDevice,
                    static_cast<uint32_t>(instanceExtensions.size()), instanceExtensions.data(),
                    static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data());
    skgpu::VulkanBackendContext backend;
    backend.fInstance = g.instance;
    backend.fPhysicalDevice = g.physicalDevice;
    backend.fDevice = g.device;
    backend.fQueue = g.queue;
    backend.fGraphicsQueueIndex = g.queueFamily;
    backend.fMaxAPIVersion = apiVersion;
    backend.fVkExtensions = &extensions;
    backend.fDeviceFeatures2 = &enabled;
    backend.fGetProc = getProc;
    g.allocator = SwitchAllocator::Make(backend);
    if (!g.allocator) return false;
    backend.fMemoryAllocator = g.allocator;
    g.pipelineCache = std::make_unique<Internal::SwitchPipelineCache>();
    skgpu::graphite::ContextOptions options;
    options.fInternalMultisampleCount = skgpu::graphite::SampleCount::k1;
    options.fRequireOrderedRecordings = true;
    g.pipelineCache->configure(options);
    g.context = skgpu::graphite::ContextFactory::MakeVulkan(backend, options);
    if (!g.context) return false;
    g.recorder = createGraphiteRecorder(g.context.get());
    if (!g.recorder || !configureSurface()) return false;
    g.pipelineCache->prewarm(g.context.get(), g.format);
    SkDebugf("Switch Graphite/Vulkan ready (%ux%u).\n", g.extent.width, g.extent.height);
    return true;
}

void SkiaCtx_switch::destroySwapchain() {
    if (!graphite || !graphite->device) return;
    auto& g = *graphite;
    // Caller must first finish Graphite work and wait for device/presentation.
    surface.reset();
    for (auto& image : g.images) {
        image.surface.reset();
        if (image.rendered) g.vk.DestroySemaphore(g.device, image.rendered, nullptr);
    }
    g.images.clear();
    if (g.acquired) g.vk.DestroySemaphore(g.device, g.acquired, nullptr);
    g.acquired = VK_NULL_HANDLE;
    if (g.swapchain) g.vk.DestroySwapchainKHR(g.device, g.swapchain, nullptr);
    g.swapchain = VK_NULL_HANDLE;
    g.frameAcquired = g.frameSubmitted = false;
}

bool SkiaCtx_switch::configureSurface() {
    auto& g = *graphite;
    if (!g.device || !g.nativeSurface) return false;
    if (g.swapchain) {
        if (!finishGraphiteWork(g.context.get(), g.recorder.get()) ||
            !checkVk(g.vk.DeviceWaitIdle(g.device), "wait before resize")) return false;
        destroySwapchain();
    }
    VkSurfaceCapabilitiesKHR caps{};
    if (!checkVk(g.vk.GetPhysicalDeviceSurfaceCapabilitiesKHR(
            g.physicalDevice, g.nativeSurface, &caps), "query surface capabilities")) return false;
    uint32_t count = 0;
    if (!checkVk(g.vk.GetPhysicalDeviceSurfaceFormatsKHR(
            g.physicalDevice, g.nativeSurface, &count, nullptr), "query surface formats") ||
        count == 0) return false;
    std::vector<VkSurfaceFormatKHR> formats(count);
    if (!checkVk(g.vk.GetPhysicalDeviceSurfaceFormatsKHR(
            g.physicalDevice, g.nativeSurface, &count, formats.data()), "read surface formats"))
        return false;
    auto format = std::find_if(formats.begin(), formats.end(), [](const auto& value) {
        return value.format == VK_FORMAT_R8G8B8A8_UNORM &&
               value.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });
    if (format == formats.end()) format = std::find_if(formats.begin(), formats.end(),
        [](const auto& value) { return value.format == VK_FORMAT_B8G8R8A8_UNORM; });
    if (format == formats.end()) {
        SkDebugf("NVK surface has no supported unorm RGBA/BGRA format.\n");
        return false;
    }
    g.format = format->format;
    const auto size = getSize();
    if (size.width <= 0 || size.height <= 0) return false;
    _size = size;
    g.extent = caps.currentExtent;
    if (g.extent.width == std::numeric_limits<uint32_t>::max()) {
        g.extent = {
            std::clamp(static_cast<uint32_t>(size.width),
                       caps.minImageExtent.width, caps.maxImageExtent.width),
            std::clamp(static_cast<uint32_t>(size.height),
                       caps.minImageExtent.height, caps.maxImageExtent.height)};
    }
    // Backdrop filters sample/copy the rendered surface; attachment-only
    // swapchains are insufficient for NXKit's material rendering.
    g.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                   VK_IMAGE_USAGE_SAMPLED_BIT |
                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if ((caps.supportedUsageFlags & g.imageUsage) != g.imageUsage) {
        SkDebugf("NVK swapchain lacks required sampling/copy usage flags.\n");
        return false;
    }
    uint32_t imageCount = std::max(caps.minImageCount, 3u);
    if (caps.maxImageCount) imageCount = std::min(imageCount, caps.maxImageCount);
    VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    info.surface = g.nativeSurface;
    info.minImageCount = imageCount;
    info.imageFormat = g.format;
    info.imageColorSpace = format->colorSpace;
    info.imageExtent = g.extent;
    info.imageArrayLayers = 1;
    info.imageUsage = g.imageUsage;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = caps.currentTransform;
    info.compositeAlpha = (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
        ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
        : static_cast<VkCompositeAlphaFlagBitsKHR>(
            caps.supportedCompositeAlpha & (~caps.supportedCompositeAlpha + 1u));
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    info.clipped = VK_TRUE;
    if (!checkVk(g.vk.CreateSwapchainKHR(g.device, &info, nullptr, &g.swapchain),
                 "create swapchain")) return false;
    if (!checkVk(g.vk.GetSwapchainImagesKHR(g.device, g.swapchain, &count, nullptr),
                 "query swapchain images") || count == 0) return false;
    std::vector<VkImage> images(count);
    if (!checkVk(g.vk.GetSwapchainImagesKHR(g.device, g.swapchain, &count, images.data()),
                 "read swapchain images")) return false;
    g.images.resize(count);
    for (uint32_t index = 0; index < count; ++index) {
        auto& image = g.images[index];
        image.image = images[index];
        VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (!checkVk(g.vk.CreateSemaphore(g.device, &semaphoreInfo, nullptr, &image.rendered),
                     "create render semaphore")) return false;
        skgpu::graphite::VulkanTextureInfo texture;
        texture.fFormat = g.format;
        texture.fImageUsageFlags = g.imageUsage;
        auto backendTexture = skgpu::graphite::BackendTextures::MakeVulkan(
            SkISize::Make(g.extent.width, g.extent.height), texture,
            VK_IMAGE_LAYOUT_UNDEFINED, g.queueFamily, image.image, {});
        image.surface = SkSurfaces::WrapBackendTexture(g.recorder.get(), backendTexture,
                                                        nullptr, nullptr);
        if (!image.surface) return false;
    }
    return true;
}

sk_sp<SkSurface> SkiaCtx_switch::getBackbufferSurface() {
    if (!graphite || graphite->failed || !graphite->swapchain) return nullptr;
    auto& g = *graphite;
    if (surface) return surface;
    const auto size = getSize();
    if (size.width != _size.width || size.height != _size.height) {
        if (!configureSurface()) { g.failed = true; return nullptr; }
    }
    VkSemaphoreCreateInfo info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (!checkVk(g.vk.CreateSemaphore(g.device, &info, nullptr, &g.acquired),
                 "create acquire semaphore")) { g.failed = true; return nullptr; }
    const auto result = g.vk.AcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX,
                                                 g.acquired, VK_NULL_HANDLE, &g.imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        g.vk.DestroySemaphore(g.device, g.acquired, nullptr);
        g.acquired = VK_NULL_HANDLE;
        if (!configureSurface()) g.failed = true;
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        checkVk(result, "acquire image");
        g.failed = true;
        return nullptr;
    }
    g.frameAcquired = true;
    g.frameSubmitted = false;
    surface = g.images[g.imageIndex].surface;
    return surface;
}

void SkiaCtx_switch::flushAndSubmit(sk_sp<SkSurface> frameSurface) {
    auto& g = *graphite;
    if (!g.frameAcquired || g.failed || !frameSurface) return;
    auto wait = skgpu::graphite::BackendSemaphores::MakeVulkan(g.acquired);
    auto signal = skgpu::graphite::BackendSemaphores::MakeVulkan(g.images[g.imageIndex].rendered);
    auto state = skgpu::MutableTextureStates::MakeVulkan(
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, g.queueFamily);
    skgpu::graphite::InsertRecordingInfo info;
    info.fTargetSurface = frameSurface.get();
    info.fTargetTextureState = &state;
    info.fNumWaitSemaphores = info.fNumSignalSemaphores = 1;
    info.fWaitSemaphores = &wait;
    info.fSignalSemaphores = &signal;
    struct AcquireRelease {
        PFN_vkDestroySemaphore destroy;
        VkDevice device;
        VkSemaphore semaphore;
    };
    info.fFinishedContext = new AcquireRelease{g.vk.DestroySemaphore, g.device, g.acquired};
    info.fFinishedProc = [](void* pointer, skgpu::CallbackResult) {
        std::unique_ptr<AcquireRelease> release(static_cast<AcquireRelease*>(pointer));
        release->destroy(release->device, release->semaphore, nullptr);
    };
    // The finished callback takes ownership only once insertRecording is called.
    // flushAndSubmitFrame always snaps before insertion, so keep failure cleanup
    // in the submission helper's documented callback contract.
    g.acquired = VK_NULL_HANDLE;
    g.frameSubmitted = flushAndSubmitFrame(std::move(frameSurface), info);
    if (!g.frameSubmitted) g.failed = true;
}

void SkiaCtx_switch::swapBuffers() {
    if (!graphite || graphite->failed || !graphite->frameSubmitted) return;
    auto& g = *graphite;
    VkPresentInfoKHR info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &g.images[g.imageIndex].rendered;
    info.swapchainCount = 1;
    info.pSwapchains = &g.swapchain;
    info.pImageIndices = &g.imageIndex;
    const auto result = g.vk.QueuePresentKHR(g.queue, &info);
    surface.reset();
    g.frameAcquired = g.frameSubmitted = false;
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        if (!configureSurface()) g.failed = true;
    } else if (!checkVk(result, "present")) {
        g.failed = true;
    }
    ++g.frames;
    // No SD-card writes in frame submission. Persist on orderly shutdown and
    // in the explicit shader-bake mode; compilation callbacks only collect RAM.
    if (g.frames == 1 || g.frames % 600 == 0)
        SkDebugf("Switch Vulkan presented frame %llu.\n",
                 static_cast<unsigned long long>(g.frames));
}

skgpu::graphite::Context* SkiaCtx_switch::graphiteContext() {
    return graphite ? graphite->context.get() : nullptr;
}
skgpu::graphite::Recorder* SkiaCtx_switch::graphiteRecorder() {
    return graphite ? graphite->recorder.get() : nullptr;
}
UIUserInterfaceStyle SkiaCtx_switch::getThemeMode() {
    ColorSetId style = ColorSetId_Light;
    setsysGetColorSetId(&style);
    return style == ColorSetId_Dark ? UIUserInterfaceStyle::dark : UIUserInterfaceStyle::light;
}
NXSize SkiaCtx_switch::getSize() {
    int width = 0, height = 0;
    if (window) SDL_GetWindowSize(window, &width, &height);
    return {static_cast<NXFloat>(width), static_cast<NXFloat>(height)};
}
float SkiaCtx_switch::getScaleFactor() { return 1; }
bool SkiaCtx_switch::platformRunLoop(std::function<bool()> loop) {
    if (!graphite || graphite->failed) return false;
    currentRunLoopStartTimer = Timer();
    return loop();
}
std::unique_ptr<SkiaCtx> NXKit::MakeSkiaCtx() {
    return std::make_unique<SkiaCtx_switch>();
}
