/*
 * GLCompatExtensions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCompatExtensions.h"
#include "../../GLCore.h"


namespace LLGL
{


/* ~~~~~ Define all GL core extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = nullptr

// Include inline header for object definitions
#include "GLCompatExtensionsDecl.inl"

#undef DECL_GLPROC

/* ~~~~~ Define proxy implementations for GL core extension functions ~~~~~ */

#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS            \
    {                                           \
        ErrUnsupportedGLProc(#NAME);            \
    }

// Include inline header for proxy function definitions
#include "GLCompatExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL



// ================================================================================
