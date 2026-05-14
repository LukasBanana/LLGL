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
#include <LLGL/Texture.h>


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
    SmallVector<SwapchainImage>&&   images)
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
        texturePtrs_.push_back(img.texture);
}

OpenXRSwapChain::~OpenXRSwapChain()
{
    for (auto& img : images_)
    {
        if (img.texture != nullptr)
            binding_.ReleaseSwapchainImage(renderSystem_, *img.texture);
    }
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

ArrayView<Texture*> OpenXRSwapChain::GetImages() const
{
    return ArrayView<Texture*>{ texturePtrs_.data(), texturePtrs_.size() };
}

std::uint32_t OpenXRSwapChain::AcquireImage()
{
    XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    std::uint32_t index = 0;
    const XrResult result = xrAcquireSwapchainImage(swapchain_, &acquireInfo, &index);
    if (Failed(result))
    {
        acquiredIndex_ = UINT32_MAX;
        return UINT32_MAX;
    }
    acquiredIndex_ = index;
    return index;
}

bool OpenXRSwapChain::WaitImage(std::uint64_t timeoutNs)
{
    XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    waitInfo.timeout = static_cast<XrDuration>(timeoutNs);
    const XrResult result = xrWaitSwapchainImage(swapchain_, &waitInfo);
    return XR_SUCCEEDED(result);
}

bool OpenXRSwapChain::ReleaseImage()
{
    XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    const XrResult result = xrReleaseSwapchainImage(swapchain_, &releaseInfo);
    acquiredIndex_ = UINT32_MAX;
    return XR_SUCCEEDED(result);
}

void OpenXRSwapChain::SetDepthCompanion(XRSwapChain* depthSwapChain)
{
    depthCompanion_ = depthSwapChain;
}

XRSwapChain* OpenXRSwapChain::GetDepthCompanion() const
{
    return depthCompanion_;
}


} // /namespace OpenXR

} // /namespace LLGL



// ================================================================================
