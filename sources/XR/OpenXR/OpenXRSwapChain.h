/*
 * OpenXRSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_SWAP_CHAIN_H
#define LLGL_OPENXR_SWAP_CHAIN_H


#include <LLGL/XRSwapChain.h>
#include <LLGL/Container/SmallVector.h>

#include "OpenXRPlatform.h"
#include "OpenXRGraphicsBinding.h"

#include <cstdint>
#include <vector>


namespace LLGL
{


class RenderSystem;


namespace OpenXR
{

class OpenXRSwapChain final : public XRSwapChain
{

    public:

        OpenXRSwapChain(
            GraphicsBinding&                binding,
            RenderSystem&                   renderSystem,
            XrSwapchain                     swapchain,
            const XRSwapChainDescriptor&    desc,
            std::int64_t                    nativeFormat,
            SmallVector<SwapchainImage>&&   images
        );

        ~OpenXRSwapChain();

    public:

        Format              GetFormat() const override;
        Extent2D            GetResolution() const override;
        std::uint32_t       GetSampleCount() const override;
        std::uint32_t       GetArrayLayers() const override;
        ArrayView<Texture*> GetImages() const override;

        std::uint32_t       AcquireImage() override;
        bool                WaitImage(std::uint64_t timeoutNs) override;
        bool                ReleaseImage() override;

        void                SetDepthCompanion(XRSwapChain* depthSwapChain) override;
        XRSwapChain*        GetDepthCompanion() const override;

    public:

        //! Returns the underlying OpenXR swap-chain handle. Used by OpenXRSession when assembling composition layers.
        XrSwapchain GetSwapchain() const { return swapchain_; }

        //! Returns the index of the most recently acquired image, or UINT32_MAX if no image is currently acquired.
        std::uint32_t GetAcquiredIndex() const { return acquiredIndex_; }

    private:

        GraphicsBinding&                binding_;
        RenderSystem&                   renderSystem_;
        XrSwapchain                     swapchain_;
        XRSwapChainDescriptor           desc_;
        std::int64_t                    nativeFormat_;
        SmallVector<SwapchainImage>     images_;
        SmallVector<Texture*>           texturePtrs_;
        std::uint32_t                   acquiredIndex_  = UINT32_MAX;
        XRSwapChain*                    depthCompanion_ = nullptr;

};


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
