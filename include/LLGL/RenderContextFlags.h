/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_FLAGS_H
#define LLGL_RENDER_CONTEXT_FLAGS_H


#include "Export.h"
#include "Types.h"
#include "PipelineStateFlags.h"
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Video mode descriptor structure.
\remarks This is mainly used to set the video mode of a RenderContext object.
The counterpart for a physical display mode is the DisplayModeDescriptor structure.
\see RenderContext::SetVideoMode
\see DisplayModeDescriptor
*/
struct VideoModeDescriptor
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
    To determine the actual color format of a render context, use the RenderContext::GetColorFormat function.
    \see RenderContext::GetColorFormat
    */
    int             colorBits       = 32;

    /**
    \brief Number of bits for each pixel in the depth buffer. Should be 24, 32, or zero to disable depth buffer. By default 24.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the depth buffer.
    To determine the actual depth-stencil format of a render context, use the RenderContext::GetDepthStencilFormat function.
    \see RenderContext::GetDepthStencilFormat
    */
    int             depthBits       = 24;

    /**
    \brief Number of bits for each pixel in the stencil buffer. Should be 8, or zero to disable stencil buffer. By default 8.
    \remarks This is only a hint to the renderer and there is no guarantee which hardware format is finally used for the stencil buffer.
    To determine the actual depth-stencil format of a render context, use the RenderContext::GetDepthStencilFormat function.
    \see RenderContext::GetDepthStencilFormat
    */
    int             stencilBits     = 8;

    //! Specifies whether to enable fullscreen mode or windowed mode. By default windowed mode.
    bool            fullscreen      = false;

    /**
    \brief Number of swap-chain buffers. By default 2 (for double-buffering).
    \remarks This is only a hint to the renderer and there is no guarantee how many buffers are finally used for the swap chain.
    Especially OpenGL does not support custom swap chain sizes.
    If this value is 0, the video mode is invalid.
    */
    std::uint32_t   swapChainSize   = 2;
};

/**
\brief Render context descriptor structure.
\see RenderSystem::CreateRenderContext
*/
struct RenderContextDescriptor
{
    //! Video mode descriptor.
    VideoModeDescriptor     videoMode;

    /**
    \brief Number of samples for the swap-chain buffers. By default 1.
    \remarks If the specified number of samples is not supported, LLGL will silently reduce it.
    The actual number of samples can be queried by the \c GetSamples function of the RenderTarget interface.
    \see RenderTarget::GetSamples
    */
    std::uint32_t           samples         = 1;

    /**
    \brief Vertical synchronisation (V-sync) interval.
    \remarks This is typically 0 to disable V-sync or 1 to enable V-sync, but higher values are possible, too.
    A value of 2 for instance effectively halves the frame refresh rate that the active display is capable of,
    e.g. a display with a refresh rate of 60 Hz and a V-sync value of 2 limits the frame rate to 30 Hz.
    */
    std::uint32_t           vsyncInterval   = 0;
};


/* ----- Operators ----- */

//! Compares the two specified video mode descriptors on equality.
LLGL_EXPORT bool operator == (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);

//! Compares the two specified video mode descriptors on inequality.
LLGL_EXPORT bool operator != (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs);


} // /namespace LLGL


#endif



// ================================================================================
