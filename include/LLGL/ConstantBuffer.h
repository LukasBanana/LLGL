/*
 * ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_CONSTANT_BUFFER_H__
#define __LLGL_CONSTANT_BUFFER_H__


#include "Export.h"
#include "BufferFlags.h"


namespace LLGL
{


//! Constant buffer descriptor structure.
struct ConstantBufferDescriptor
{
    ConstantBufferDescriptor() = default;

    ConstantBufferDescriptor(unsigned int size, BufferUsage usage) :
        size    ( size  ),
        usage   ( usage )
    {
    }

    //! Buffer size (in bytes).
    unsigned int    size    = 0;

    //! Buffer usage (typically "BufferUsage::Dynamic", since a constant buffer is commonly frequently changed).
    BufferUsage     usage   = BufferUsage::Dynamic;
};


//! Constant buffer (also "Uniform Buffer Object") interface.
class LLGL_EXPORT ConstantBuffer
{

    public:

        virtual ~ConstantBuffer()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
