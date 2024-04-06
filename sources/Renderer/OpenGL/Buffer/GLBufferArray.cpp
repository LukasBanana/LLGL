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


namespace LLGL
{


GLBufferArray::GLBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) :
    BufferArray { GetCombinedBindFlags(numBuffers, bufferArray) }
{
    BuildArray(numBuffers, bufferArray);
}


/*
 * ======= Protected: =======
 */

void GLBufferArray::BuildArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    /* Store the ID of each GLBuffer inside the array */
    idArray_.clear();
    idArray_.reserve(numBuffers);
    while (GLBuffer* next = NextArrayResource<GLBuffer>(numBuffers, bufferArray))
        idArray_.push_back(next->GetID());
}


} // /namespace LLGL



// ================================================================================
