/*
 * CsBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsBufferFlags.h"
#include <LLGL/Format.h>


namespace LHermanns
{

namespace LLGL
{


/*
 * IndexFormat class
 */

IndexFormat::IndexFormat() :
    dataType_ { LHermanns::LLGL::DataType::UInt32 }
{
}

IndexFormat::IndexFormat(IndexFormat^ rhs) :
    dataType_ { rhs->DataType }
{
}

IndexFormat::IndexFormat(LHermanns::LLGL::DataType dataType) :
    dataType_ { dataType }
{
}

LHermanns::LLGL::DataType IndexFormat::DataType::get()
{
    return dataType_;
}

unsigned int IndexFormat::FormatSize::get()
{
    return ::LLGL::DataTypeSize(static_cast<::LLGL::DataType>(dataType_));
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
    Format      = LHermanns::LLGL::Format::RGBA32Float;
}


/*
 * BufferDescriptor class
 */

BufferDescriptor::BufferDescriptor()
{
    Type            = BufferType::Vertex;
    Flags           = BufferFlags::None;
    Size            = 0;
    VertexBuffer    = gcnew VertexBufferDescriptor();
    IndexBuffer     = gcnew IndexBufferDescriptor();
    StorageBuffer   = gcnew StorageBufferDescriptor();
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
