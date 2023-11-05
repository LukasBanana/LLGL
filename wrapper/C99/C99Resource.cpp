/*
 * C99Resource.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Resource.h>
#include <LLGL-C/Resource.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT LLGLResourceType llglGetResourceType(LLGLResource resource)
{
    return static_cast<LLGLResourceType>(LLGL_PTR(Resource, resource)->GetResourceType());
}


// } /namespace LLGL



// ================================================================================
