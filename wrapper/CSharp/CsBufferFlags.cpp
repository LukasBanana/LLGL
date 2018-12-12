/*
 * CsBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsBufferFlags.h"
#include <LLGL/Format.h>


namespace SharpLLGL
{


/*
 * VertexBufferDescriptor class
 */

BufferDescriptor::VertexBufferDescriptor::VertexBufferDescriptor()
{
    Format = gcnew VertexFormat();
}


/*
 * IndexBufferDescriptor class
 */

BufferDescriptor::IndexBufferDescriptor::IndexBufferDescriptor()
{
    Format = SharpLLGL::Format::R32UInt;
}


/*
 * StorageBufferDescriptor class
 */

BufferDescriptor::StorageBufferDescriptor::StorageBufferDescriptor()
{
    StorageType = StorageBufferType::Buffer;
    Format      = SharpLLGL::Format::RGBA32Float;
}


/*
 * BufferDescriptor class
 */

BufferDescriptor::BufferDescriptor()
{
    Size            = 0;
    BindFlags       = SharpLLGL::BindFlags::None;
    CPUAccessFlags  = SharpLLGL::CPUAccessFlags::None;
    MiscFlags       = SharpLLGL::MiscFlags::None;
    VertexBuffer    = gcnew VertexBufferDescriptor();
    IndexBuffer     = gcnew IndexBufferDescriptor();
    StorageBuffer   = gcnew StorageBufferDescriptor();
}


} // /namespace SharpLLGL



// ================================================================================
