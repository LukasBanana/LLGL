/*
 * GLBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferArray.h"
#include "GLBuffer.h"
#include "../../CheckedCast.h"


namespace LLGL
{


GLBufferArray::GLBufferArray(const BufferType type, unsigned int numBuffers, Buffer* const * bufferArray) :
    BufferArray( type )
{
    /* Store the ID of each GLBuffer inside the array */
    for (idArray_.reserve(numBuffers); numBuffers > 0; --numBuffers)
    {
        auto bufferGL = LLGL_CAST(GLBuffer*, (*bufferArray));
        idArray_.push_back(bufferGL->GetID());
        ++bufferArray;
    }
}


} // /namespace LLGL



// ================================================================================
