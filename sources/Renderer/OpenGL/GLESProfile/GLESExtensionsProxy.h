/*
 * GLESExtensionsProxy.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GLES_EXTENSIONS_PROXY_H
#define LLGL_GLES_EXTENSIONS_PROXY_H


#include "../OpenGL.h"


namespace LLGL
{


#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    GL_APICALL RTYPE GL_APIENTRY Proxy_##NAME ARGS

// Include inline header for proxy function declarations
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
