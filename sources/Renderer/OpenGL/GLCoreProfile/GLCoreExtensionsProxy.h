/*
 * GLCoreExtensionsProxy.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../OpenGL.h"


namespace LLGL
{


#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    RTYPE APIENTRY Proxy_##NAME ARGS

// Include inline header for proxy function declarations
#include "GLCoreExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL



// ================================================================================
