/*
 * OpenXRSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_SWAP_CHAIN_H
#define LLGL_OPENXR_SWAP_CHAIN_H


#include <LLGL/XR/XRSwapChain.h>
#include <LLGL/Container/SmallVector.h>

#include "OpenXRPlatform.h"
#include "OpenXRGraphicsBinding.h"

#include <cstdint>
#include <memory>
#include <vector>


namespace LLGL
{


class RenderSystem;
class RenderTarget;
class RenderPass;
class Texture;


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
            std::vector<XRSwapchainImage>&& images
        );

        ~OpenXRSwapChain();

    public:

        Format              GetFormat() const override;
        Extent2D            GetResolution() const override;
        std::uint32_t       GetSampleCount() const override;
        std::uint32_t       GetArrayLayers() const override;
        RenderTarget*       AcquireRenderTarget() override;
        RenderTarget*       GetRenderTarget() const override;
        const RenderPass*   GetRenderPass() const override;

        bool                GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) override;

    public:

        //! Returns the underlying OpenXR swap-chain handle. Used by OpenXRSession when assembling composition layers.
        XrSwapchain GetSwapchain() const { return swapchain_; }

        //! Returns the index of the most recently acquired image, or UINT32_MAX if no image is currently acquired.
        std::uint32_t GetAcquiredIndex() const { return acquiredIndex_; }

        /**
        \brief Releases the currently acquired image (and its managed depth image, if any) back to the runtime.
        \remarks Called by OpenXRSession::EndFrame once per frame before submission. Flushes pending GPU work first (for
        backends without implicit submission). No-op if no image is currently acquired.
        */
        bool ReleaseImage();

        /**
        \brief Acquires and waits for the next image (and the depth companion's, in lockstep) so it is ready to render into.
        \remarks Called on demand by AcquireRenderTarget (inside the frame, after XRSession::BeginFrame). The acquire is
        kept within the begin/end-frame bracket because some OpenXR runtimes (e.g. VIVE WAVE) return
        XR_ERROR_CALL_ORDER_INVALID from xrAcquireSwapchainImage when it is called between xrEndFrame and the next
        xrBeginFrame. On failure the swap-chain holds no acquired image. Returns true if an image is ready.
        */
        bool AcquireNextImage();

        //! Returns the depth swap-chain submitted to the runtime for reprojection, or null if none. Used by OpenXRSession::EndFrame.
        OpenXRSwapChain* GetDepthCompanion() const { return depthCompanion_.get(); }

        /**
        \brief Hands this (color) swap-chain ownership of a depth swap-chain that the runtime composites for reprojection.
        \remarks Its image is acquired/waited/released in lockstep with this swap-chain's color image, and it is destroyed
        together with this swap-chain. Mutually exclusive with AttachDepthTexture.
        */
        void AttachDepthCompanion(std::unique_ptr<OpenXRSwapChain>&& depthSwapChain);

        /**
        \brief Hands this (color) swap-chain ownership of a private depth texture used when the runtime cannot composite depth.
        \remarks Reused for every image; not submitted to the runtime. Mutually exclusive with AttachDepthCompanion.
        */
        void AttachDepthTexture(Texture* depthTexture);

        /**
        \brief Builds one render target per (color image, depth image) combination from this swap-chain's images and depth.
        \remarks Must be called once after the depth resource (if any) has been attached. The render targets and the
        compatible render pass are owned by this swap-chain.
        \return True on success.
        */
        bool BuildRenderTargets();

    private:

        //! Acquires the next image (and the depth companion's image in lockstep); returns UINT32_MAX on failure. Used by GetRenderTarget.
        std::uint32_t   AcquireImage();

        //! Waits until the acquired image (and the depth companion's, if any) is safe to render into. Used by GetRenderTarget.
        bool            WaitImage(std::uint64_t timeoutNs);

        //! Raw OpenXR acquire/wait/release for this swap-chain's own image, without driving the depth companion or flushing GPU work.
        std::uint32_t   AcquireImageHandle();
        bool            WaitImageHandle(std::uint64_t timeoutNs);
        bool            ReleaseImageHandle();

    private:

        GraphicsBinding&                            binding_;
        RenderSystem&                               renderSystem_;
        XrSwapchain                                 swapchain_;
        XRSwapChainDescriptor                       desc_;
        std::int64_t                                nativeFormat_;
        std::vector<XRSwapchainImage>               images_;
        SmallVector<Texture*>                       texturePtrs_;
        std::uint32_t                               acquiredIndex_  = UINT32_MAX;

        // Depth resources owned by a color swap-chain (at most one is set). The companion is submitted to the
        // runtime; the texture is a private fallback when the runtime can't composite depth.
        std::unique_ptr<OpenXRSwapChain>            depthCompanion_;
        Texture*                                    depthTexture_   = nullptr;

        // Render targets indexed [colorImageIndex][depthImageIndex]; the depth axis has a single slot when depth is a
        // private texture or absent. Owned by this swap-chain.
        SmallVector<SmallVector<RenderTarget*>>     renderTargets_;

        // The single render pass shared by every render target above (and returned by GetRenderPass). Owned by the
        // first render target. Null until BuildRenderTargets runs. For a multiview swap-chain (arrayLayers > 1) this
        // is that target's default multiview render pass (built from RenderTargetDescriptor::views).
        const RenderPass*                           renderPass_     = nullptr;

};


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
