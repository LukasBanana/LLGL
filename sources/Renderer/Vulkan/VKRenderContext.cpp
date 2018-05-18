/*
 * VKRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderContext.h"
#include "VKCommandBuffer.h"
#include "VKCore.h"
#include "Memory/VKDeviceMemoryManager.h"
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
    const VKPtr<VkInstance>& instance,
    VkPhysicalDevice physicalDevice,
    const VKPtr<VkDevice>& device,
    VKDeviceMemoryManager& deviceMemoryMngr,
    RenderContextDescriptor desc,
    const std::shared_ptr<Surface>& surface) :
        instance_            { instance                      },
        physicalDevice_      { physicalDevice                },
        device_              { device                        },
        deviceMemoryMngr_    { deviceMemoryMngr              },
        surface_             { instance, vkDestroySurfaceKHR },
        swapChain_           { device, vkDestroySwapchainKHR },
        swapChainRenderPass_ { device, vkDestroyRenderPass   },
        vsyncDesc_           { desc.vsync                    }
{
    SetOrCreateSurface(surface, desc.videoMode, nullptr);

    CreatePresentSemaphores();
    CreateGpuSurface();

    if (desc.videoMode.depthBits > 0 || desc.videoMode.stencilBits > 0)
        CreateDepthStencilBuffer(desc.videoMode);

    CreateSwapChainRenderPass();
    CreateSwapChain(desc.videoMode, desc.vsync);
}

VKRenderContext::~VKRenderContext()
{
    ReleaseDepthStencilBuffer();
}

void VKRenderContext::Present()
{
    if (!commandBuffer_)
        throw std::runtime_error("no command buffer set to present render context");

    /* End command buffer and render pass */
    commandBuffer_->SetRenderPassNull();
    commandBuffer_->EndCommandBuffer();

    /* Initialize semaphorse */
    VkSemaphore waitSemaphorse[] = { imageAvailableSemaphore_ };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphorse[] = { renderFinishedSemaphore_ };
    VkCommandBuffer commandBuffers[] = { commandBuffer_->GetVkCommandBuffer() };

    /* Submit command buffer to graphics queue */
    VkSubmitInfo submitInfo;
    {
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphorse;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = commandBuffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphorse;
    }
    auto result = vkQueueSubmit(graphicsQueue_, 1, &submitInfo, commandBuffer_->GetQueueSubmitFence());
    VKThrowIfFailed(result, "failed to submit Vulkan graphics queue");

    /* Present result on screen */
    VkSwapchainKHR swapChains[] = { swapChain_ };

    VkPresentInfoKHR presentInfo;
    {
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext               = nullptr;
        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = signalSemaphorse;
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = swapChains;
        presentInfo.pImageIndices       = &presentImageIndex_;
        presentInfo.pResults            = nullptr;
    }
    result = vkQueuePresentKHR(presentQueue_, &presentInfo);
    VKThrowIfFailed(result, "failed to present Vulkan graphics queue");

    /* Get image index for next presentation */
    AcquireNextPresentImage();
}

/* --- Extended functions --- */

void VKRenderContext::SetPresentCommandBuffer(VKCommandBuffer* commandBuffer)
{
    commandBuffer_ = commandBuffer;
    commandBuffer_->SetPresentIndex(presentImageIndex_);
}

bool VKRenderContext::HasDepthStencilBuffer() const
{
    return (depthStencilFormat_ != VK_FORMAT_UNDEFINED);
}


/*
 * ======= Private: =======
 */

bool VKRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    /* Wait until graphics queue is idle before resources are destroyed and recreated */
    vkQueueWaitIdle(graphicsQueue_);

    /* Recreate presenting semaphores and Vulkan surface */
    CreatePresentSemaphores();
    CreateGpuSurface();

    /* Recreate (or just release) depth-stencil buffer */
    ReleaseDepthStencilBuffer();
    if (videoModeDesc.depthBits > 0 || videoModeDesc.stencilBits > 0)
        CreateDepthStencilBuffer(videoModeDesc);

    /* Recreate only swap-chain but keep render pass (independent of swap-chain object) */
    CreateSwapChain(videoModeDesc, vsyncDesc_);

    return true;
}

bool VKRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    /* Recreate swap-chain with new vsnyc settings */
    CreateSwapChain(GetVideoMode(), vsyncDesc);
    vsyncDesc_ = vsyncDesc;
    return true;
}

void VKRenderContext::CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore)
{
    /* Create semaphore (no flags) */
    VkSemaphoreCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
    }
    auto result = vkCreateSemaphore(device_, &createInfo, nullptr, semaphore.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan semaphore");
}

void VKRenderContext::CreatePresentSemaphores()
{
    /* Create presentation semaphorse */
    CreateGpuSemaphore(imageAvailableSemaphore_);
    CreateGpuSemaphore(renderFinishedSemaphore_);
}

void VKRenderContext::CreateGpuSurface()
{
    /* All previous swap-chains must be destroyed before VkSurfaceKHR can be destroyed */
    swapChain_.Release();

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
    auto result = vkCreateWin32SurfaceKHR(instance_, &surfaceDesc, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Win32 surface for Vulkan render context");

    #endif

    /* Query surface support details and pick surface format */
    surfaceSupportDetails_ = VKQuerySurfaceSupport(physicalDevice_, surface_);
    swapChainFormat_ = PickSwapSurfaceFormat(surfaceSupportDetails_.formats);
}

void VKRenderContext::CreateSwapChainRenderPass()
{
    VkAttachmentDescription attachments[2];

    /* Initialize color attachment */
    attachments[0].flags                = 0;
    attachments[0].format               = swapChainFormat_.format;
    attachments[0].samples              = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp              = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (HasDepthStencilBuffer())
    {
        /* Initialize depth-stencil attachment */
        attachments[1].flags                = 0;
        attachments[1].format               = depthStencilFormat_;
        attachments[1].samples              = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp              = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout          = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    /* Initialize color attachment reference */
    VkAttachmentReference colorAttachmentRef;
    {
        colorAttachmentRef.attachment       = 0;
        colorAttachmentRef.layout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    /* Initialize depth-stencil attachment reference */
    VkAttachmentReference depthAttachmentRef;
    {
        depthAttachmentRef.attachment       = 1;
        depthAttachmentRef.layout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    /* Initialize sub-pass descriptor */
    VkSubpassDescription subpassDesc = {};
    {
        subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount    = 1;
        subpassDesc.pColorAttachments       = (&colorAttachmentRef);
        if (HasDepthStencilBuffer())
            subpassDesc.pDepthStencilAttachment = (&depthAttachmentRef);
    }

    /* Initialize sub-pass dependency */
    VkSubpassDependency subpassDep;
    {
        subpassDep.srcSubpass               = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass               = 0;
        subpassDep.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.dstStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.srcAccessMask            = 0;
        subpassDep.dstAccessMask            = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDep.dependencyFlags          = 0;
    }

    /* Create swap-chain render pass */
    VkRenderPassCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.attachmentCount          = (HasDepthStencilBuffer() ? 2 : 1);
        createInfo.pAttachments             = attachments;
        createInfo.subpassCount             = 1;
        createInfo.pSubpasses               = (&subpassDesc);
        createInfo.dependencyCount          = 1;
        createInfo.pDependencies            = (&subpassDep);
    }
    auto result = vkCreateRenderPass(device_, &createInfo, nullptr, swapChainRenderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan swap-chain render pass");
}

void VKRenderContext::CreateSwapChain(const VideoModeDescriptor& videoModeDesc, const VsyncDescriptor& vsyncDesc)
{
    /* Pick swap-chain extent by resolution */
    swapChainExtent_ = PickSwapExtent(
        surfaceSupportDetails_.caps,
        videoModeDesc.resolution.width,
        videoModeDesc.resolution.height
    );

    /* Determine required image count for swap-chain */
    auto imageCount = surfaceSupportDetails_.caps.minImageCount;
    if (surfaceSupportDetails_.caps.maxImageCount > 0)
        imageCount = std::max(imageCount, std::min(videoModeDesc.swapChainSize, surfaceSupportDetails_.caps.maxImageCount));

    /* Get device queues for graphics and presentation */
    VkSurfaceKHR surface = surface_.Get();
    auto queueFamilyIndices = VKFindQueueFamilies(physicalDevice_, VK_QUEUE_GRAPHICS_BIT, &surface);

    vkGetDeviceQueue(device_, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, queueFamilyIndices.presentFamily, 0, &presentQueue_);

    /* Pick swap-chain presentation mode (with v-sync parameters) */
    auto presentMode = PickSwapPresentMode(surfaceSupportDetails_.presentModes, vsyncDesc);

    /* Create swap-chain */
    VkSwapchainCreateInfoKHR createInfo;
    {
        createInfo.sType                        = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                        = nullptr;
        createInfo.flags                        = 0;
        createInfo.surface                      = surface_;
        createInfo.minImageCount                = imageCount;
        createInfo.imageFormat                  = swapChainFormat_.format;
        createInfo.imageColorSpace              = swapChainFormat_.colorSpace;
        createInfo.imageExtent                  = swapChainExtent_;
        createInfo.imageArrayLayers             = 1;
        createInfo.imageUsage                   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
        {
            createInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount    = 2;
            createInfo.pQueueFamilyIndices      = queueFamilyIndices.indices;
        }
        else
        {
            createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount    = 0;
            createInfo.pQueueFamilyIndices      = nullptr;
        }

        createInfo.preTransform                 = surfaceSupportDetails_.caps.currentTransform;
        createInfo.compositeAlpha               = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode                  = presentMode;
        createInfo.clipped                      = VK_TRUE;
        createInfo.oldSwapchain                 = VK_NULL_HANDLE;
    }
    auto result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, swapChain_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan swap-chain");

    /* Query swap-chain images */
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan swap-chain images");

    swapChainImages_.resize(imageCount);
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());
    VKThrowIfFailed(result, "failed to query Vulkan swap-chain images");

    /* Create all swap-chain dependent resources */
    CreateSwapChainImageViews();
    CreateSwapChainFramebuffers();

    /* Acquire first image for presentation */
    AcquireNextPresentImage();
}

void VKRenderContext::CreateSwapChainImageViews()
{
    swapChainImageViews_.resize(swapChainImages_.size(), VKPtr<VkImageView> { device_, vkDestroyImageView });

    for (std::size_t i = 0, n = swapChainImages_.size(); i < n; ++i)
    {
        /* Create image view */
        VkImageViewCreateInfo createInfo;
        {
            createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.pNext                            = nullptr;
            createInfo.flags                            = 0;
            createInfo.image                            = swapChainImages_[i];
            createInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format                           = swapChainFormat_.format;
            createInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel    = 0;
            createInfo.subresourceRange.levelCount      = 1;
            createInfo.subresourceRange.baseArrayLayer  = 0;
            createInfo.subresourceRange.layerCount      = 1;
        }
        auto result = vkCreateImageView(device_, &createInfo, nullptr, swapChainImageViews_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain image view");
    }
}

void VKRenderContext::CreateSwapChainFramebuffers()
{
    swapChainFramebuffers_.resize(swapChainImageViews_.size(), VKPtr<VkFramebuffer>{ device_, vkDestroyFramebuffer });

    /* Initialize image view attachments */
    VkImageView attachments[2];

    if (HasDepthStencilBuffer())
        attachments[1] = depthImageView_.Get();

    /* Initialize framebuffer descriptor */
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = swapChainRenderPass_;
        createInfo.attachmentCount  = (HasDepthStencilBuffer() ? 2 : 1);
        createInfo.pAttachments     = attachments;
        createInfo.width            = swapChainExtent_.width;
        createInfo.height           = swapChainExtent_.height;
        createInfo.layers           = 1;
    }

    for (std::size_t i = 0, n = swapChainImageViews_.size(); i < n; ++i)
    {
        /* Update color attachment image view */
        attachments[0] = swapChainImageViews_[i].Get();

        /* Create framebuffer */
        auto result = vkCreateFramebuffer(device_, &createInfo, nullptr, swapChainFramebuffers_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain framebuffer");
    }
}

void VKRenderContext::CreateDepthStencilBuffer(const VideoModeDescriptor& videoModeDesc)
{
    CreateDepthStencilImage(videoModeDesc);
    CreateDepthStencilMemory();
    CreateDepthStencilImageView(videoModeDesc);
}

void VKRenderContext::CreateDepthStencilImage(const VideoModeDescriptor& videoModeDesc)
{
    /* Pick depth-stencil format */
    depthStencilFormat_ = (videoModeDesc.stencilBits > 0 ? PickDepthStencilFormat() : PickDepthFormat());

    /* Create image object */
    VkImageCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.imageType                = VK_IMAGE_TYPE_2D;
        createInfo.format                   = depthStencilFormat_;
        createInfo.extent.width             = videoModeDesc.resolution.width;
        createInfo.extent.height            = videoModeDesc.resolution.height;
        createInfo.extent.depth             = 1;
        createInfo.mipLevels                = 1;
        createInfo.arrayLayers              = 1;
        createInfo.samples                  = VK_SAMPLE_COUNT_1_BIT; //TODO: multi-sampling
        createInfo.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage                    = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE; // only used by graphics queue
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    VkResult result = vkCreateImage(device_, &createInfo, nullptr, depthImage_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan image for depth-stencil buffer");
}

void VKRenderContext::CreateDepthStencilImageView(const VideoModeDescriptor& videoModeDesc)
{
    /* Determine image aspect mask */
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (videoModeDesc.stencilBits > 0)
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    /* Create image view object */
    VkImageViewCreateInfo createInfo;
    {
        createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                            = nullptr;
        createInfo.flags                            = 0;
        createInfo.image                            = depthImage_;
        createInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                           = depthStencilFormat_;
        createInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask      = aspectMask;
        createInfo.subresourceRange.baseMipLevel    = 0;
        createInfo.subresourceRange.levelCount      = 1;
        createInfo.subresourceRange.baseArrayLayer  = 0;
        createInfo.subresourceRange.layerCount      = 1;
    }
    VkResult result = vkCreateImageView(device_, &createInfo, nullptr, depthImageView_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan image view for depth-stencil buffer");
}

void VKRenderContext::CreateDepthStencilMemory()
{
    /* Allocate device memory */
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device_, depthImage_, &requirements);

    depthImageMemoryRegion_ = deviceMemoryMngr_.Allocate(
        requirements.size,
        requirements.alignment,
        requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (depthImageMemoryRegion_)
        depthImageMemoryRegion_->BindImage(device_, depthImage_);
    else
        throw std::runtime_error("failed to allocate device memory for depth-stencil buffer");
}

void VKRenderContext::ReleaseDepthStencilBuffer()
{
    /* Release image and image view of depth-stencil buffer */
    depthImageView_.Release();
    depthImage_.Release();

    /* Release device memory region of depth-stencil buffer */
    deviceMemoryMngr_.Release(depthImageMemoryRegion_);
    depthImageMemoryRegion_ = nullptr;

    /* Reset depth-stencil format */
    depthStencilFormat_ = VK_FORMAT_UNDEFINED;
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

VkPresentModeKHR VKRenderContext::PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, const VsyncDescriptor& vsyncDesc) const
{
    if (!vsyncDesc.enabled)
    {
        /* Check if MAILBOX or IMMEDIATE presentation mode is available, to avoid vertical synchronization */
        for (const auto& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR || mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKRenderContext::PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, std::uint32_t width, std::uint32_t height) const
{
    if (surfaceCaps.currentExtent.width == std::numeric_limits<std::uint32_t>::max())
    {
        return VkExtent2D
        {
            std::max(surfaceCaps.minImageExtent.width, std::min(surfaceCaps.maxImageExtent.width, width)),
            std::max(surfaceCaps.minImageExtent.height, std::min(surfaceCaps.maxImageExtent.height, height))
        };
    }
    return surfaceCaps.currentExtent;
}

VkFormat VKRenderContext::PickDepthStencilFormat() const
{
    return VKFindSupportedImageFormat(
        physicalDevice_,
        { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat VKRenderContext::PickDepthFormat() const
{
    return VKFindSupportedImageFormat(
        physicalDevice_,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM, VK_FORMAT_D16_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void VKRenderContext::AcquireNextPresentImage()
{
    /* Get next image for presentation */
    vkAcquireNextImageKHR(
        device_,
        swapChain_,
        UINT64_MAX,
        imageAvailableSemaphore_,
        VK_NULL_HANDLE,
        &presentImageIndex_
    );
}


} // /namespace LLGL



// ================================================================================
