/*
 * GLCoreExtensions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCoreExtensions.h"


namespace LLGL
{


#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = nullptr

// Include inline header for object definitions
#include "GLCoreExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL



// ================================================================================
