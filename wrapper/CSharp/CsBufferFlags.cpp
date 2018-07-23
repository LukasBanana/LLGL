/*
 * CsBufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsBufferFlags.h"


namespace LHermanns
{

namespace LLGL
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
    VertexBuffer    = gcnew VertexBufferDescriptor();
    IndexBuffer     = gcnew IndexBufferDescriptor();
    StorageBuffer   = gcnew StorageBufferDescriptor();
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
