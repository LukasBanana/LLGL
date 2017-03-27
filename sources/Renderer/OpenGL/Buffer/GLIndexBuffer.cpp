/*
 * GLIndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLIndexBuffer.h"


namespace LLGL
{


GLIndexBuffer::GLIndexBuffer(const IndexFormat& indexFormat) :
    GLBuffer    ( BufferType::Index ),
    indexFormat_( indexFormat       )
{
}


} // /namespace LLGL



// ================================================================================
