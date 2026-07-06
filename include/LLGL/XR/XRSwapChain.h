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
        \brief Acquires this frame's swap-chain image and returns the render target to render the current view into.
        \return Render target for the current frame's view, or null if no image could be acquired (in which case the
        view should be skipped for this frame).
        \remarks This has side effects: it acquires and waits on the runtime image (and its managed depth image, if
        any), so it must be called exactly once per frame per view, after XRSession::BeginFrame and before rendering.
        The image is released by the next XRSession::EndFrame. Acquire/wait/release are hidden inside LLGL, analogous
        to how LLGL::SwapChain hides image acquisition behind Present; the acquire happens here (inside the frame)
        rather than ahead of time because some OpenXR runtimes require it to stay within the begin/end-frame bracket.
        The render target combines the current color image with the swap-chain's managed depth buffer (present when
        the swap-chain was created with a depth-stencil format).
        \see GetRenderTarget
        \see GetRenderPass
        \see XRSession::EndFrame
        */
        virtual RenderTarget* AcquireRenderTarget() = 0;

        /**
        \brief Returns the render target for the image currently acquired by AcquireRenderTarget, or null if none.
        \remarks Pure accessor with no side effects. Unlike AcquireRenderTarget, this never acquires an image; it only
        reports the render target for the image acquired earlier this frame (or null if AcquireRenderTarget has not
        been called, or the image was already released by XRSession::EndFrame).
        \see AcquireRenderTarget
        */
        virtual RenderTarget* GetRenderTarget() const = 0;

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
