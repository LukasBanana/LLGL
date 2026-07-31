/*
 * GLRenderingCaps.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RENDERING_CAPS_H
#define LLGL_GL_RENDERING_CAPS_H


#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Platform/Platform.h>
#include <vector>


namespace LLGL
{


// Queries all OpenGL rendering capacilities.
void GLQueryRenderingCaps(RenderingCapabilities& caps);

/*
Returns the color formats a GL swap-chain color buffer can have.
The GL backend does not honor SwapChainDescriptor::colorFormat; the format is determined by the
platform's pixel format and then labeled by GLContext::DeduceColorFormat/SetDefaultColorFormat.
*/
inline std::vector<Format> GLGetSupportedSwapChainColorFormats()
{
    #ifdef LLGL_OS_WIN32
    /* Win32 deduces the format from the pixel format's component shifts, which can yield either component order */
    return { Format::RGBA8UNorm, Format::BGRA8UNorm };
    #else
    /* All other platforms use GLContext::SetDefaultColorFormat */
    return { Format::RGBA8UNorm };
    #endif
}

/*
Returns the depth-stencil formats a GL swap-chain depth-stencil buffer can have.
GLContext::DeduceDepthStencilFormat can label any of the four LLGL depth-stencil formats, but the GL backend does not
choose the format: it belongs to the pixel format the windowing system selects, and every platform LLGL supports
requests a 24-bit depth and 8-bit stencil buffer by default. Drivers reliably provide that combination and commonly
substitute it for anything else that is requested, so it is the only format an application can count on here.
*/
inline std::vector<Format> GLGetSupportedSwapChainDepthStencilFormats()
{
    return { Format::D24UNormS8UInt };
}

// Queries a string used to identify invalidated pipeline caches. This includes the shader binary format.
void GLQueryPipelineCacheID(std::vector<char>& cacheID);


} // /namespace LLGL


#endif



// ================================================================================
