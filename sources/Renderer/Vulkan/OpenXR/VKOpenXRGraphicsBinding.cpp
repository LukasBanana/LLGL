/*
 * VKOpenXRGraphicsBinding.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef LLGL_BUILD_XR_OPENXR


#include "VKOpenXRGraphicsBinding.h"

#include <LLGL/Backend/Vulkan/NativeHandle.h>
#include <LLGL/Report.h>

#include "../../../XR/OpenXR/OpenXRError.h"
#include "../Texture/VKTexture.h"

#include <vector>
#include <cstring>


namespace LLGL
{

namespace OpenXR
{


static const char* g_requiredExtensions[] =
{
    XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME,
};

static Format VkColorFormatToLLGL(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_B8G8R8A8_UNORM:      return Format::BGRA8UNorm;
        case VK_FORMAT_B8G8R8A8_SRGB:       return Format::BGRA8UNorm_sRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:      return Format::RGBA8UNorm;
        case VK_FORMAT_R8G8B8A8_SRGB:       return Format::RGBA8UNorm_sRGB;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return Format::RGBA16Float;
        default:                            return Format::Undefined;
    }
}

static Format VkDepthFormatToLLGL(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_D16_UNORM:           return Format::D16UNorm;
        case VK_FORMAT_D24_UNORM_S8_UINT:   return Format::D24UNormS8UInt;
        case VK_FORMAT_D32_SFLOAT:          return Format::D32Float;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:  return Format::D32FloatS8X24UInt;
        default:                            return Format::Undefined;
    }
}

static VkFormat LLGLToVkFormat(Format format)
{
    switch (format)
    {
        case Format::BGRA8UNorm:            return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8UNorm_sRGB:       return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::RGBA8UNorm:            return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8UNorm_sRGB:       return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::RGBA16Float:           return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::D16UNorm:              return VK_FORMAT_D16_UNORM;
        case Format::D24UNormS8UInt:        return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32Float:              return VK_FORMAT_D32_SFLOAT;
        case Format::D32FloatS8X24UInt:     return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:                            return VK_FORMAT_UNDEFINED;
    }
}

static VkSampleCountFlagBits SampleCountToVkBits(std::uint32_t sampleCount)
{
    switch (sampleCount)
    {
        case 1:  return VK_SAMPLE_COUNT_1_BIT;
        case 2:  return VK_SAMPLE_COUNT_2_BIT;
        case 4:  return VK_SAMPLE_COUNT_4_BIT;
        case 8:  return VK_SAMPLE_COUNT_8_BIT;
        case 16: return VK_SAMPLE_COUNT_16_BIT;
        case 32: return VK_SAMPLE_COUNT_32_BIT;
        case 64: return VK_SAMPLE_COUNT_64_BIT;
        default: return VK_SAMPLE_COUNT_1_BIT;
    }
}

static bool GetVulkanNativeHandle(RenderSystem& renderSystem, Vulkan::RenderSystemNativeHandle& outHandle)
{
    return renderSystem.GetNativeHandle(&outHandle, sizeof(outHandle));
}


VKOpenXRGraphicsBinding::VKOpenXRGraphicsBinding() = default;

VKOpenXRGraphicsBinding::~VKOpenXRGraphicsBinding()
{
    // Destroy Vulkan objects we own. The LLGL Vulkan backend held only weak references,
    // so it's our responsibility to free them. Caller must destroy the LLGL RenderSystem first.
    if (ownedDevice_ != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(ownedDevice_);
        vkDestroyDevice(ownedDevice_, nullptr);
    }
    if (ownedInstance_ != VK_NULL_HANDLE)
        vkDestroyInstance(ownedInstance_, nullptr);
}

const char* VKOpenXRGraphicsBinding::GetRendererModuleName() const
{
    return "Vulkan";
}

ArrayView<const char*> VKOpenXRGraphicsBinding::GetRequiredXrExtensions() const
{
    return ArrayView<const char*>{ g_requiredExtensions, sizeof(g_requiredExtensions) / sizeof(g_requiredExtensions[0]) };
}

bool VKOpenXRGraphicsBinding::LoadFunctions(XrInstance instance, Functions& fns, Report* report)
{
    auto get = [&](const char* name, PFN_xrVoidFunction* outPfn) -> bool
    {
        const XrResult result = xrGetInstanceProcAddr(instance, name, outPfn);
        if (Failed(result))
        {
            ReportXrErrorf(report, instance, result, "xrGetInstanceProcAddr(\"%s\")", name);
            return false;
        }
        return true;
    };

    if (!get("xrGetVulkanGraphicsRequirements2KHR", reinterpret_cast<PFN_xrVoidFunction*>(&fns.getGraphicsRequirements))) return false;
    if (!get("xrCreateVulkanInstanceKHR",           reinterpret_cast<PFN_xrVoidFunction*>(&fns.createInstance)))          return false;
    if (!get("xrGetVulkanGraphicsDevice2KHR",       reinterpret_cast<PFN_xrVoidFunction*>(&fns.getGraphicsDevice)))      return false;
    if (!get("xrCreateVulkanDeviceKHR",             reinterpret_cast<PFN_xrVoidFunction*>(&fns.createDevice)))           return false;
    return true;
}

RenderSystemPtr VKOpenXRGraphicsBinding::CreateRenderSystem(
    XrInstance                          instance,
    XrSystemId                          systemId,
    const XRRenderSystemDescriptor&     renderSystemDesc,
    Report*                             report)
{
    Functions fns;
    if (!LoadFunctions(instance, fns, report))
        return nullptr;

    XrGraphicsRequirementsVulkanKHR gfxRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
    XrResult xrResult = fns.getGraphicsRequirements(instance, systemId, &gfxRequirements);
    if (Failed(xrResult))
    {
        ReportXrError(report, instance, xrResult, "xrGetVulkanGraphicsRequirements2KHR");
        return nullptr;
    }

    // --- Create VkInstance via the OpenXR runtime ---
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName    = "LLGL OpenXR Application";
    appInfo.applicationVersion  = 1;
    appInfo.pEngineName         = "LLGL";
    appInfo.engineVersion       = 1;
    appInfo.apiVersion          = VK_API_VERSION_1_1;

    VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo = &appInfo;

    XrVulkanInstanceCreateInfoKHR xrInstanceCreate{ XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR };
    xrInstanceCreate.systemId               = systemId;
    xrInstanceCreate.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    xrInstanceCreate.vulkanCreateInfo       = &instanceCreateInfo;
    xrInstanceCreate.vulkanAllocator        = nullptr;

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkResult vkResultOut = VK_SUCCESS;
    xrResult = fns.createInstance(instance, &xrInstanceCreate, &vkInstance, &vkResultOut);
    if (Failed(xrResult) || vkResultOut != VK_SUCCESS || vkInstance == VK_NULL_HANDLE)
    {
        ReportXrErrorf(report, instance, xrResult, "xrCreateVulkanInstanceKHR (vkResult=%d)", static_cast<int>(vkResultOut));
        return nullptr;
    }

    // --- Pick the runtime-required physical device ---
    XrVulkanGraphicsDeviceGetInfoKHR getDeviceInfo{ XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR };
    getDeviceInfo.systemId      = systemId;
    getDeviceInfo.vulkanInstance = vkInstance;

    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    xrResult = fns.getGraphicsDevice(instance, &getDeviceInfo, &vkPhysicalDevice);
    if (Failed(xrResult))
    {
        ReportXrError(report, instance, xrResult, "xrGetVulkanGraphicsDevice2KHR");
        vkDestroyInstance(vkInstance, nullptr);
        return nullptr;
    }

    // --- Find a graphics-capable queue family ---
    std::uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    std::uint32_t graphicsFamily = UINT32_MAX;
    for (std::uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            graphicsFamily = i;
            break;
        }
    }
    if (graphicsFamily == UINT32_MAX)
    {
        if (report) report->Errorf("xrCreateVulkanInstanceKHR returned a physical device with no graphics queue family\n");
        vkDestroyInstance(vkInstance, nullptr);
        return nullptr;
    }

    // --- Create VkDevice via the OpenXR runtime ---
    const float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos    = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures     = &features;

    XrVulkanDeviceCreateInfoKHR xrDeviceCreate{ XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR };
    xrDeviceCreate.systemId                 = systemId;
    xrDeviceCreate.pfnGetInstanceProcAddr   = &vkGetInstanceProcAddr;
    xrDeviceCreate.vulkanPhysicalDevice     = vkPhysicalDevice;
    xrDeviceCreate.vulkanCreateInfo         = &deviceCreateInfo;
    xrDeviceCreate.vulkanAllocator          = nullptr;

    VkDevice vkDevice = VK_NULL_HANDLE;
    xrResult = fns.createDevice(instance, &xrDeviceCreate, &vkDevice, &vkResultOut);
    if (Failed(xrResult) || vkResultOut != VK_SUCCESS || vkDevice == VK_NULL_HANDLE)
    {
        ReportXrErrorf(report, instance, xrResult, "xrCreateVulkanDeviceKHR (vkResult=%d)", static_cast<int>(vkResultOut));
        vkDestroyInstance(vkInstance, nullptr);
        return nullptr;
    }

    VkQueue vkQueue = VK_NULL_HANDLE;
    vkGetDeviceQueue(vkDevice, graphicsFamily, 0, &vkQueue);

    // Hand the prepared context to LLGL's Vulkan backend via its existing nativeHandle path.
    Vulkan::RenderSystemNativeHandle nativeHandle{};
    nativeHandle.instance       = vkInstance;
    nativeHandle.physicalDevice = vkPhysicalDevice;
    nativeHandle.device         = vkDevice;
    nativeHandle.queue          = vkQueue;
    nativeHandle.queueFamily    = graphicsFamily;

    RenderSystemDescriptor rsDesc;
    rsDesc.moduleName           = "Vulkan";
    rsDesc.flags                = renderSystemDesc.flags;
    rsDesc.rendererConfig       = renderSystemDesc.rendererConfig;
    rsDesc.rendererConfigSize   = renderSystemDesc.rendererConfigSize;
    rsDesc.nativeHandle         = &nativeHandle;
    rsDesc.nativeHandleSize     = sizeof(nativeHandle);
#ifdef LLGL_OS_ANDROID
    // The Vulkan backend's AndroidApp::Initialize traps if this is null on Android.
    rsDesc.androidApp           = renderSystemDesc.androidApp;
#endif

    RenderSystemPtr renderSystem = RenderSystem::Load(rsDesc, report);
    if (!renderSystem)
    {
        // RenderSystem::Load failed. Tear down the partially-built Vk objects to avoid leaking.
        vkDestroyDevice(vkDevice, nullptr);
        vkDestroyInstance(vkInstance, nullptr);
        return nullptr;
    }

    // Take ownership so we can clean up after the LLGL render system is destroyed.
    ownedInstance_ = vkInstance;
    ownedDevice_   = vkDevice;
    return renderSystem;
}

const void* VKOpenXRGraphicsBinding::GetSessionGraphicsBinding(RenderSystem& renderSystem)
{
    Vulkan::RenderSystemNativeHandle native{};
    if (!GetVulkanNativeHandle(renderSystem, native))
        return nullptr;

    graphicsBinding_                 = XrGraphicsBindingVulkanKHR{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
    graphicsBinding_.instance        = native.instance;
    graphicsBinding_.physicalDevice  = native.physicalDevice;
    graphicsBinding_.device          = native.device;
    graphicsBinding_.queueFamilyIndex = native.queueFamily;
    graphicsBinding_.queueIndex      = 0;
    return &graphicsBinding_;
}

Format VKOpenXRGraphicsBinding::SelectColorFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    // If the application requested a specific format that isn't a color format we know,
    // bail out so OpenXRSession can try SelectDepthFormat instead.
    if (preferred != Format::Undefined)
    {
        const VkFormat preferredVk = LLGLToVkFormat(preferred);
        if (preferredVk == VK_FORMAT_UNDEFINED || VkColorFormatToLLGL(preferredVk) == Format::Undefined)
        {
            outNativeFormat = 0;
            return Format::Undefined;
        }
        for (std::int64_t fmt : runtimeFormats)
        {
            if (static_cast<VkFormat>(fmt) == preferredVk)
            {
                outNativeFormat = fmt;
                return preferred;
            }
        }
        outNativeFormat = 0;
        return Format::Undefined;
    }

    // Preferred=Undefined: caller is asking "what's available?". Return the first color format.
    for (std::int64_t fmt : runtimeFormats)
    {
        const Format llglFmt = VkColorFormatToLLGL(static_cast<VkFormat>(fmt));
        if (llglFmt != Format::Undefined)
        {
            outNativeFormat = fmt;
            return llglFmt;
        }
    }

    outNativeFormat = 0;
    return Format::Undefined;
}

Format VKOpenXRGraphicsBinding::SelectDepthFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    if (preferred != Format::Undefined)
    {
        const VkFormat preferredVk = LLGLToVkFormat(preferred);
        if (preferredVk == VK_FORMAT_UNDEFINED || VkDepthFormatToLLGL(preferredVk) == Format::Undefined)
        {
            outNativeFormat = 0;
            return Format::Undefined;
        }
        for (std::int64_t fmt : runtimeFormats)
        {
            if (static_cast<VkFormat>(fmt) == preferredVk)
            {
                outNativeFormat = fmt;
                return preferred;
            }
        }
        outNativeFormat = 0;
        return Format::Undefined;
    }

    for (std::int64_t fmt : runtimeFormats)
    {
        const Format llglFmt = VkDepthFormatToLLGL(static_cast<VkFormat>(fmt));
        if (llglFmt != Format::Undefined)
        {
            outNativeFormat = fmt;
            return llglFmt;
        }
    }

    outNativeFormat = 0;
    return Format::Undefined;
}

bool VKOpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        nativeFormat,
    SwapchainKind                       kind,
    SmallVector<SwapchainImage>&        outImages,
    Report*                             report)
{
    Vulkan::RenderSystemNativeHandle native{};
    if (!GetVulkanNativeHandle(renderSystem, native))
    {
        if (report) report->Errorf("VKOpenXRGraphicsBinding::EnumerateSwapchainImages failed: render system did not yield a Vulkan native handle\n");
        return false;
    }

    std::uint32_t imageCount = 0;
    XrResult result = xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr);
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateSwapchainImages");
        return false;
    }

    std::vector<XrSwapchainImageVulkanKHR> xrImages(imageCount, XrSwapchainImageVulkanKHR{ XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
    result = xrEnumerateSwapchainImages(
        swapchain,
        imageCount,
        &imageCount,
        reinterpret_cast<XrSwapchainImageBaseHeader*>(xrImages.data())
    );
    if (Failed(result))
    {
        ReportXrError(report, XR_NULL_HANDLE, result, "xrEnumerateSwapchainImages");
        return false;
    }

    outImages.clear();
    outImages.reserve(imageCount);

    VKTexture::AdoptionParams params{};
    params.type             = TextureType::Texture2D;
    params.format           = static_cast<VkFormat>(nativeFormat);
    params.extent           = { swapChainDesc.resolution.width, swapChainDesc.resolution.height, 1u };
    params.numMipLevels     = 1;
    params.numArrayLayers   = swapChainDesc.arrayLayers;
    params.sampleCountBits  = SampleCountToVkBits(swapChainDesc.sampleCount);
    params.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

    if (kind == SwapchainKind::DepthStencil)
    {
        params.bindFlags  = BindFlags::DepthStencilAttachment;
        params.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        params.bindFlags  = BindFlags::ColorAttachment | BindFlags::Sampled;
        params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    for (auto& xrImg : xrImages)
    {
        params.image = xrImg.image;
        auto* texture = new VKTexture(native.device, params);
        SwapchainImage entry;
        entry.texture = texture;
        outImages.push_back(entry);
    }
    return true;
}

void VKOpenXRGraphicsBinding::ReleaseSwapchainImage(RenderSystem& /*renderSystem*/, Texture& texture)
{
    // We constructed these VKTextures directly with `new`, outside the render system's tracking.
    // They wrap externally-owned VkImages, so deleting only the wrapper is correct.
    delete static_cast<VKTexture*>(&texture);
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateVulkanGraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new VKOpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
