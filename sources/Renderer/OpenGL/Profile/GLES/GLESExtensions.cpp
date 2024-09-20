/*
 * GLESExtensions.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLESExtensions.h"
#include "../../GLCore.h"


namespace LLGL
{


/* ~~~~~ Define all GLES extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = Proxy_##NAME

// Include inline header for object definitions
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Define proxy implementations for GLES extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS)     \
    GL_APICALL RTYPE GL_APIENTRY Proxy_##NAME ARGS  \
    {                                               \
        ErrUnsupportedGLProc(#NAME);                \
    }

// Include inline header for proxy function definitions
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL



// ================================================================================
