/*
 * OpenXRSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "OpenXRSwapChain.h"
#include "OpenXRGraphicsBinding.h"
#include "OpenXRError.h"

#include <LLGL/RenderSystem.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/Texture.h>
#include <LLGL/Log.h>
#include <LLGL/Backend/OpenXR/NativeHandle.h>

namespace LLGL
{

namespace OpenXR
{


OpenXRSwapChain::OpenXRSwapChain(
    GraphicsBinding&                binding,
    RenderSystem&                   renderSystem,
    XrSwapchain                     swapchain,
    const XRSwapChainDescriptor&    desc,
    std::int64_t                    nativeFormat,
    std::vector<XRSwapchainImage>&& images)
:
    binding_      { binding      },
    renderSystem_ { renderSystem },
    swapchain_    { swapchain    },
    desc_         { desc         },
    nativeFormat_ { nativeFormat },
    images_       { std::move(images) }
{
    texturePtrs_.reserve(images_.size());
    for (auto& img : images_)
        texturePtrs_.push_back(img.texture.get());
}

OpenXRSwapChain::~OpenXRSwapChain()
{
    // Render targets reference both the color and depth image views, so they must be destroyed before any of the
    // textures (our color images, the depth companion's images, or the private depth texture) are freed.
    for (auto& row : renderTargets_)
    {
        for (RenderTarget* renderTarget : row)
        {
            if (renderTarget != nullptr)
                renderSystem_.Release(*renderTarget);
        }
    }
    renderTargets_.clear();

    if (depthTexture_ != nullptr)
        renderSystem_.Release(*depthTexture_);

    // Destroy the depth companion (its images + XR swap-chain) before our own images.
    depthCompanion_.reset();

    // Destroy the textures before destroying the swap-chain in case underlying runtime requires it
    images_.clear();

    if (swapchain_ != XR_NULL_HANDLE)
        xrDestroySwapchain(swapchain_);
}

Format OpenXRSwapChain::GetFormat() const
{
    return desc_.format;
}

Extent2D OpenXRSwapChain::GetResolution() const
{
    return desc_.resolution;
}

std::uint32_t OpenXRSwapChain::GetSampleCount() const
{
    return desc_.sampleCount;
}

std::uint32_t OpenXRSwapChain::GetArrayLayers() const
{
    return desc_.arrayLayers;
}

RenderTarget* OpenXRSwapChain::AcquireRenderTarget()
{
    // Acquire + wait this frame's image on demand, i.e. inside the frame (this is called after xrBeginFrame, just
    // before rendering). The acquire/render/release cycle must stay within the runtime's frame bracket: some
    // runtimes (e.g. VIVE WAVE) return XR_ERROR_CALL_ORDER_INVALID from xrAcquireSwapchainImage if it is called
    // between xrEndFrame and the next xrBeginFrame (which an "acquire-ahead" scheme does). The image is released by
    // XRSession::EndFrame. Returns null if the image could not be acquired (the view should be skipped this frame).
    if (acquiredIndex_ == UINT32_MAX)
    {
        if (!AcquireNextImage())
            return nullptr;
    }
    return GetRenderTarget();
}

RenderTarget* OpenXRSwapChain::GetRenderTarget() const
{
    // Pure accessor (no side effects): maps the image acquired by AcquireRenderTarget this frame to its color/depth
    // render target. Returns null if no image is currently acquired (AcquireRenderTarget not yet called this frame,
    // or the image was already released by XRSession::EndFrame).
    if (acquiredIndex_ == UINT32_MAX || acquiredIndex_ >= renderTargets_.size())
        return nullptr;

    // The depth axis has a single slot unless a depth companion provides per-frame depth images.
    std::uint32_t depthIndex = 0;
    if (depthCompanion_ != nullptr)
    {
        depthIndex = depthCompanion_->GetAcquiredIndex();
        if (depthIndex == UINT32_MAX)
            return nullptr;
    }

    const SmallVector<RenderTarget*>& row = renderTargets_[acquiredIndex_];
    return (depthIndex < row.size() ? row[depthIndex] : nullptr);
}

const RenderPass* OpenXRSwapChain::GetRenderPass() const
{
    return renderPass_;
}

std::uint32_t OpenXRSwapChain::AcquireImageHandle()
{
    XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    std::uint32_t index = 0;
    const XrResult result = xrAcquireSwapchainImage(swapchain_, &acquireInfo, &index);
    if (Failed(result))
    {
        Log::Errorf("xrAcquireSwapchainImage failed: result=%d\n", static_cast<int>(result));
        acquiredIndex_ = UINT32_MAX;
        return UINT32_MAX;
    }
    acquiredIndex_ = index;
    return index;
}

bool OpenXRSwapChain::WaitImageHandle(std::uint64_t timeoutNs)
{
    XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    waitInfo.timeout = static_cast<XrDuration>(timeoutNs);
    const XrResult result = xrWaitSwapchainImage(swapchain_, &waitInfo);
    if (!XR_SUCCEEDED(result))
        Log::Errorf("xrWaitSwapchainImage failed: result=%d\n", static_cast<int>(result));
    return XR_SUCCEEDED(result);
}

bool OpenXRSwapChain::ReleaseImageHandle()
{
    XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    const XrResult result = xrReleaseSwapchainImage(swapchain_, &releaseInfo);
    acquiredIndex_ = UINT32_MAX;
    return XR_SUCCEEDED(result);
}

std::uint32_t OpenXRSwapChain::AcquireImage()
{
    const std::uint32_t index = AcquireImageHandle();
    if (index == UINT32_MAX)
        return UINT32_MAX;

    // Acquire the managed depth image in lockstep; on failure hand the color image back so the pair stays balanced.
    if (depthCompanion_ != nullptr && depthCompanion_->AcquireImageHandle() == UINT32_MAX)
    {
        ReleaseImageHandle();
        return UINT32_MAX;
    }

    return index;
}

bool OpenXRSwapChain::WaitImage(std::uint64_t timeoutNs)
{
    if (!WaitImageHandle(timeoutNs))
        return false;
    if (depthCompanion_ != nullptr && !depthCompanion_->WaitImageHandle(timeoutNs))
        return false;
    return true;
}

bool OpenXRSwapChain::ReleaseImage()
{
    // Nothing was acquired this frame (e.g. the view was skipped); keep acquire/release balanced.
    if (acquiredIndex_ == UINT32_MAX)
        return true;

    // Ensure all rendering commands targeting this swap-chain image have reached the GPU before
    // handing the image back to the runtime. Required for D3D11 (no implicit submission); a no-op
    // for D3D12 / Vulkan where the binding has nothing to do here. Flush once for the color/depth pair.
    binding_.FlushPendingGpuWork(renderSystem_);

    bool result = ReleaseImageHandle();
    if (depthCompanion_ != nullptr)
        result = depthCompanion_->ReleaseImageHandle() && result;
    return result;
}

bool OpenXRSwapChain::AcquireNextImage()
{
    if (AcquireImage() == UINT32_MAX)
        return false;
    if (!WaitImage(UINT64_MAX))
    {
        // Hand the acquired image(s) back so the ring stays balanced; GetRenderTarget will report "not ready".
        ReleaseImage();
        return false;
    }
    return true;
}

void OpenXRSwapChain::AttachDepthCompanion(std::unique_ptr<OpenXRSwapChain>&& depthSwapChain)
{
    depthCompanion_ = std::move(depthSwapChain);
}

void OpenXRSwapChain::AttachDepthTexture(Texture* depthTexture)
{
    depthTexture_ = depthTexture;
}

bool OpenXRSwapChain::BuildRenderTargets()
{
    // Collect the depth attachments: one per companion image (per-frame depth submission), or a single private
    // texture, or none. The depth axis of renderTargets_ has one slot per entry, or a single null-depth slot.
    SmallVector<Texture*> depthTextures;
    if (depthCompanion_ != nullptr)
        depthTextures = depthCompanion_->texturePtrs_;
    else if (depthTexture_ != nullptr)
        depthTextures.push_back(depthTexture_);

    const bool hasDepth = !depthTextures.empty();
    const std::size_t depthSlots = (hasDepth ? depthTextures.size() : 1u);

    // For a multiview swap-chain (one array layer per view), each render target renders to all array layers at
    // once via a multiview render pass, so a single draw is broadcast to every view (single-pass stereo). The view
    // count is passed through RenderTargetDescriptor::views below; the backend then creates a multiview render pass
    // whose per-attachment final layouts are derived from the attachment textures (color images end in
    // COLOR_ATTACHMENT_OPTIMAL, depth in DEPTH_STENCIL_ATTACHMENT_OPTIMAL), exactly like the single-view path.
    const std::uint32_t numViews = (desc_.arrayLayers > 1 ? desc_.arrayLayers : 1);

    renderTargets_.resize(texturePtrs_.size());
    for (std::size_t colorIndex = 0; colorIndex < texturePtrs_.size(); ++colorIndex)
    {
        renderTargets_[colorIndex].resize(depthSlots, nullptr);
        for (std::size_t depthIndex = 0; depthIndex < depthSlots; ++depthIndex)
        {
            RenderTargetDescriptor renderTargetDesc;
            renderTargetDesc.resolution          = desc_.resolution;
            renderTargetDesc.views               = numViews;
            renderTargetDesc.colorAttachments[0] = AttachmentDescriptor{ texturePtrs_[colorIndex] };
            if (hasDepth)
                renderTargetDesc.depthStencilAttachment = AttachmentDescriptor{ depthTextures[depthIndex] };

            // Every render target shares one render pass object (created by the first target): all targets have
            // identical attachment formats, so reusing it avoids redundant render passes and guarantees GetRenderPass
            // returns the exact pass these targets use, not merely a compatible one. For multiview, the first target
            // builds its default multiview render pass from renderTargetDesc.views above; the rest reuse it (and the
            // view count is then read from that render pass, so renderTargetDesc.views is ignored for them).
            renderTargetDesc.renderPass = renderPass_;

            RenderTarget* renderTarget = renderSystem_.CreateRenderTarget(renderTargetDesc);
            if (renderTarget == nullptr)
                return false;
            renderTargets_[colorIndex][depthIndex] = renderTarget;

            if (renderPass_ == nullptr)
                renderPass_ = renderTarget->GetRenderPass();
        }
    }

    // No acquire-ahead: each frame's image is acquired on demand by AcquireRenderTarget (inside the frame) and
    // released by XRSession::EndFrame, keeping acquire/release within the runtime's begin/end-frame bracket.

    return true;
}

bool OpenXRSwapChain::GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle == nullptr || nativeHandleSize != sizeof(SwapChainNativeHandle))
        return false;

    auto* handle = reinterpret_cast<SwapChainNativeHandle*>(nativeHandle);
    handle->swapchain = swapchain_;
    return true;
}

} // /namespace OpenXR

} // /namespace LLGL



// ================================================================================
