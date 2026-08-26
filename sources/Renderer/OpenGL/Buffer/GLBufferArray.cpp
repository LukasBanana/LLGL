/*
 * GLBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferArray.h"
#include "GLBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static std::vector<GLBuffer*> GetAsGLBuffers(ArrayView<Buffer*> inBuffers)
{
    std::vector<GLBuffer*> outBuffers;
    outBuffers.resize(inBuffers.size());
    for_range(i, inBuffers.size())
        outBuffers[i] = LLGL_CAST(GLBuffer*, inBuffers[i]);
    return outBuffers;
}

GLBufferArray::GLBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray        { GetCombinedBindFlags(numBuffers, bufferArray)                 },
    bufferInputLayout_ { GetAsGLBuffers(ArrayView<Buffer*>{ bufferArray, numBuffers }) }
{
}


} // /namespace LLGL



// ================================================================================
