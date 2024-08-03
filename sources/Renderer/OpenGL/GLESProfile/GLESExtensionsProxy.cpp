/*
 * GLESExtensionsProxy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../OpenGL.h"
#include "../GLCore.h"
#include <stdexcept>
#include <string>



namespace LLGL
{


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
