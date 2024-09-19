/*
 * GLSharedContextVertexArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHARED_CONTEXT_VERTEX_ARRAY_H
#define LLGL_GL_SHARED_CONTEXT_VERTEX_ARRAY_H


#if LLGL_GL_ENABLE_OPENGL2X
#   include "GL2XSharedContextVertexArray.h"
#else
#   include "GL3PlusSharedContextVertexArray.h"
#endif


namespace LLGL
{


#if LLGL_GL_ENABLE_OPENGL2X
using GLSharedContextVertexArray = GL2XSharedContextVertexArray;
#else
using GLSharedContextVertexArray = GL3PlusSharedContextVertexArray;
#endif


} // /namespace LLGL


#endif



// ================================================================================
