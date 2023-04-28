/*
 * GLCoreExtensionsProxy.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../OpenGL.h"


namespace LLGL
{


#define LLGL_DECL_GL_PROXY_PROCS

// Include inline header for proxy function declarations
#include "GLCoreExtensionsDecl.inl"

#undef LLGL_DECL_GL_PROXY_PROCS


} // /namespace LLGL



// ================================================================================
