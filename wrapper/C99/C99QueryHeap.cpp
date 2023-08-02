/*
 * C99QueryHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/QueryHeap.h>
#include <LLGL-C/QueryHeap.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT LLGLQueryType llglGetQueryHeapType(LLGLQueryHeap queryHeap)
{
    return static_cast<LLGLQueryType>(LLGL_PTR(QueryHeap, queryHeap)->GetType());
}


// } /namespace LLGL



// ================================================================================
