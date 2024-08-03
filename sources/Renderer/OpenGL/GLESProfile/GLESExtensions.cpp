/*
 * GLESExtensions.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLESExtensions.h"
#include "GLESExtensionsProxy.h"


namespace LLGL
{


#define DECL_GLPROC(PFNTYPE, NAME, RTYPE, ARGS) \
    PFNTYPE NAME = Proxy_##NAME

// Include inline header for object definitions
#include "GLESExtensionsDecl.inl"

#undef DECL_GLPROC


} // /namespace LLGL



// ================================================================================
