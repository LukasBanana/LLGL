/*
 * GLRenderingCaps.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RENDERING_CAPS_H
#define LLGL_GL_RENDERING_CAPS_H


#include <LLGL/RenderSystemFlags.h>
#include <vector>


namespace LLGL
{


// Queries all OpenGL rendering capacilities.
void GLQueryRenderingCaps(RenderingCapabilities& caps);

// Queries a string used to identify invalidated pipeline caches. This includes the shader binary format.
void GLQueryPipelineCacheID(std::vector<char>& cacheID);


} // /namespace LLGL


#endif



// ================================================================================
