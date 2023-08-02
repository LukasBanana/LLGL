/*
 * C99Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Buffer.h>
#include <LLGL-C/Buffer.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT long llglGetBufferBindFlags(LLGLBuffer buffer)
{
    return LLGL_PTR(Buffer, buffer)->GetBindFlags();
}

LLGL_C_EXPORT void llglGetBufferDesc(LLGLBuffer buffer, LLGLBufferDescriptor* outDesc)
{
    LLGL_ASSERT_PTR(outDesc);
    const BufferDescriptor internalDesc = LLGL_PTR(Buffer, buffer)->GetDesc();
    *outDesc = *reinterpret_cast<const LLGLBufferDescriptor*>(&internalDesc);
}


// } /namespace LLGL



// ================================================================================
