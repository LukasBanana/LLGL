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

VertexBuffer* GLRenderSystem::CreateVertexBuffer()
{
    return TakeOwnership(vertexBuffers_, MakeUnique<GLVertexBuffer>());
}

IndexBuffer* GLRenderSystem::CreateIndexBuffer()
{
    return TakeOwnership(indexBuffers_, MakeUnique<GLIndexBuffer>());
}

ConstantBuffer* GLRenderSystem::CreateConstantBuffer()
{
    LLGL_ASSERT_CAP(hasConstantBuffers);
    return TakeOwnership(constantBuffers_, MakeUnique<GLConstantBuffer>());
}

StorageBuffer* GLRenderSystem::CreateStorageBuffer()
{
    LLGL_ASSERT_CAP(hasStorageBuffers);
    return TakeOwnership(storageBuffers_, MakeUnique<GLStorageBuffer>());
}

VertexBuffer* GLRenderSystem::CreateVertexBuffer(std::size_t size, const BufferUsage usage, const VertexFormat& vertexFormat, const void* initialData)
{
    auto vertexBufferGL = MakeUnique<GLVertexBuffer>();
    GLStateManager::active->BindBuffer(*vertexBufferGL);
    {
        /* Update buffer data and update new vertex format */
        vertexBufferGL->hwBuffer.BufferData(initialData, size, GLTypes::Map(usage));
        vertexBufferGL->UpdateVertexFormat(vertexFormat);
    }
    return TakeOwnership(vertexBuffers_, std::move(vertexBufferGL));
}

IndexBuffer* GLRenderSystem::CreateIndexBuffer(std::size_t size, const BufferUsage usage, const IndexFormat& indexFormat, const void* initialData)
{
    auto indexBufferGL = MakeUnique<GLIndexBuffer>();
    GLStateManager::active->BindBuffer(*indexBufferGL);
    {
        /* Update buffer data and update new index format */
        indexBufferGL->hwBuffer.BufferData(initialData, size, GLTypes::Map(usage));
        indexBufferGL->UpdateIndexFormat(indexFormat);
    }
    return TakeOwnership(indexBuffers_, std::move(indexBufferGL));
}

ConstantBuffer* GLRenderSystem::CreateConstantBuffer(std::size_t size, const BufferUsage usage, const void* initialData)
{
    LLGL_ASSERT_CAP(hasConstantBuffers);
    auto constantBufferGL = MakeUnique<GLConstantBuffer>();
    GLStateManager::active->BindBuffer(*constantBufferGL);
    {
        constantBufferGL->hwBuffer.BufferData(initialData, size, GLTypes::Map(usage));
    }
    return TakeOwnership(constantBuffers_, std::move(constantBufferGL));
}

StorageBuffer* GLRenderSystem::CreateStorageBuffer(std::size_t size, const BufferUsage usage, const void* initialData)
{
    LLGL_ASSERT_CAP(hasStorageBuffers);
    auto storageBufferGL = MakeUnique<GLStorageBuffer>();
    GLStateManager::active->BindBuffer(*storageBufferGL);
    {
        storageBufferGL->hwBuffer.BufferData(initialData, size, GLTypes::Map(usage));
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

void GLRenderSystem::SetupVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);

    /* Update buffer data and update new vertex format */
    vertexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypes::Map(usage));
    vertexBufferGL.UpdateVertexFormat(vertexFormat);
}

void GLRenderSystem::SetupIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Update buffer data and update new index format */
    indexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypes::Map(usage));
    indexBufferGL.UpdateIndexFormat(indexFormat);
}

void GLRenderSystem::SetupConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    BindAndGetHWBuffer<GLConstantBuffer>(constantBuffer).BufferData(data, dataSize, GLTypes::Map(usage));
}

void GLRenderSystem::SetupStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    BindAndGetHWBuffer<GLStorageBuffer>(storageBuffer).BufferData(data, dataSize, GLTypes::Map(usage));
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


} // /namespace LLGL



// ================================================================================
