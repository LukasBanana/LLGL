/*
 * WebGLExtensions.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WEBGL_EXTENSIONS_H
#define LLGL_WEBGL_EXTENSIONS_H


#include <LLGL/Platform/Platform.h>
#include "../../OpenGL.h"


namespace LLGL
{


/* ~~~~~ Declare all non-default enabled WebGL extension functions (Emscripten doesn't include GL_OES_draw_buffers_indexed for instance) ~~~~~ */

#define DECL_GLPROC(EXTENSION, PFNTYPE, NAME, RTYPE, PARAMS, ARGS) \
    extern PFNTYPE NAME

// Include inline header for object declarations
#include "WebGLExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Declare proxy implementations for WebGL extension functions ~~~~~ */

#define DECL_GLPROC(EXTENSION, PFNTYPE, NAME, RTYPE, PARAMS, ARGS) \
    GL_APICALL RTYPE GL_APIENTRY Proxy_##NAME PARAMS

// Include inline header for proxy function declarations
#include "WebGLExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
