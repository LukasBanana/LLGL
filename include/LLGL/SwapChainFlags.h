/*
 * SwapChainFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SWAP_CHAIN_FLAGS_H
#define LLGL_SWAP_CHAIN_FLAGS_H


#include <LLGL/Types.h>
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
    \brief Screen resolution (in pixels).
    \remarks If the resolution contains a member with a value of 0, the video mode is invalid.
    \see RenderTarget::GetResolution
    */
    Extent2D        resolution;

    /**
    \brief Number of bits for each pixel in the color buffer. Should be 24 or 32. By default 32.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the color buffer.
    To determine the actual color format of a swap-chain, use the SwapChain::GetColorFormat function.
    \see SwapChain::GetColorFormat
    */
    int             colorBits       = 32;

    /**
    \brief Number of bits for each pixel in the depth buffer. Should be 24, 32, or zero to disable depth buffer. By default 24.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the depth buffer.
    To determine the actual depth-stencil format of a swap-chain, use the SwapChain::GetDepthStencilFormat function.
    \see SwapChain::GetDepthStencilFormat
    */
    int             depthBits       = 24;

    /**
    \brief Number of bits for each pixel in the stencil buffer. Should be 8, or zero to disable stencil buffer. By default 8.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the stencil buffer.
    To determine the actual depth-stencil format of a swap-chain, use the SwapChain::GetDepthStencilFormat function.
    \see SwapChain::GetDepthStencilFormat
    */
    int             stencilBits     = 8;

    /**
    \brief Number of samples for the swap-chain buffers. By default 1.
    \remarks If the specified number of samples is not supported, LLGL will silently reduce it.
    The actual number of samples can be queried by the \c GetSamples function of the RenderTarget interface.
    \see RenderTarget::GetSamples
    */
    std::uint32_t   samples         = 1;

    /**
    \brief Number of swap buffers. By default 2 (for double-buffering).
    \remarks This is only a hint to the renderer and there is no guarantee how many buffers are finally used for the swap chain.
    Especially OpenGL does not support custom swap chain sizes.
    \see SwapChain::GetCurrentSwapIndex
    \see SwapChain::GetNumSwapBuffers
    */
    std::uint32_t   swapBuffers     = 2;

    //! Specifies whether to enable fullscreen mode or windowed mode. By default windowed mode.
    bool            fullscreen      = false;
};


} // /namespace LLGL


#endif



// ================================================================================
