/*
 * GLESExtensions.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GLES_EXTENSIONS_H
#define LLGL_GLES_EXTENSIONS_H


#include <LLGL/Platform/Platform.h>
#include "../../OpenGL.h"


namespace LLGL
{


/* ~~~~~ Declare all GLES extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    extern PFNTYPE NAME

// Include inline header for object declarations
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Declare proxy implementations for GLES extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    GL_APICALL RTYPE GL_APIENTRY Proxy_##NAME ARGS

// Include inline header for proxy function declarations
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
