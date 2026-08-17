/*
 * SwapChainFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SWAP_CHAIN_FLAGS_H
#define LLGL_SWAP_CHAIN_FLAGS_H


#include <LLGL/Types.h>
#include <LLGL/Format.h>
#include <cstdint>


namespace LLGL
{


/* ----- Flags ----- */

/**
\brief Swap-chain resize buffers flags.
\see SwapChain::ResizeBuffers
*/
struct ResizeBuffersFlags
{
    enum
    {
        /**
        \brief Adapts the swap-chain's surface for the new resolution.
        \see Surface::AdaptForVideoMode
        */
        AdaptSurface    = (1 << 0),

        /**
        \brief Puts the swap-chain into fullscreen mode.
        \remarks This implies AdaptSurface but cannot be used in combination with the WindowedMode flag.
        \see WindowedMode
        \see SwapChain::SwitchFullscreen
        */
        FullscreenMode  = (1 << 1),

        /**
        \brief Puts the swap-chain into windowed mode.
        \remarks This implies AdaptSurface but cannot be used in combination with the FullscreenMode flag.
        \see FullscreenMode
        \see SwapChain::SwitchFullscreen
        */
        WindowedMode    = (1 << 2),

        /**
        \brief Only reports success if the swap buffers were resized to the exact resolution requested.
        \remarks A requested resolution is only a hint: the platform may clamp it to what the surface permits.
        By default, SwapChain::ResizeBuffers returns true as long as the swap buffers were resized to a valid
        resolution, even if it differs from the request. With this flag, it returns false on such a mismatch.
        The swap buffers are resized either way and SwapChain::GetResolution reports what was actually allocated.
        \see SwapChain::ResizeBuffers
        \see SwapChain::GetResolution
        */
        StrictResolution = (1 << 3),
    };
};


/* ----- Structures ----- */

/**
\brief Swap chain descriptor structure.
\see RenderSystem::CreateSwapChain
*/
struct SwapChainDescriptor
{
    /**
    \brief Optional name for debugging purposes. By default null.
    \remarks The final name of the native hardware resource is implementation defined.
    \see RenderSystemChild::SetDebugName
    */
    const char*     debugName           = nullptr;

    /**
    \brief Screen resolution (in pixels).
    \remarks If the resolution contains a member with a value of 0, the video mode is invalid.
    \see RenderTarget::GetResolution
    */
    Extent2D        resolution;

    /**
    \brief Preferred color format for the swap-chain buffers. By default Format::Undefined.
    \remarks If this is Format::Undefined, the renderer picks a format automatically from the deprecated \c colorBits field (the default behavior).
    Otherwise the renderer uses this format if the presentation surface supports it, and falls back to its
    automatic selection if it does not. This is primarily intended to request an sRGB format (e.g.
    Format::BGRA8UNorm_sRGB), so the hardware performs the linear-to-sRGB conversion on write.
    To determine the actual color format of a swap-chain, use the SwapChain::GetColorFormat function.
    \see SwapChain::GetColorFormat
    */
    Format          colorFormat         = Format::Undefined;

    /**
    \brief Preferred depth-stencil format for the swap-chain. By default Format::Undefined.
    \remarks If this is Format::Undefined, the renderer picks a format automatically from the deprecated \c depthBits and \c stencilBits fields (the default behavior).
    Otherwise this must be one of the depth-stencil formats, i.e. Format::D16UNorm, Format::D24UNormS8UInt, Format::D32Float, or Format::D32FloatS8X24UInt.
    The renderer uses this format if the hardware supports it and falls back to its automatic selection if it does not.
    To determine the actual depth-stencil format of a swap-chain, use the SwapChain::GetDepthStencilFormat function.
    \see SwapChain::GetDepthStencilFormat
    */
    Format          depthStencilFormat  = Format::Undefined;

    /**
    \brief Number of bits for each pixel in the color buffer. Should be 24 or 32. By default 32.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the color buffer.
    To determine the actual color format of a swap-chain, use the SwapChain::GetColorFormat function.
    \note This field is deprecated and ignored if \c colorFormat is not Format::Undefined; Use \c colorFormat instead!
    \see colorFormat
    \see SwapChain::GetColorFormat
    \todo Deprecate these as soon as `colorFormat` fields is supported in all backends.
    */
    int             colorBits           = 32;

    /**
    \brief Number of bits for each pixel in the depth buffer. Should be 24, 32, or zero to disable depth buffer. By default 24.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the depth buffer.
    To determine the actual depth-stencil format of a swap-chain, use the SwapChain::GetDepthStencilFormat function.
    \note This field is deprecated and ignored if \c depthStencilFormat is not Format::Undefined; Use \c depthStencilFormat instead!
    \see depthStencilFormat
    \see SwapChain::GetDepthStencilFormat
    \todo Deprecate these as soon as `depthStencilFormat` fields is supported in all backends.
    */
    int             depthBits           = 24;

    /**
    \brief Number of bits for each pixel in the stencil buffer. Should be 8, or zero to disable stencil buffer. By default 8.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the stencil buffer.
    To determine the actual depth-stencil format of a swap-chain, use the SwapChain::GetDepthStencilFormat function.
    \note This field is deprecated and ignored if \c depthStencilFormat is not Format::Undefined; Use \c depthStencilFormat instead!
    \see depthStencilFormat
    \see SwapChain::GetDepthStencilFormat
    \todo Deprecate these as soon as `depthStencilFormat` fields is supported in all backends.
    */
    int             stencilBits         = 8;

    /**
    \brief Number of samples for the swap-chain buffers. By default 1.
    \remarks If the specified number of samples is not supported, LLGL will silently reduce it.
    The actual number of samples can be queried by the \c GetSamples function of the RenderTarget interface.
    \see RenderTarget::GetSamples
    */
    std::uint32_t   samples             = 1;

    /**
    \brief Number of swap buffers. By default 2 (for double-buffering).
    \remarks This is only a hint to the renderer and there is no guarantee how many buffers are finally used for the swap chain.
    Especially OpenGL does not support custom swap chain sizes.
    \see SwapChain::GetCurrentSwapIndex
    \see SwapChain::GetNumSwapBuffers
    */
    std::uint32_t   swapBuffers         = 2;

    /**
    \brief Specifies whether to create the swap-chain initially in fullscreen mode or windowed mode otherwise.
    \see SwapChain::ResizeBuffers
    \see ResizeBuffersFlags::FullscreenMode
    */
    bool            fullscreen          = false;

    /**
    \brief Specifies whether to create the default surface for the swap-chain with the resizable attribute.
    \remarks If a custom surface is specified, this field is ignored.
    \see WindowFlags::Resizable
    */
    bool            resizable           = false;
};


} // /namespace LLGL


#endif



// ================================================================================
