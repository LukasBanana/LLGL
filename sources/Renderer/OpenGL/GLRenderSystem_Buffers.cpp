/*
 * GLRenderSystem_Buffers.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

static GLbitfield GetGLBufferFlags(long flags)
{
    #ifdef GL_ARB_buffer_storage

    GLbitfield flagsGL = 0;

    /* Allways enable dynamic storage, to enable usage of 'glBufferSubData' */
    flagsGL |= GL_DYNAMIC_STORAGE_BIT;

    if ((flags & BufferFlags::MapReadAccess) != 0)
        flagsGL |= GL_MAP_READ_BIT;
    if ((flags & BufferFlags::MapWriteAccess) != 0)
        flagsGL |= GL_MAP_WRITE_BIT;

    return flagsGL;

    #else

    return 0;

    #endif // /GL_ARB_buffer_storage
}

static GLenum GetGLBufferUsage(long flags)
{
    return ((flags & BufferFlags::DynamicUsage) != 0 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

static void GLBufferStorage(GLBuffer& bufferGL, const BufferDescriptor& desc, const void* initialData)
{
    bufferGL.BufferStorage(
        static_cast<GLsizeiptr>(desc.size),
        initialData,
        GetGLBufferFlags(desc.flags),
        GetGLBufferUsage(desc.flags)
    );
}

Buffer* GLRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max()));

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
    RemoveFromUniqueSet(buffers_, &buffer);
}

void GLRenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void GLRenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    dstBufferGL.BufferSubData(static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(dataSize), data);
}

void* GLRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    return bufferGL.MapBuffer(GLTypes::Map(access));
}

void GLRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    bufferGL.UnmapBuffer();
}


} // /namespace LLGL



// ================================================================================
