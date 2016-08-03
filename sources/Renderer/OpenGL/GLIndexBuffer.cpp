/*
 * GLIndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLIndexBuffer.h"


namespace LLGL
{


GLIndexBuffer::GLIndexBuffer() :
    hwBuffer( GL_ELEMENT_ARRAY_BUFFER )
{
}

void GLIndexBuffer::UpdateIndexFormat(const IndexFormat& indexFormat)
{
    SetIndexFormat(indexFormat);
}


} // /namespace LLGL



// ================================================================================
