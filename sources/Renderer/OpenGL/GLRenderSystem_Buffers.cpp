/*
 * GLRenderSystem_Buffers.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "GLTypes.h"
#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLVertexBufferArray.h"


namespace LLGL
{


/* ----- Hardware Buffers ------ */

Buffer* GLRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    /* Create either base of sub-class GLBuffer object */
    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            /* Create vertex buffer and build vertex array */
            auto bufferGL = MakeUnique<GLVertexBuffer>();
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
                bufferGL->BuildVertexArray(desc.vertexBuffer.vertexFormat);
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
        break;

        case BufferType::Index:
        {
            /* Create index buffer and store index format */
            auto bufferGL = MakeUnique<GLIndexBuffer>(desc.indexBuffer.indexFormat);
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
        break;

        default:
        {
            /* Create generic buffer */
            auto bufferGL = MakeUnique<GLBuffer>(desc.type);
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
    }
}

BufferArray* GLRenderSystem::CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    auto type = (*bufferArray)->GetType();

    if (type == BufferType::Vertex)
    {
        /* Create vertex buffer array and build VAO */
        auto vertexBufferArray = MakeUnique<GLVertexBufferArray>();
        vertexBufferArray->BuildVertexArray(numBuffers, bufferArray);
        return TakeOwnership(bufferArrays_, std::move(vertexBufferArray));
    }

    return TakeOwnership(bufferArrays_, MakeUnique<GLBufferArray>(type, numBuffers, bufferArray));
}

void GLRenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void GLRenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void GLRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind and update buffer sub-data */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    GLStateManager::active->BindBuffer(bufferGL);
    bufferGL.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}


} // /namespace LLGL



// ================================================================================
