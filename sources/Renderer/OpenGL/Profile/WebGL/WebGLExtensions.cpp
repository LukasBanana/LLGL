/*
 * WebGLExtensions.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WebGLExtensions.h"
#include "../../GLCore.h"
#include <emscripten/emscripten.h>


namespace LLGL
{


/* ~~~~~ Define all WebGL extension functions ~~~~~ */

#define DECL_GLPROC(EXTENSION, PFNTYPE, NAME, RTYPE, PARAMS, ARGS) \
    PFNTYPE NAME = Proxy_##NAME

// Include inline header for object definitions
#include "WebGLExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Define proxy implementations for WebGL extension functions ~~~~~ */

#define DECL_GLPROC_void(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)        \
    GL_APICALL void GL_APIENTRY Proxy_##NAME PARAMS                     \
    {                                                                   \
        ErrUnsupportedGLProc(#NAME);                                    \
    }

#define DECL_GLPROC_GLboolean(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)   \
    GL_APICALL GLboolean GL_APIENTRY Proxy_##NAME PARAMS                \
    {                                                                   \
        ErrUnsupportedGLProc(#NAME);                                    \
        return GL_FALSE;                                                \
    }

#define DECL_GLPROC(EXTENSION, PFNTYPE, NAME, RTYPE, PARAMS, ARGS)      \
    DECL_GLPROC_##RTYPE(EXTENSION, PFNTYPE, NAME, PARAMS, ARGS)

// Include inline header for proxy function definitions
#include "WebGLExtensionsDecl.inl"

#undef DECL_GLPROC
#undef DECL_GLPROC_void
#undef DECL_GLPROC_GLboolean


} // /namespace LLGL



// ================================================================================
