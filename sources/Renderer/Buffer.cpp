/*
 * Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Buffer.h>


namespace LLGL
{


Buffer::Buffer(const BufferType type) :
    type_ { type }
{
}

ResourceType Buffer::QueryResourceType() const
{
    switch (GetType())
    {
        case BufferType::Vertex:        return ResourceType::VertexBuffer;
        case BufferType::Index:         return ResourceType::IndexBuffer;
        case BufferType::Constant:      return ResourceType::ConstantBuffer;
        case BufferType::Storage:       return ResourceType::StorageBuffer;
        case BufferType::StreamOutput:  return ResourceType::StreamOutputBuffer;
    }
    return ResourceType::Undefined;
}


} // /namespace LLGL



// ================================================================================
