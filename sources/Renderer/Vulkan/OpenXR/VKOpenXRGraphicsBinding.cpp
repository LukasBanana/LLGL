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
#include "../../../Core/MacroUtils.h"
#include "../Texture/VKTexture.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../Ext/VKExtensionRegistry.h"

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


static bool GetVulkanNativeHandle(RenderSystem& renderSystem, Vulkan::RenderSystemNativeHandle& outHandle)
{
    return renderSystem.GetNativeHandle(&outHandle, sizeof(outHandle));
}

// Find the first runtime VkFormat matching the requested color/depth bucket and (optionally)
// LLGL Format. Mirrors the D3D11/D3D12 binding's FindRuntimeFormat helper — see the D3D11
// version for rationale around the depth/color bucketing filter.
static Format FindRuntimeFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    bool                        wantDepth,
    std::int64_t&               outNativeFormat)
{
    for (std::int64_t fmt : runtimeFormats)
    {
        const VkFormat vkFmt = static_cast<VkFormat>(fmt);
        if (VKTypes::IsVkFormatDepthStencil(vkFmt) != wantDepth)
            continue;
        const Format llglFmt = VKTypes::Unmap(vkFmt);
        if (llglFmt == Format::Undefined)
            continue;
        if (preferred != Format::Undefined && llglFmt != preferred)
            continue;
        outNativeFormat = fmt;
        return llglFmt;
    }
    outNativeFormat = 0;
    return Format::Undefined;
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
    return ArrayView<const char*>{ g_requiredExtensions, LLGL_ARRAY_LENGTH(g_requiredExtensions) };
}

bool VKOpenXRGraphicsBinding::LoadXRProc(XrInstance instance, Functions &fns, Report *report)
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
    const RenderSystemDescriptor&       renderSystemDesc,
    Report*                             report)
{
    Functions fns;
    if (!LoadXRProc(instance, fns, report))
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

    // Opt the runtime-built instance into the same validation layer and debug extensions the non-XR
    // path enables. Without this, the LLGL Vulkan backend (which gets handed this instance below via
    // nativeHandle) fails to install its debug callback because the extension isn't on the instance.
    // Reuses VKIsInstanceDebugLayer / VKIsInstanceExtensionEnabled so both paths share one source of
    // truth for what gets enabled.
    const bool debugDevice = ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0);

    std::vector<VkLayerProperties>      availableLayers;
    std::vector<VkExtensionProperties>  availableInstanceExts;
    std::vector<const char*>            enabledLayers;
    std::vector<const char*>            enabledInstanceExts;
    if (debugDevice)
    {
        availableLayers = VKQueryInstanceLayerProperties();
        for (const VkLayerProperties& layer : availableLayers)
        {
            if (VKIsInstanceDebugLayer(layer.layerName))
                enabledLayers.push_back(layer.layerName);
        }

        availableInstanceExts = VKQueryInstanceExtensionProperties();
        for (const VkExtensionProperties& ext : availableInstanceExts)
        {
            if (VKIsInstanceExtensionEnabled(ext.extensionName, /*debugLayerEnabled:*/ true))
                enabledInstanceExts.push_back(ext.extensionName);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo        = &appInfo;
    instanceCreateInfo.enabledLayerCount       = static_cast<std::uint32_t>(enabledLayers.size());
    instanceCreateInfo.ppEnabledLayerNames     = enabledLayers.empty() ? nullptr : enabledLayers.data();
    instanceCreateInfo.enabledExtensionCount   = static_cast<std::uint32_t>(enabledInstanceExts.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExts.empty() ? nullptr : enabledInstanceExts.data();

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

    // Enable LLGL's required device extensions (VK_KHR_swapchain, VK_KHR_maintenance1) when the
    // physical device advertises them. The OpenXR runtime adds its own extensions on top of this
    // list inside xrCreateVulkanDeviceKHR, so we don't displace anything by setting these. Without
    // this, an application that initializes LLGL via XR and then tries to create a regular
    // LLGL::SwapChain hits "PRESENT_SRC_KHR requires VK_KHR_swapchain" at vkCreateRenderPass
    // (and would also fail at vkCreateSwapchainKHR). Treat the list as best-effort: skip any the
    // device doesn't support rather than failing device creation outright, because XR-only setups
    // (e.g. headless OpenXR runtimes) don't necessarily need them.
    const std::vector<VkExtensionProperties> availableDeviceExts = VKQueryDeviceExtensionProperties(vkPhysicalDevice);
    std::vector<const char*> enabledDeviceExts;
    for (const char** required = VKGetRequiredDeviceExtensions(); *required != nullptr; ++required)
    {
        for (const VkExtensionProperties& available : availableDeviceExts)
        {
            if (std::strcmp(available.extensionName, *required) == 0)
            {
                enabledDeviceExts.push_back(*required);
                break;
            }
        }
    }

    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures        = &features;
    deviceCreateInfo.enabledExtensionCount   = static_cast<std::uint32_t>(enabledDeviceExts.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExts.empty() ? nullptr : enabledDeviceExts.data();

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

    RenderSystemDescriptor rsDesc = renderSystemDesc;
    rsDesc.nativeHandle         = &nativeHandle;
    rsDesc.nativeHandleSize     = sizeof(nativeHandle);

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
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ false, outNativeFormat);
}

Format VKOpenXRGraphicsBinding::SelectDepthFormat(
    ArrayView<std::int64_t>     runtimeFormats,
    Format                      preferred,
    std::int64_t&               outNativeFormat) const
{
    return FindRuntimeFormat(runtimeFormats, preferred, /*wantDepth:*/ true, outNativeFormat);
}

bool VKOpenXRGraphicsBinding::EnumerateSwapchainImages(
    RenderSystem&                       renderSystem,
    XrSwapchain                         swapchain,
    const XRSwapChainDescriptor&        swapChainDesc,
    std::int64_t                        nativeFormat,
    SwapchainKind                       kind,
    std::vector<XRSwapchainImage>&      outImages,
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

    VKExternalImageInfo params{};
    params.type             = TextureType::Texture2D;
    params.format           = static_cast<VkFormat>(nativeFormat);
    params.extent           = { swapChainDesc.resolution.width, swapChainDesc.resolution.height, 1u };
    params.numMipLevels     = 1;
    params.numArrayLayers   = swapChainDesc.arrayLayers;
    params.sampleCountBits  = VKTypes::ToVkSampleCountBits(swapChainDesc.sampleCount);
    params.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

    if (kind == SwapchainKind::DepthStencil)
    {
        params.bindFlags  = BindFlags::DepthStencilAttachment;
        params.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        // BindFlags is what the LLGL *application* does with the texture, not what the VkImage
        // can technically do. Applications use XR swap-chain images strictly as color attachments;
        // only the OpenXR runtime (compositor / distortion) samples them, and it does that outside
        // LLGL's awareness. Setting BindFlags::Sampled here would trigger LLGL's "render pass ends
        // in SHADER_READ_ONLY_OPTIMAL" auto-transition (see VKRenderTarget::GetFinalLayoutForAttachment),
        // leaving the image in a layout the OpenXR runtime doesn't expect at release time.
        // The VkImage still gets VK_IMAGE_USAGE_SAMPLED_BIT below so the runtime can sample it.
        params.bindFlags  = BindFlags::ColorAttachment;
        params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    for (auto& xrImg : xrImages)
    {
        params.image = xrImg.image;
        XRSwapchainImage entry;
        entry.texture = std::unique_ptr<VKTexture>(new VKTexture(native.device, params));
        outImages.emplace_back(std::move(entry));
    }
    return true;
}


LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateVulkanGraphicsBinding()
{
    return std::unique_ptr<GraphicsBinding>(new VKOpenXRGraphicsBinding());
}


} // /namespace OpenXR

} // /namespace LLGL


#endif // LLGL_BUILD_XR_OPENXR



// ================================================================================
