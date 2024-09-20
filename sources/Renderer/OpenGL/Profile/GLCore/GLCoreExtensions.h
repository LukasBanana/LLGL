/*
 * GLCoreExtensions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_CORE_EXTENSIONS_H
#define LLGL_GL_CORE_EXTENSIONS_H


#include <LLGL/Platform/Platform.h>
#include "../../OpenGL.h"


namespace LLGL
{


/* ~~~~~ Declare all GL core extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    extern PFNTYPE NAME

// Include inline header for object declarations
#include "GLCoreExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Declare proxy implementations for GL core extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS

// Include inline header for proxy function declarations
#include "GLCoreExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL


#endif



// ================================================================================
