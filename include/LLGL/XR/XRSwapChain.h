/*
 * XRSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_SWAP_CHAIN_H
#define LLGL_XR_SWAP_CHAIN_H


#include <LLGL/Interface.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/XR/XRSystemFlags.h>

#include <cstdint>


namespace LLGL
{


class Texture;

/**
\brief XR swap-chain interface.
\remarks Unlike LLGL::SwapChain, an XR swap-chain does not present to a window surface; it is consumed by the
OpenXR runtime as a composition layer. The runtime owns the images and hands them back through the
acquire/wait/release lifecycle. Each image is exposed as an LLGL::Texture so application code can build
LLGL::RenderTarget objects from them and render with the normal CommandBuffer API.
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
        \brief Returns the full set of textures backing this swap-chain.
        \remarks The returned array is valid for the lifetime of the XRSwapChain. Use these textures to build
        LLGL::RenderTarget objects once at startup; per-frame, AcquireImage returns the index into this array
        that is safe to render into.
        \see AcquireImage
        */
        virtual ArrayView<Texture*> GetImages() const = 0;

    public:

        /**
        \brief Acquires the next swap-chain image for rendering.
        \return Index into the array returned by GetImages identifying the image the application may render into,
        or UINT32_MAX on failure (in which case the frame should be skipped).
        \remarks Must be followed by WaitImage before issuing render commands that target the image, and by
        ReleaseImage after all such commands have been submitted.
        \see WaitImage
        \see ReleaseImage
        */
        virtual std::uint32_t AcquireImage() = 0;

        /**
        \brief Waits until the acquired image is safe for the application to render into.
        \param[in] timeoutNs Wait timeout in nanoseconds. Pass UINT64_MAX for an infinite wait.
        \return True if the image is ready, false on timeout or failure.
        \see AcquireImage
        */
        virtual bool WaitImage(std::uint64_t timeoutNs = UINT64_MAX) = 0;

        /**
        \brief Releases the acquired image back to the runtime for composition.
        \remarks Must be called after the application has finished rendering and submitting commands that target the image.
        \see AcquireImage
        */
        virtual bool ReleaseImage() = 0;

    public:

        /**
        \brief Pairs a depth swap-chain with this color swap-chain so the runtime can use the rendered depth for reprojection.
        \param[in] depthSwapChain Optional pointer to a depth-format XRSwapChain. Pass null to clear the pairing.
        \remarks When set and the runtime supports \c XR_KHR_composition_layer_depth, XRSession::EndFrame chains an
        XrCompositionLayerDepthInfoKHR for this view's projection layer entry, populated with the depth swap-chain's
        currently-released image and the near/far values from XRFrameState.

        Has no effect if the runtime doesn't support the extension (composition still uses the color swap-chain only).
        The application is responsible for acquire/wait/release on the depth swap-chain in lockstep with its color
        counterpart, and for rendering depth values that match the projection matrix specified by \c XRFrameState::nearZ/farZ.
        \see XRSession::GetSupportedDepthFormats
        \see XRFrameState::nearZ
        */
        virtual void SetDepthCompanion(XRSwapChain* depthSwapChain) = 0;

        //! Returns the depth swap-chain paired via SetDepthCompanion, or null if none.
        virtual XRSwapChain* GetDepthCompanion() const = 0;

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
