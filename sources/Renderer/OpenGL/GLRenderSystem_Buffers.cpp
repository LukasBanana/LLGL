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

#ifdef GL_ARB_buffer_storage

static GLbitfield GetGLBufferFlags(long flags)
{
    GLbitfield flagsGL = 0;

    /* Allways enable dynamic storage, to enable usage of 'glBufferSubData' */
    flagsGL |= GL_DYNAMIC_STORAGE_BIT;

    if ((flags & BufferFlags::MapReadAccess) != 0)
        flagsGL |= GL_MAP_READ_BIT;
    if ((flags & BufferFlags::MapWriteAccess) != 0)
        flagsGL |= GL_MAP_WRITE_BIT;

    return flagsGL;
}

#endif

static GLenum GetGLBufferUsage(long flags)
{
    return ((flags & BufferFlags::DynamicUsage) != 0 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

static GLenum GetGLBufferTarget(const GLBuffer& bufferGL)
{
    return GLTypes::Map(bufferGL.GetType());
}

static void GLBufferStorage(GLBuffer& bufferGL, const BufferDescriptor& desc, const void* initialData)
{
    #ifdef GL_ARB_buffer_storage
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Allocate buffer with immutable storage */
        glNamedBufferStorage(bufferGL.GetID(), static_cast<GLsizeiptr>(desc.size), initialData, GetGLBufferFlags(desc.flags));
    }
    else
    #endif
    if (HasExtension(GLExt::ARB_buffer_storage))
    {
        /* Bind and allocate buffer with immutable storage */
        GLStateManager::active->BindBuffer(bufferGL);
        glBufferStorage(GetGLBufferTarget(bufferGL), static_cast<GLsizeiptr>(desc.size), initialData, GetGLBufferFlags(desc.flags));
    }
    else
    #endif
    {
        /* Bind and allocate buffer with mutable storage */
        GLStateManager::active->BindBuffer(bufferGL);
        glBufferData(GetGLBufferTarget(bufferGL), static_cast<GLsizeiptr>(desc.size), initialData, GetGLBufferUsage(desc.flags));
    }
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
                GLBufferStorage(*bufferGL, desc, initialData);
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
                GLBufferStorage(*bufferGL, desc, initialData);
            }
            return TakeOwnership(buffers_, std::move(bufferGL));
        }
        break;

        default:
        {
            /* Create generic buffer */
            auto bufferGL = MakeUnique<GLBuffer>(desc.type);
            {
                GLBufferStorage(*bufferGL, desc, initialData);
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

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
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
            GetGLBufferTarget(bufferGL),
            static_cast<GLintptr>(offset),
            static_cast<GLsizeiptr>(dataSize),
            data
        );
    }
}

void* GLRenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        return glMapNamedBuffer(bufferGL.GetID(), GLTypes::Map(access));
    }
    else
    #endif
    {
        GLStateManager::active->BindBuffer(bufferGL);
        return glMapBuffer(GetGLBufferTarget(bufferGL), GLTypes::Map(access));
    }
}

void GLRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glUnmapNamedBuffer(bufferGL.GetID());
    }
    else
    #endif
    {
        GLStateManager::active->BindBuffer(bufferGL);
        glUnmapBuffer(GetGLBufferTarget(bufferGL));
    }
}


} // /namespace LLGL



// ================================================================================
