/*
 * C99BufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/BufferArray.h>
#include <LLGL-C/BufferArray.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT long llglGetBufferArrayBindFlags(LLGLBufferArray bufferArray)
{
    return LLGL_PTR(BufferArray, bufferArray)->GetBindFlags();
}


// } /namespace LLGL



// ================================================================================
