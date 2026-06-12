/*
 * XRSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_SWAP_CHAIN_H
#define LLGL_XR_SWAP_CHAIN_H


#include <LLGL/Interface.h>
#include <LLGL/XR/XRSystemFlags.h>

#include <cstdint>


namespace LLGL
{


class RenderTarget;
class RenderPass;

/**
\brief XR swap-chain interface.
\remarks Unlike LLGL::SwapChain, an XR swap-chain does not present to a window surface; it is consumed by the
OpenXR runtime as a composition layer. The runtime owns the images and hands them back through the
acquire/wait/release lifecycle. The swap-chain wraps each runtime image in an internally-managed
LLGL::RenderTarget (combined with a managed depth buffer when one is requested), which the application renders
into with the normal CommandBuffer API; the raw image textures are not exposed.
\see XRSession::CreateSwapChain
*/
class LLGL_EXPORT XRSwapChain : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::XRSwapChain );

    public:

        //! Releases internal data. The owning XRSession releases native XR resources.
        ~XRSwapChain();

    public:

        /**
        \brief Returns the color format of this swap-chain.
        \remarks This may differ from the format requested in XRSwapChainDescriptor if the runtime substituted a compatible format.
        */
        virtual Format GetFormat() const = 0;

        //! Returns the pixel resolution of this swap-chain.
        virtual Extent2D GetResolution() const = 0;

        //! Returns the sample count of this swap-chain.
        virtual std::uint32_t GetSampleCount() const = 0;

        //! Returns the number of array layers of this swap-chain.
        virtual std::uint32_t GetArrayLayers() const = 0;

        /**
        \brief Returns the render target for this swap-chain's current-frame image.
        \return Render target to render the current frame's view into, or null if no image is ready (in which case the
        view should be skipped for this frame).
        \remarks This is a pure accessor with no side effects: the runtime image (and its managed depth image, if any)
        is acquired and waited on ahead of time by XRSession::EndFrame, and released by the next EndFrame, analogous to
        how LLGL::SwapChain hides image acquisition behind Present. The render target combines the current color image
        with the swap-chain's managed depth buffer (present when the swap-chain was created with a depth-stencil format).
        \see GetRenderPass
        \see XRSession::EndFrame
        */
        virtual RenderTarget* GetRenderTarget() = 0;

        /**
        \brief Returns a render pass compatible with this swap-chain's render targets.
        \remarks All of a swap-chain's render targets share the same color/depth attachment formats, so the returned
        render pass can be used to create an LLGL::PipelineState that renders into any image of this swap-chain (the
        Vulkan backend requires a render pass at pipeline creation time). Returns null before any render target exists.
        \see GetRenderTarget
        */
        virtual const RenderPass* GetRenderPass() const = 0;

    public:

        /**
        \brief Returns the native handle of this XR SwapChain.
        \param[out] nativeHandle Pointer to the backend-specific native handle structure.
        Obtain it from <code>#include <LLGL/Backend/OpenXR/NativeHandle.h></code>.
        \param[in] nativeHandleSize Size (in bytes) of the native handle structure.
        \return True if the native handle was successfully retrieved.
        \see OpenXR::SwapChainNativeHandle
        */
        virtual bool GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) = 0;

    protected:

        //! Allocates the internal data.
        XRSwapChain();

};


} // /namespace LLGL


#endif



// ================================================================================
