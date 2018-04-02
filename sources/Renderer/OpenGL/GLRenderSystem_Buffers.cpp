/*
 * GLRenderSystem_Buffers.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../GLCommon/GLTypes.h"
#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLVertexBufferArray.h"


namespace LLGL
{


/* ----- Buffers ------ */

static GLenum GetGLBufferUsage(long flags)
{
    return ((flags & BufferFlags::DynamicUsage) != 0 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

Buffer* GLRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<GLsizeiptr>::max()));

    /* Create either base of sub-class GLBuffer object */
    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            /* Create vertex buffer and build vertex array */
            auto bufferGL = MakeUnique<GLVertexBuffer>();
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, static_cast<GLsizeiptr>(desc.size), GetGLBufferUsage(desc.flags));
                bufferGL->BuildVertexArray(desc.vertexBuffer.format);
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
        break;

        case BufferType::Index:
        {
            /* Create index buffer and store index format */
            auto bufferGL = MakeUnique<GLIndexBuffer>(desc.indexBuffer.format);
            {
                GLStateManager::active->BindBuffer(*bufferGL);
                bufferGL->BufferData(initialData, static_cast<GLsizeiptr>(desc.size), GetGLBufferUsage(desc.flags));
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
                bufferGL->BufferData(initialData, static_cast<GLsizeiptr>(desc.size), GetGLBufferUsage(desc.flags));
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
    }
}

BufferArray* GLRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
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
    /* Notify GL state manager about object release, then release object */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    GLStateManager::NotifyBufferRelease(bufferGL.GetID(), GLStateManager::GetBufferTarget(bufferGL.GetType()));
    RemoveFromUniqueSet(buffers_, &buffer);
}

void GLRenderSystem::Release(BufferArray& bufferArray)
{
    /* Notify GL state manager about object release, then release object */
    auto& bufferArrayGL = LLGL_CAST(GLBufferArray&, bufferArray);

    const auto& bufferIDs = bufferArrayGL.GetIDArray();
    auto bufferTarget = GLStateManager::GetBufferTarget(bufferArrayGL.GetType());

    for (std::size_t i = 0, n = bufferIDs.size(); i < n; ++i)
        GLStateManager::NotifyBufferRelease(bufferIDs[i], bufferTarget);

    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void GLRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);

    #ifdef LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glNamedBufferSubData(
            bufferGL.GetID(),
            static_cast<GLintptr>(offset),
            static_cast<GLsizeiptr>(dataSize),
            data
        );
    }
    else
    #endif
    {
        GLStateManager::active->BindBuffer(bufferGL);
        glBufferSubData(
            GLTypes::Map(bufferGL.GetType()),
            static_cast<GLintptr>(offset),
            static_cast<GLsizeiptr>(dataSize),
            data
        );
    }
}

static GLBuffer& BindAndGetGLBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    GLStateManager::active->BindBuffer(bufferGL);
    return bufferGL;
}

void* GLRenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    /* Bind and map buffer */
    return BindAndGetGLBuffer(buffer).MapBuffer(GLTypes::Map(access));
}

void GLRenderSystem::UnmapBuffer(Buffer& buffer)
{
    /* Bind and unmap buffer */
    BindAndGetGLBuffer(buffer).UnmapBuffer();
}


} // /namespace LLGL



// ================================================================================
