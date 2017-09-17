/*
 * VKRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderContext.h"
#include "VKCommandBuffer.h"
#include "VKCore.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"
#include <set>


namespace LLGL
{


/* ----- Common ----- */

static const std::vector<const char*> g_deviceExtensions
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VKRenderContext::VKRenderContext(
    const VKPtr<VkInstance>& instance, VkPhysicalDevice physicalDevice, RenderContextDescriptor desc, const std::shared_ptr<Surface>& surface) :
        instance_            { instance                       },
        physicalDevice_      { physicalDevice                 },
        device_              { vkDestroyDevice                },
        surface_             { instance, vkDestroySurfaceKHR  },
        swapChain_           { device_, vkDestroySwapchainKHR },
        swapChainRenderPass_ { device_, vkDestroyRenderPass   }
{
    SetOrCreateSurface(surface, desc.videoMode, nullptr);
    CreateVkSurface();
    CreateLogicalDevice();
    CreateSwapChain(desc.videoMode);
    CreateSwapChainImageViews();
    CreateSwapChainRenderPass();
    CreateSwapChainFramebuffers();
    CreatePresentSemaphorse();
}

void VKRenderContext::Present()
{
    /* Get next image for presentation */
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device_, swapChain_, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex);

    /* Initialize semaphorse */
    VkSemaphore waitSemaphorse[] = { imageAvailableSemaphore_ };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphorse[] = { renderFinishedSemaphore_ };
    VkCommandBuffer commandBuffers[] = { commandBuffer_->GetCommandBuffer(imageIndex) };

    /* Submit command buffer to graphics queue */
    VkSubmitInfo submitInfo;

    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = nullptr;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphorse;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = commandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphorse;

    VkResult result = vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    VKThrowIfFailed(result, "failed to submit Vulkan graphics queue");

    /* Present result on screen */
    VkSwapchainKHR swapChains[] = { swapChain_ };

    VkPresentInfoKHR presentInfo;

    presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext               = nullptr;
    presentInfo.waitSemaphoreCount  = 1;
    presentInfo.pWaitSemaphores     = signalSemaphorse;
    presentInfo.swapchainCount      = 1;
    presentInfo.pSwapchains         = swapChains;
    presentInfo.pImageIndices       = &imageIndex;
    presentInfo.pResults            = nullptr;

    result = vkQueuePresentKHR(presentQueue_, &presentInfo);
    VKThrowIfFailed(result, "failed to present Vulkan graphics queue");
}

/* ----- Configuration ----- */

void VKRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    //todo
}

void VKRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    //todo
}

/* --- Extended functions --- */

void VKRenderContext::SetPresentCommandBuffer(VKCommandBuffer* commandBuffer)
{
    commandBuffer_ = commandBuffer;
}


/*
 * ======= Private: =======
 */

void VKRenderContext::CreateVkSurface()
{
    /* Get hantive handle from context surface */
    NativeHandle nativeHandle;
    GetSurface().GetNativeHandle(&nativeHandle);

    #if defined LLGL_OS_WIN32

    /* Setup Win32 surface descriptor */
    VkWin32SurfaceCreateInfoKHR surfaceDesc;
    {
        surfaceDesc.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceDesc.pNext       = nullptr;
        surfaceDesc.flags       = 0;
        surfaceDesc.hinstance   = GetModuleHandle(NULL);
        surfaceDesc.hwnd        = nativeHandle.window;
    }
    VkResult result = vkCreateWin32SurfaceKHR(instance_, &surfaceDesc, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Win32 surface for Vulkan render context");

    #endif
}

void VKRenderContext::CreateLogicalDevice()
{
    /* Initialize queue create description */
    queueFamilyIndices_ = FindQueueFamilies(physicalDevice_, surface_, VK_QUEUE_GRAPHICS_BIT);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices_.graphicsFamily, queueFamilyIndices_.presentFamily };

    float queuePriority = 1.0f;
    for (auto family : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo info;
        {
            info.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.pNext              = nullptr;
            info.flags              = 0;
            info.queueFamilyIndex   = family;
            info.queueCount         = 1;
            info.pQueuePriorities   = &queuePriority;
        }
        queueCreateInfos.push_back(info);
    }

    /* Initialize device features */
    VkPhysicalDeviceFeatures deviceFeatures;
    InitMemory(deviceFeatures);

    /* Initialize create descriptor for logical device */
    VkDeviceCreateInfo createInfo;

    createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.queueCreateInfoCount     = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos        = queueCreateInfos.data();
    createInfo.enabledLayerCount        = 0;
    createInfo.ppEnabledLayerNames      = nullptr;
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(g_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames  = g_deviceExtensions.data();
    createInfo.pEnabledFeatures         = &deviceFeatures;

    /* Create logical device */
    VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, device_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan logical device");

    /* Get device queues for graphics and presentation */
    vkGetDeviceQueue(device_, queueFamilyIndices_.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, queueFamilyIndices_.presentFamily, 0, &presentQueue_);
}

void VKRenderContext::CreateSwapChain(const VideoModeDescriptor& desc)
{
    /* Query swap-chain support for physics device and surface  */
    auto swapChainSupport = VKQuerySwapChainSupport(physicalDevice_, surface_);

    /* Pick surface format, present mode, and extent */
    auto surfaceFormat  = PickSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode    = PickSwapPresentMode(swapChainSupport.presentModes);
    auto extent         = PickSwapExtent(swapChainSupport.caps, static_cast<uint32_t>(desc.resolution.x), static_cast<uint32_t>(desc.resolution.y));

    /* Determine required image count for swap-chain */
    uint32_t imageCount = swapChainSupport.caps.minImageCount;
    if (swapChainSupport.caps.maxImageCount > 0)
        imageCount = std::min(imageCount, swapChainSupport.caps.maxImageCount);

    /* Find queue family indices */
    auto indices = FindQueueFamilies(physicalDevice_, surface_, VK_QUEUE_GRAPHICS_BIT);
    
    const uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };
    
    /* Initialize swap-chain descriptor */
    VkSwapchainCreateInfoKHR createInfo;

    createInfo.sType                        = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext                        = nullptr;
    createInfo.flags                        = 0;
    createInfo.surface                      = surface_;
    createInfo.minImageCount                = imageCount;
    createInfo.imageFormat                  = surfaceFormat.format;
    createInfo.imageColorSpace              = surfaceFormat.colorSpace;
    createInfo.imageExtent                  = extent;
    createInfo.imageArrayLayers             = 1;
    createInfo.imageUsage                   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount    = 2;
        createInfo.pQueueFamilyIndices      = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
    }

    createInfo.preTransform                 = swapChainSupport.caps.currentTransform;
    createInfo.compositeAlpha               = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode                  = presentMode;
    createInfo.clipped                      = VK_TRUE;
    createInfo.oldSwapchain                 = VK_NULL_HANDLE;

    /* Create swap-chain */
    VkResult result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, swapChain_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan swap-chain");

    /* Query swap-chain images */
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan swap-chain images");

    swapChainImages_.resize(imageCount);
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());
    VKThrowIfFailed(result, "failed to query Vulkan swap-chain images");

    /* Store swap-chain format and extent */
    swapChainFormat_ = surfaceFormat.format;
    swapChainExtent_ = extent;
}

void VKRenderContext::CreateSwapChainImageViews()
{
    swapChainImageViews_.resize(swapChainImages_.size(), VKPtr<VkImageView>{ device_, vkDestroyImageView });

    for (std::size_t i = 0, n = swapChainImages_.size(); i < n; ++i)
    {
        /* Initialize image view descriptor */
        VkImageViewCreateInfo createInfo;

        createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                            = nullptr;
        createInfo.flags                            = 0;
        createInfo.image                            = swapChainImages_[i];
        createInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                           = swapChainFormat_;
        createInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel    = 0;
        createInfo.subresourceRange.levelCount      = 1;
        createInfo.subresourceRange.baseArrayLayer  = 0;
        createInfo.subresourceRange.layerCount      = 1;

        /* Create image view */
        VkResult result = vkCreateImageView(device_, &createInfo, nullptr, swapChainImageViews_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain image view");
    }
}

void VKRenderContext::CreateSwapChainRenderPass()
{
    /* Initialize attachment descriptor */
    VkAttachmentDescription attachmentDesc;

    attachmentDesc.flags                = 0;
    attachmentDesc.format               = swapChainFormat_;
    attachmentDesc.samples              = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc.loadOp               = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc.storeOp              = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc.stencilLoadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDesc.stencilStoreOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDesc.initialLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc.finalLayout          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /* Initialize attachment reference */
    VkAttachmentReference attachmentRef;

    attachmentRef.attachment            = 0;
    attachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    /* Initialize sub-pass descriptor */
    VkSubpassDescription subpassDesc;
    InitMemory(subpassDesc);

    subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount    = 1;
    subpassDesc.pColorAttachments       = (&attachmentRef);

    /* Initialize sub-pass dependency */
    VkSubpassDependency subpassDep;

    subpassDep.srcSubpass               = VK_SUBPASS_EXTERNAL;
    subpassDep.dstSubpass               = 0;
    subpassDep.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.dstStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.srcAccessMask            = 0;
    subpassDep.dstAccessMask            = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDep.dependencyFlags          = 0;

    /* Initialize render pass descriptor */
    VkRenderPassCreateInfo createInfo;

    createInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.attachmentCount          = 1;
    createInfo.pAttachments             = (&attachmentDesc);
    createInfo.subpassCount             = 1;
    createInfo.pSubpasses               = (&subpassDesc);
    createInfo.dependencyCount          = 1;
    createInfo.pDependencies            = (&subpassDep);

    /* Create swap-chain render pass */
    VkResult result = vkCreateRenderPass(device_, &createInfo, nullptr, swapChainRenderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan swap-chain render pass");
}

void VKRenderContext::CreateSwapChainFramebuffers()
{
    swapChainFramebuffers_.resize(swapChainImageViews_.size(), VKPtr<VkFramebuffer>{ device_, vkDestroyFramebuffer });

    for (std::size_t i = 0, n = swapChainImageViews_.size(); i < n; ++i)
    {
        /* Initialize framebuffer descriptor */
        VkFramebufferCreateInfo createInfo;

        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = swapChainRenderPass_;
        createInfo.attachmentCount  = 1;
        createInfo.pAttachments     = &(swapChainImageViews_[i]);
        createInfo.width            = swapChainExtent_.width;
        createInfo.height           = swapChainExtent_.height;
        createInfo.layers           = 1;

        /* Create framebuffer */
        VkResult result = vkCreateFramebuffer(device_, &createInfo, nullptr, swapChainFramebuffers_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain framebuffer");
    }
}

void VKRenderContext::CreateVkSemaphore(VKPtr<VkSemaphore>& semaphore)
{
    /* Create semaphore (no flags) */
    VkSemaphoreCreateInfo createInfo;

    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VkResult result = vkCreateSemaphore(device_, &createInfo, nullptr, semaphore.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan semaphore");
}

void VKRenderContext::CreatePresentSemaphorse()
{
    /* Create presentation semaphorse */
    CreateVkSemaphore(imageAvailableSemaphore_);
    CreateVkSemaphore(renderFinishedSemaphore_);
}

std::vector<VkQueueFamilyProperties> VKRenderContext::QueryQueueFamilyProperties(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

QueueFamilyIndices VKRenderContext::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, const VkQueueFlags flags)
{
    QueueFamilyIndices indices;

    auto queueFamilies = QueryQueueFamilyProperties(device);

    uint32_t i = 0;
    for (const auto& family : queueFamilies)
    {
        /* Get graphics family index */
        if (family.queueCount > 0 && (family.queueFlags & flags) != 0)
            indices.graphicsFamily = i;

        /* Get present family index */
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (family.queueCount > 0 && presentSupport)
            indices.presentFamily = i;

        /* Check if queue family is complete */
        if (indices.Complete())
            break;

        ++i;
    }

    return indices;
}

VkSurfaceFormatKHR VKRenderContext::PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const
{
    if (surfaceFormats.empty())
        throw std::runtime_error("no Vulkan surface formats available");

    if (surfaceFormats.size() == 1 && surfaceFormats.front().format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const auto& format : surfaceFormats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return surfaceFormats.front();
}

VkPresentModeKHR VKRenderContext::PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
{
    for (const auto& mode : presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKRenderContext::PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, uint32_t width, uint32_t height) const
{
    if (surfaceCaps.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        return VkExtent2D
        {
            std::max(surfaceCaps.minImageExtent.width, std::min(surfaceCaps.maxImageExtent.width, width)),
            std::max(surfaceCaps.minImageExtent.height, std::min(surfaceCaps.maxImageExtent.height, height))
        };
    }
    return surfaceCaps.currentExtent;
}


} // /namespace LLGL



// ================================================================================
