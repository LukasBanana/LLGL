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
 * IndexFormat class
 */

IndexFormat::IndexFormat() :
    dataType_ { SharpLLGL::DataType::UInt32 }
{
}

IndexFormat::IndexFormat(IndexFormat^ rhs) :
    dataType_ { rhs->DataType }
{
}

IndexFormat::IndexFormat(SharpLLGL::DataType dataType) :
    dataType_ { dataType }
{
}

SharpLLGL::DataType IndexFormat::DataType::get()
{
    return dataType_;
}

unsigned int IndexFormat::FormatSize::get()
{
    return LLGL::DataTypeSize(static_cast<LLGL::DataType>(dataType_));
}


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
    Format = gcnew IndexFormat();
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
