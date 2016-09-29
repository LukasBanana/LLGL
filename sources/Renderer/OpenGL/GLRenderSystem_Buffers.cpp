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
#include "Buffer/GLVertexBuffer_.h"
#include "Buffer/GLIndexBuffer_.h"


namespace LLGL
{


/* ----- Hardware Buffers ------ */

template <typename To, typename From>
GLHardwareBuffer& BindAndGetHWBuffer(From& buffer)
{
    auto& bufferGL = LLGL_CAST(To&, buffer);
    GLStateManager::active->BindBuffer(bufferGL);
    return bufferGL.hwBuffer;
}

Buffer* GLRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    /* Create either base of sub-class GLBuffer object */
    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            /* Create vertex buffer and build vertex array */
            auto bufferGL = MakeUnique<GLVertexBuffer_>();
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
                bufferGL->BuildVertexArray(desc.vertexBufferDesc.vertexFormat);
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
        break;

        case BufferType::Index:
        {
            /* Create index buffer and store index format */
            auto bufferGL = MakeUnique<GLIndexBuffer_>(desc.indexBufferDesc.indexFormat);
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

void GLRenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void GLRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind and update buffer sub-data */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    GLStateManager::active->BindBuffer(bufferGL);
    bufferGL.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

#if 1//TODO: remove

VertexBuffer* GLRenderSystem::CreateVertexBuffer(const VertexBufferDescriptor& desc, const void* initialData)
{
    auto vertexBufferGL = MakeUnique<GLVertexBuffer>();
    GLStateManager::active->BindBuffer(*vertexBufferGL);
    {
        /* Update buffer data and update new vertex format */
        vertexBufferGL->hwBuffer.BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
        vertexBufferGL->UpdateVertexFormat(desc.vertexFormat);
    }
    return TakeOwnership(vertexBuffers_, std::move(vertexBufferGL));
}

IndexBuffer* GLRenderSystem::CreateIndexBuffer(const IndexBufferDescriptor& desc, const void* initialData)
{
    auto indexBufferGL = MakeUnique<GLIndexBuffer>();
    GLStateManager::active->BindBuffer(*indexBufferGL);
    {
        /* Update buffer data and update new index format */
        indexBufferGL->hwBuffer.BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
        indexBufferGL->UpdateIndexFormat(desc.indexFormat);
    }
    return TakeOwnership(indexBuffers_, std::move(indexBufferGL));
}

ConstantBuffer* GLRenderSystem::CreateConstantBuffer(const ConstantBufferDescriptor& desc, const void* initialData)
{
    LLGL_ASSERT_CAP(hasConstantBuffers);
    auto constantBufferGL = MakeUnique<GLConstantBuffer>();
    GLStateManager::active->BindBuffer(*constantBufferGL);
    {
        constantBufferGL->hwBuffer.BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
    }
    return TakeOwnership(constantBuffers_, std::move(constantBufferGL));
}

StorageBuffer* GLRenderSystem::CreateStorageBuffer(const StorageBufferDescriptor& desc, const void* initialData)
{
    LLGL_ASSERT_CAP(hasStorageBuffers);
    auto storageBufferGL = MakeUnique<GLStorageBuffer>();
    GLStateManager::active->BindBuffer(*storageBufferGL);
    {
        storageBufferGL->hwBuffer.BufferData(initialData, desc.size, GLTypes::Map(desc.usage));
    }
    return TakeOwnership(storageBuffers_, std::move(storageBufferGL));
}

void GLRenderSystem::Release(VertexBuffer& vertexBuffer)
{
    RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void GLRenderSystem::Release(IndexBuffer& indexBuffer)
{
    RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void GLRenderSystem::Release(ConstantBuffer& constantBuffer)
{
    RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void GLRenderSystem::Release(StorageBuffer& storageBuffer)
{
    RemoveFromUniqueSet(storageBuffers_, &storageBuffer);
}

void GLRenderSystem::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHWBuffer<GLVertexBuffer>(vertexBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHWBuffer<GLIndexBuffer>(indexBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHWBuffer<GLConstantBuffer>(constantBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHWBuffer<GLStorageBuffer>(storageBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

#endif


} // /namespace LLGL



// ================================================================================
