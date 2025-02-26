/*
 * VKSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKSwapChain.h"
#include "VKCore.h"
#include "VKTypes.h"
#include "Command/VKCommandContext.h"
#include "Memory/VKDeviceMemoryManager.h"
#include "Texture/VKImageUtils.h"
#include "../TextureUtils.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Exception.h"
#include "Platform/Apple/CAMetalLayerBridge.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Utils/ForRange.h>
#include <limits.h>
#include <set>


namespace LLGL
{


/* ----- Common ----- */

constexpr std::uint32_t VKSwapChain::maxNumFramesInFlight;

static VKPtr<VkImageView> NullVkImageView(VkDevice device)
{
    return VKPtr<VkImageView>{ device, vkDestroyImageView };
}

static VKPtr<VkFramebuffer> NullVkFramebuffer(VkDevice device)
{
    return VKPtr<VkFramebuffer>{ device, vkDestroyFramebuffer };
}

static VKPtr<VkSemaphore> NullVkSemaphore(VkDevice device)
{
    return VKPtr<VkSemaphore>{ device, vkDestroySemaphore };
}

static VKPtr<VkFence> NullVkFence(VkDevice device)
{
    return VKPtr<VkFence>{ device, vkDestroyFence };
}

VKSwapChain::VKSwapChain(
    VkInstance                      instance,
    VkPhysicalDevice                physicalDevice,
    VkDevice                        device,
    VKDeviceMemoryManager&          deviceMemoryMngr,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface,
    const RendererInfo&             rendererInfo)
:
    SwapChain                { desc                            },
    instance_                { instance                        },
    physicalDevice_          { physicalDevice                  },
    device_                  { device                          },
    deviceMemoryMngr_        { deviceMemoryMngr                },
    surface_                 { instance, vkDestroySurfaceKHR   },
    swapChain_               { device, vkDestroySwapchainKHR   },
    swapChainRenderPass_     { device                          },
    swapChainSamples_        { GetClampedSamples(desc.samples) },
    secondaryRenderPass_     { device                          },
    depthStencilBuffer_      { device                          },
    imageAvailableSemaphore_ { NullVkSemaphore(device_),
                               NullVkSemaphore(device_),
                               NullVkSemaphore(device_)        },
    renderFinishedSemaphore_ { NullVkSemaphore(device_),
                               NullVkSemaphore(device_),
                               NullVkSemaphore(device_)        },
    inFlightFences_          { NullVkFence(device_),
                               NullVkFence(device_),
                               NullVkFence(device_)            }
{
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(rendererInfo), desc);

    CreatePresentSemaphoresAndFences();
    CreateGpuSurface();

    /* Pick image count for swap-chain and depth-stencil format */
    numPreferredColorBuffers_   = PickSwapChainSize(desc.swapBuffers);
    depthStencilFormat_         = PickDepthStencilFormat(desc.depthBits, desc.stencilBits);

    /* Create Vulkan render passes, swap-chain, depth-stencil buffer, and multisampling color buffers */
    CreateDefaultAndSecondaryRenderPass();
    CreateResolutionDependentResources(GetResolution());

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

bool VKSwapChain::IsPresentable() const
{
    /* Use implicit boolean conversion here, since this type can either be a pointer or an integer type depending on the platform */
    return !!surface_.Get();
}

void VKSwapChain::Present()
{
    /* Initialize semaphores */
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore_[currentFrameInFlight_] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore_[currentFrameInFlight_] };

    /* Submit signal semaphore to graphics queue */
    VkSubmitInfo submitInfo;
    {
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 0;
        submitInfo.pCommandBuffers      = nullptr;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;
    }
    VkResult result = vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrameInFlight_]);
    VKThrowIfFailed(result, "failed to submit semaphore to Vulkan graphics queue");

    /* Present result on screen */
    VkPresentInfoKHR presentInfo;
    {
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext               = nullptr;
        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = signalSemaphores;
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = swapChain_.GetAddressOf();
        presentInfo.pImageIndices       = &currentColorBuffer_;
        presentInfo.pResults            = nullptr;
    }
    result = vkQueuePresentKHR(presentQueue_, &presentInfo);
    VKThrowIfFailed(result, "failed to present Vulkan graphics queue");

    /* Move to next frame */
    AcquireNextColorBuffer();
}

std::uint32_t VKSwapChain::GetCurrentSwapIndex() const
{
    return currentColorBuffer_;
}

std::uint32_t VKSwapChain::GetNumSwapBuffers() const
{
    return numColorBuffers_;
}

std::uint32_t VKSwapChain::GetSamples() const
{
    return swapChainSamples_;
}

Format VKSwapChain::GetColorFormat() const
{
    return VKTypes::Unmap(swapChainFormat_.format);
}

Format VKSwapChain::GetDepthStencilFormat() const
{
    return VKTypes::Unmap(depthStencilFormat_);
}

const RenderPass* VKSwapChain::GetRenderPass() const
{
    return (&swapChainRenderPass_);
}

bool VKSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    /* Recreate swap-chain with new vsnyc settings */
    if (vsyncInterval_ != vsyncInterval)
    {
        CreatePresentSemaphoresAndFences();
        CreateSwapChain(GetResolution(), vsyncInterval);
        CreateSwapChainFramebuffers();
        vsyncInterval_ = vsyncInterval;
    }
    return true;
}

/* --- Extended functions --- */

std::uint32_t VKSwapChain::TranslateSwapIndex(std::uint32_t swapBufferIndex) const
{
    if (swapBufferIndex == LLGL_CURRENT_SWAP_INDEX)
        return currentColorBuffer_;
    else
        return std::min(swapBufferIndex, numColorBuffers_ - 1);
}

bool VKSwapChain::HasDepthStencilBuffer() const
{
    return (depthStencilFormat_ != VK_FORMAT_UNDEFINED);
}

bool VKSwapChain::HasMultiSampling() const
{
    return (swapChainSamples_ > 1);
}

template <typename TDst>
void CopyVkImageRegion(TDst& outRegion, const TextureRegion& dstRegion, const Offset2D& srcOffset, VkImageAspectFlags aspectFlags)
{
    outRegion.srcSubresource.aspectMask     = aspectFlags;
    outRegion.srcSubresource.mipLevel       = 0;
    outRegion.srcSubresource.baseArrayLayer = 0;
    outRegion.srcSubresource.layerCount     = 1u;
    outRegion.srcOffset.x                   = srcOffset.x;
    outRegion.srcOffset.y                   = srcOffset.y;
    outRegion.srcOffset.z                   = 0;
    outRegion.dstSubresource.aspectMask     = aspectFlags;
    outRegion.dstSubresource.mipLevel       = dstRegion.subresource.baseMipLevel;
    outRegion.dstSubresource.baseArrayLayer = dstRegion.subresource.baseArrayLayer;
    outRegion.dstSubresource.layerCount     = 1u;
    outRegion.dstOffset.x                   = dstRegion.offset.x;
    outRegion.dstOffset.y                   = dstRegion.offset.y;
    outRegion.dstOffset.z                   = dstRegion.offset.z;
    outRegion.extent.width                  = dstRegion.extent.width;
    outRegion.extent.height                 = dstRegion.extent.height;
    outRegion.extent.depth                  = 1u;
}

void VKSwapChain::CopyImage(
    VKCommandContext&       context,
    VkImage                 dstImage,
    VkImageLayout           dstImageLayout,
    const TextureRegion&    dstRegion,
    std::uint32_t           srcColorBuffer,
    const Offset2D&         srcOffset,
    VkFormat                format)
{
    const bool                  isDepthStencil  = VKTypes::IsVkFormatDepthStencil(format);
    const VkImageAspectFlags    aspectFlags     = VKImageUtils::GetInclusiveVkImageAspect(format);

    if (HasMultiSampling())
    {
        VkImageResolve resolveRegion;
        CopyVkImageRegion(resolveRegion, dstRegion, srcOffset, aspectFlags);

        if (isDepthStencil)
        {
            if (depthStencilFormat_ == VK_FORMAT_UNDEFINED)
                return /*No depth-stencil buffer*/;

            VkImage srcImage = depthStencilBuffer_.GetVkImage();
            context.ResolveImage(srcImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, dstImage, dstImageLayout, resolveRegion, format);
        }
        else
        {
            LLGL_ASSERT(srcColorBuffer < colorBuffers_.size());
            VkImage srcImage = colorBuffers_[srcColorBuffer].GetVkImage();
            context.ResolveImage(srcImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, dstImage, dstImageLayout, resolveRegion, format);
        }
    }
    else
    {
        VkImageCopy copyRegion;
        CopyVkImageRegion(copyRegion, dstRegion, srcOffset, aspectFlags);

        if (isDepthStencil)
        {
            if (depthStencilFormat_ == VK_FORMAT_UNDEFINED)
                return /*No depth-stencil buffer*/;

            VkImage srcImage = depthStencilBuffer_.GetVkImage();
            context.CopyImage(srcImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, dstImage, dstImageLayout, copyRegion, format);
        }
        else
        {
            LLGL_ASSERT(srcColorBuffer < swapChainImages_.size());
            VkImage srcImage = swapChainImages_[srcColorBuffer];
            context.CopyImage(srcImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, dstImage, dstImageLayout, copyRegion, format);
        }
    }
}


/*
 * ======= Private: =======
 */

bool VKSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    /* Check if new resolution would actually change the swap-chain extent */
    if (swapChainExtent_.width  != resolution.width ||
        swapChainExtent_.height != resolution.height)
    {
        /* Wait until graphics queue is idle before resources are destroyed and recreated */
        vkQueueWaitIdle(graphicsQueue_);

        /* Recreate presenting semaphores and Vulkan surface */
        CreatePresentSemaphoresAndFences();
        CreateGpuSurface();

        /* Recreate color and depth-stencil buffers */
        ReleaseRenderBuffers();
        CreateResolutionDependentResources(resolution);
    }
    return true;
}

void VKSwapChain::CreateGpuSemaphore(VKPtr<VkSemaphore>& semaphore)
{
    /* Create semaphore (no flags) */
    VkSemaphoreCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
    }
    VkResult result = vkCreateSemaphore(device_, &createInfo, nullptr, semaphore.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan semaphore");
}

void VKSwapChain::CreateGpuFence(VKPtr<VkFence>& fence)
{
    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VkResult result = vkCreateFence(device_, &createInfo, nullptr, fence.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan fence");
}

void VKSwapChain::CreatePresentSemaphoresAndFences()
{
    /* Create presentation semaphorse */
    for_range(i, maxNumFramesInFlight)
    {
        CreateGpuSemaphore(imageAvailableSemaphore_[i]);
        CreateGpuSemaphore(renderFinishedSemaphore_[i]);
        CreateGpuFence(inFlightFences_[i]);
    }
}

void VKSwapChain::CreateGpuSurface()
{
    /* All previous swap-chains must be destroyed before VkSurfaceKHR can be destroyed */
    swapChain_.Release();

    /* Get hantive handle from context surface */
    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    #if defined LLGL_OS_WIN32

    /* Setup Win32 surface descriptor */
    VkWin32SurfaceCreateInfoKHR createInfo;
    {
        createInfo.sType        = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.hinstance    = GetModuleHandle(NULL);
        createInfo.hwnd         = nativeHandle.window;
    }
    VkResult result = vkCreateWin32SurfaceKHR(instance_, &createInfo, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Win32 surface for Vulkan swap-chain");

    #elif defined LLGL_OS_LINUX

    VkXlibSurfaceCreateInfoKHR createInfo;
    {
        createInfo.sType    = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.dpy      = nativeHandle.display;
        createInfo.window   = nativeHandle.window;
    }
    VkResult result = vkCreateXlibSurfaceKHR(instance_, &createInfo, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Xlib surface for Vulkan swap-chain");

    #elif defined LLGL_OS_ANDROID

    LLGL_ASSERT(nativeHandle.window != nullptr, "missing valid ANativeWindow object to create Vulkan surface on Android");
    VkAndroidSurfaceCreateInfoKHR createInfo;
    {
        createInfo.sType    = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.window   = nativeHandle.window;
    }
    VkResult result = vkCreateAndroidSurfaceKHR(instance_, &createInfo, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Android surface for Vulkan swap-chain");

    #elif defined LLGL_OS_MACOS || defined LLGL_OS_IOS

    VkMetalSurfaceCreateInfoEXT createInfo;
    {
        createInfo.sType    = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.pLayer   = CreateCAMetalLayerForSurfaceHandle(&nativeHandle, sizeof(nativeHandle));
    }
    VkResult result = vkCreateMetalSurfaceEXT(instance_, &createInfo, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Macos surface for Vulkan swap-chain");

    #else

    #error Platform not supported for Vulkan backend

    #endif

    /* Query surface support details and pick surface format */
    surfaceSupportDetails_  = VKQuerySurfaceSupport(physicalDevice_, surface_);
    swapChainFormat_        = PickSwapSurfaceFormat(surfaceSupportDetails_.formats);
}

void VKSwapChain::CreateRenderPass(VKRenderPass& renderPass, AttachmentLoadOp loadOp, AttachmentStoreOp storeOp)
{
    RenderPassDescriptor renderPassDesc;
    {
        /* Pass number of samples to render pass descriptor */
        renderPassDesc.samples = swapChainSamples_;

        /* Specify single color attachment */
        renderPassDesc.colorAttachments[0] = AttachmentFormatDescriptor{ GetColorFormat(), loadOp, storeOp };

        /* Specify depth-stencil attachment */
        const Format depthStencilFormat = GetDepthStencilFormat();

        if (IsDepthFormat(depthStencilFormat))
            renderPassDesc.depthAttachment = AttachmentFormatDescriptor{ depthStencilFormat, loadOp, storeOp };
        if (IsStencilFormat(depthStencilFormat))
            renderPassDesc.stencilAttachment = AttachmentFormatDescriptor{ depthStencilFormat, loadOp, storeOp };
    }
    renderPass.CreateVkRenderPass(device_, renderPassDesc);
}

void VKSwapChain::CreateDefaultAndSecondaryRenderPass()
{
    CreateRenderPass(swapChainRenderPass_, AttachmentLoadOp::Undefined, AttachmentStoreOp::Store);
    CreateRenderPass(secondaryRenderPass_, AttachmentLoadOp::Load, AttachmentStoreOp::Store);
}

void VKSwapChain::CreateSwapChain(const Extent2D& resolution, std::uint32_t vsyncInterval)
{
    /* Pick swap-chain extent by resolution */
    swapChainExtent_ = PickSwapExtent(surfaceSupportDetails_.caps, resolution);

    /* Get device queues for graphics and presentation */
    VkSurfaceKHR surface = surface_.Get();
    const VKQueueFamilyIndices queueFamilyIndices = VKFindQueueFamilies(physicalDevice_, VK_QUEUE_GRAPHICS_BIT, &surface);

    vkGetDeviceQueue(device_, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, queueFamilyIndices.presentFamily, 0, &presentQueue_);

    /* Pick swap-chain presentation mode (with v-sync parameters) */
    const VkPresentModeKHR presentMode = PickSwapPresentMode(surfaceSupportDetails_.presentModes, vsyncInterval);

    /* Create swap-chain */
    VkSwapchainCreateInfoKHR createInfo;
    {
        createInfo.sType                        = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext                        = nullptr;
        createInfo.flags                        = 0;
        createInfo.surface                      = surface_;
        createInfo.minImageCount                = numPreferredColorBuffers_;
        createInfo.imageFormat                  = swapChainFormat_.format;
        createInfo.imageColorSpace              = swapChainFormat_.colorSpace;
        createInfo.imageExtent                  = swapChainExtent_;
        createInfo.imageArrayLayers             = 1;
        //TODO: allow more fine grain control; VK_IMAGE_USAGE_TRANSFER_SRC_BIT is required for CopyTextureFromFramebuffer()
        createInfo.imageUsage                   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
        {
            createInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount    = queueFamilyIndices.Count();
            createInfo.pQueueFamilyIndices      = queueFamilyIndices.Ptr();
        }
        else
        {
            createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount    = 0;
            createInfo.pQueueFamilyIndices      = nullptr;
        }

        /* Prefer identity transformation */
        if ((surfaceSupportDetails_.caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0)
            createInfo.preTransform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            createInfo.preTransform             = surfaceSupportDetails_.caps.currentTransform;

        createInfo.compositeAlpha               = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode                  = presentMode;
        createInfo.clipped                      = VK_TRUE;
        createInfo.oldSwapchain                 = VK_NULL_HANDLE;
    }
    VkResult result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, swapChain_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan swap-chain");

    /* Query swap-chain images */
    numColorBuffers_ = numPreferredColorBuffers_;
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &numColorBuffers_, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan swap-chain images");

    swapChainImages_.resize(numColorBuffers_, VK_NULL_HANDLE);
    result = vkGetSwapchainImagesKHR(device_, swapChain_, &numColorBuffers_, swapChainImages_.data());
    VKThrowIfFailed(result, "failed to query Vulkan swap-chain images");

    /* Create swap-chain image views */
    CreateSwapChainImageViews();

    /* Get initial color buffer index for new Vulkan swap-chain */
    AcquireNextColorBuffer();
}

void VKSwapChain::CreateSwapChainImageViews()
{
    /* Initialize image-view descriptor */
    VkImageViewCreateInfo createInfo;
    {
        createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                            = nullptr;
        createInfo.flags                            = 0;
        createInfo.image                            = VK_NULL_HANDLE;
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

    /* Create all image views for the swap-chain */
    swapChainImageViews_.resize(numColorBuffers_);
    for_range(i, numColorBuffers_)
    {
        /* Update image handle in Vulkan descriptor */
        createInfo.image = swapChainImages_[i];

        /* Create image view for framebuffer */
        VKPtr<VkImageView> imageView{ NullVkImageView(device_) };
        VkResult result = vkCreateImageView(device_, &createInfo, nullptr, imageView.ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain image view");
        swapChainImageViews_[i] = std::move(imageView);
    }
}

void VKSwapChain::CreateSwapChainFramebuffers()
{
    /* Initialize image view attachments */
    VkImageView attachments[3] = {};
    std::uint32_t numAttachments = 0;

    const std::uint32_t attachmentColor = numAttachments++;

    if (HasDepthStencilBuffer())
    {
        const std::uint32_t attachmentDSV = numAttachments++;
        attachments[attachmentDSV] = depthStencilBuffer_.GetVkImageView();
    }

    const std::uint32_t attachmentResolve = (HasMultiSampling() ? numAttachments++ : 0);

    /* Initialize framebuffer descriptor */
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = swapChainRenderPass_.GetVkRenderPass();
        createInfo.attachmentCount  = numAttachments;
        createInfo.pAttachments     = attachments;
        createInfo.width            = swapChainExtent_.width;
        createInfo.height           = swapChainExtent_.height;
        createInfo.layers           = 1;
    }

    /* Create all framebuffers for the swap-chain */
    swapChainFramebuffers_.resize(numColorBuffers_);
    for_range(i, numColorBuffers_)
    {
        LLGL_ASSERT(swapChainImageViews_[i].Get() != VK_NULL_HANDLE, "failed to create Vulkan framebuffer for swap-buffer [%u]", i);

        /* Update image view in Vulkan descriptor */
        if (HasMultiSampling())
        {
            LLGL_ASSERT(colorBuffers_[i].GetVkImageView() != VK_NULL_HANDLE, "failed to create Vulkan framebuffer for swap-buffer [%u]", i);
            attachments[attachmentColor] = colorBuffers_[i].GetVkImageView();
            attachments[attachmentResolve] = swapChainImageViews_[i];
        }
        else
            attachments[attachmentColor] = swapChainImageViews_[i];

        /* Create framebuffer */
        VKPtr<VkFramebuffer> framebuffer{ NullVkFramebuffer(device_) };
        VkResult result = vkCreateFramebuffer(device_, &createInfo, nullptr, framebuffer.ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan swap-chain framebuffer");
        swapChainFramebuffers_[i] = std::move(framebuffer);
    }
}

void VKSwapChain::CreateDepthStencilBuffer(const Extent2D& resolution)
{
    const VkSampleCountFlagBits sampleCountBits = VKTypes::ToVkSampleCountBits(swapChainSamples_);
    depthStencilBuffer_.Create(deviceMemoryMngr_, resolution, depthStencilFormat_, sampleCountBits);
}

void VKSwapChain::CreateColorBuffers(const Extent2D& resolution)
{
    /* Create VkImage objects for each swap-chain buffer */
    const VkSampleCountFlagBits sampleCountBits = VKTypes::ToVkSampleCountBits(swapChainSamples_);
    colorBuffers_.resize(numColorBuffers_);
    for_range(i, numColorBuffers_)
    {
        VKColorBuffer colorBuffer{ device_ };
        colorBuffer.Create(deviceMemoryMngr_, resolution, swapChainFormat_.format, sampleCountBits);
        colorBuffers_[i] = std::move(colorBuffer);
    }
}

void VKSwapChain::ReleaseRenderBuffers()
{
    depthStencilBuffer_.Release();
    if (HasMultiSampling())
    {
        for_range(i, numColorBuffers_)
            colorBuffers_[i].Release();
    }
}

void VKSwapChain::CreateResolutionDependentResources(const Extent2D& resolution)
{
    CreateSwapChain(resolution, vsyncInterval_);

    if (HasMultiSampling())
        CreateColorBuffers(resolution);

    if (depthStencilFormat_ != VK_FORMAT_UNDEFINED)
        CreateDepthStencilBuffer(resolution);

    CreateSwapChainFramebuffers();
}

VkSurfaceFormatKHR VKSwapChain::PickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const
{
    if (surfaceFormats.empty())
        LLGL_TRAP("no Vulkan surface formats available");

    if (surfaceFormats.size() == 1 && surfaceFormats.front().format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const VkSurfaceFormatKHR& format : surfaceFormats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return surfaceFormats.front();
}

VkPresentModeKHR VKSwapChain::PickSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, std::uint32_t vsyncInterval) const
{
    if (vsyncInterval == 0)
    {
        /* Check if MAILBOX or IMMEDIATE presentation mode is available, to avoid vertical synchronization */
        for (const VkPresentModeKHR& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR || mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKSwapChain::PickSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, const Extent2D& resolution) const
{
    return VkExtent2D
    {
        std::max(surfaceCaps.minImageExtent.width,  std::min(surfaceCaps.maxImageExtent.width,  resolution.width )),
        std::max(surfaceCaps.minImageExtent.height, std::min(surfaceCaps.maxImageExtent.height, resolution.height))
    };
}

static std::vector<VkFormat> GetDepthStencilFormatPreference(int depthBits, int stencilBits)
{
    if (stencilBits == 0)
    {
        if (depthBits == 32)
            return { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    }
    else
    {
        if (depthBits == 32)
            return { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };
    }
    return { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM };
}

VkFormat VKSwapChain::PickDepthStencilFormat(int depthBits, int stencilBits) const
{
    const std::vector<VkFormat> formats = GetDepthStencilFormatPreference(depthBits, stencilBits);
    return VKFindSupportedImageFormat(
        physicalDevice_,
        formats.data(),
        formats.size(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

std::uint32_t VKSwapChain::PickSwapChainSize(std::uint32_t swapBuffers) const
{
    return Clamp(
        swapBuffers,
        surfaceSupportDetails_.caps.minImageCount,
        surfaceSupportDetails_.caps.maxImageCount
    );
}

void VKSwapChain::AcquireNextColorBuffer()
{
    currentFrameInFlight_ = (currentFrameInFlight_ + 1) % maxNumFramesInFlight;
    vkWaitForFences(device_, 1, inFlightFences_[currentFrameInFlight_].GetAddressOf(), VK_TRUE, UINT64_MAX);

    vkAcquireNextImageKHR(
        device_,
        swapChain_,
        UINT64_MAX,
        imageAvailableSemaphore_[currentFrameInFlight_],
        VK_NULL_HANDLE,
        &currentColorBuffer_
    );

    LLGL_ASSERT(
        currentColorBuffer_ < numColorBuffers_,
        "next swap-chain image index (%u) exceeds upper bound (%u)",
        currentColorBuffer_, numColorBuffers_
    );

    vkResetFences(device_, 1, inFlightFences_[currentFrameInFlight_].GetAddressOf());
}


} // /namespace LLGL



// ================================================================================
