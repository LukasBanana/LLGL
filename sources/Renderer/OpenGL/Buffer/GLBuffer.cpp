/*
 * GLBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBuffer.h"
#include "GLBufferConverter.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../../Core/Helper.h"
#include <memory>


namespace LLGL
{


GLBuffer::GLBuffer(const BufferType type) :
    Buffer { type }
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Creates a new GL buffer object and binds it to an unspecified target */
        glCreateBuffers(1, &id_);
    }
    else
    #endif
    {
        /* Creates a new GL buffer object (must be bound to a target before it can be used) */
        glGenBuffers(1, &id_);
    }
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &id_);
    GLStateManager::active->NotifyBufferRelease(id_, GLStateManager::GetBufferTarget(GetType()));
}

void GLBuffer::BufferStorage(GLsizeiptr size, const void* data, GLbitfield flags, GLenum usage)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Allocate buffer with immutable storage (4.5+) */
        glNamedBufferStorage(GetID(), size, data, flags);
    }
    else
    #endif // /GL_ARB_direct_state_access
    #ifdef GL_ARB_buffer_storage
    if (HasExtension(GLExt::ARB_buffer_storage))
    {
        /* Bind and allocate buffer with immutable storage (GL 4.4+) */
        GLStateManager::active->BindBuffer(*this);
        glBufferStorage(GLTypes::Map(GetType()), size, data, flags);
    }
    else
    #endif // /GL_ARB_buffer_storage
    {
        /* Bind and allocate buffer with mutable storage */
        GLStateManager::active->BindBuffer(*this);
        glBufferData(GLTypes::Map(GetType()), size, data, usage);
    }
}

void GLBuffer::BufferSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glNamedBufferSubData(GetID(), offset, size, data);
    }
    else
    #endif // /GL_ARB_direct_state_access
    {
        GLStateManager::active->BindBuffer(*this);
        glBufferSubData(GLTypes::Map(GetType()), offset, size, data);
    }
}

// Fill destination buffer repeatedly with the source buffer
static void FillBuffer(char* dst, char* dstEnd, const void* src, std::size_t srcSize)
{
    if (std::distance(dst, dstEnd) % srcSize == 0)
    {
        while (dst != dstEnd)
        {
            ::memcpy(dst, src, srcSize);
            dst += srcSize;
        }
    }
}

void GLBuffer::ClearBufferData(GLenum internalFormat, GLenum format, GLenum type, const void* data)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glClearNamedBufferData(GetID(), internalFormat, format, type, data);
    }
    else
    #endif // /GL_ARB_direct_state_access
    if (HasExtension(GLExt::ARB_clear_buffer_object))
    {
        GLStateManager::active->BindBuffer(*this);
        glClearBufferData(GLTypes::Map(GetType()), internalFormat, format, type, data);
    }
    else
    {
        /* Emulate buffer fill operation */
        GLStateManager::active->BindBuffer(*this);

        /* Query buffer size */
        GLenum  bufferTarget    = GLTypes::Map(GetType());
        GLint   bufferSize      = 0;

        glGetBufferParameteriv(bufferTarget, GL_BUFFER_SIZE, &bufferSize);

        /* Allocate intermediate buffer to fill the GPU buffer with */
        auto dstSize = static_cast<std::size_t>(bufferSize);
        auto intermediateBuffer = MakeUniqueArray<char>(dstSize);

        /* Convert format data to fill the buffer with */
        const auto srcData = reinterpret_cast<const GLBufferConverter::FormatData*>(data);
        GLBufferConverter::FormatData formatData;

        auto formatSize = GLBufferConverter::ConvertFormatData(
            formatData,
            GLBufferConverter::GetFormatDataDesc(internalFormat),
            *srcData,
            GLBufferConverter::GetFormatDataDesc(format)
        );

        /* Fill intermediate buffer with data */
        FillBuffer(intermediateBuffer.get(), intermediateBuffer.get() + dstSize, &formatData, formatSize);

        /* Submit intermeidate buffer to GPU buffer */
        glBufferSubData(bufferTarget, 0, static_cast<GLintptr>(bufferSize), intermediateBuffer.get());
    }
}

void GLBuffer::ClearBufferSubData(GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glClearNamedBufferSubData(GetID(), internalFormat, offset, size, format, type, data);
    }
    else
    #endif // /GL_ARB_direct_state_access
    if (HasExtension(GLExt::ARB_clear_buffer_object))
    {
        GLStateManager::active->BindBuffer(*this);
        glClearBufferSubData(GLTypes::Map(GetType()), internalFormat, offset, size, format, type, data);
    }
    else
    {
        /* Emulate buffer fill operation */
        GLStateManager::active->BindBuffer(*this);

        /* Allocate intermediate buffer to fill the GPU buffer with */
        auto dstSize = static_cast<std::size_t>(size);
        auto intermediateBuffer = MakeUniqueArray<char>(dstSize);

        /* Convert format data to fill the buffer with */
        const auto srcData = reinterpret_cast<const GLBufferConverter::FormatData*>(data);
        GLBufferConverter::FormatData formatData;

        auto formatSize = GLBufferConverter::ConvertFormatData(
            formatData,
            GLBufferConverter::GetFormatDataDesc(internalFormat),
            *srcData,
            GLBufferConverter::GetFormatDataDesc(format)
        );

        /* Fill intermediate buffer with data */
        FillBuffer(intermediateBuffer.get(), intermediateBuffer.get() + dstSize, &formatData, formatSize);

        /* Submit intermeidate buffer to GPU buffer */
        glBufferSubData(GLTypes::Map(GetType()), offset, size, intermediateBuffer.get());
    }
}

void GLBuffer::CopyBufferSubData(const GLBuffer& readBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Copy buffer directly (GL 4.5+) */
        glCopyNamedBufferSubData(readBuffer.GetID(), GetID(), readOffset, writeOffset, size);
    }
    else
    #endif // /GL_ARB_direct_state_access
    #ifdef GL_ARB_copy_buffer
    if (HasExtension(GLExt::ARB_copy_buffer))
    {
        /* Bind source and destination buffer for copy operation (GL 3.1+) */
        GLStateManager::active->BindBuffer(GLBufferTarget::COPY_READ_BUFFER, readBuffer.GetID());
        GLStateManager::active->BindBuffer(GLBufferTarget::COPY_WRITE_BUFFER, GetID());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, readOffset, writeOffset, size);
    }
    else
    #endif // /GL_ARB_copy_buffer
    {
        /* Emulate buffer copy operation */
        auto intermediateBuffer = MakeUniqueArray<char>(size);

        /* Read source buffer data */
        GLStateManager::active->BindBuffer(readBuffer);
        glGetBufferSubData(GLTypes::Map(readBuffer.GetType()), readOffset, size, intermediateBuffer.get());

        /* Write destination buffer data */
        GLStateManager::active->BindBuffer(*this);
        glBufferSubData(GLTypes::Map(GetType()), writeOffset, size, intermediateBuffer.get());
    }
}

void* GLBuffer::MapBuffer(GLenum access)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        return glMapNamedBuffer(GetID(), access);
    }
    else
    #endif // /GL_ARB_direct_state_access
    {
        GLStateManager::active->BindBuffer(*this);
        return glMapBuffer(GLTypes::Map(GetType()), access);
    }
}

void GLBuffer::UnmapBuffer()
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glUnmapNamedBuffer(GetID());
    }
    else
    #endif // /GL_ARB_direct_state_access
    {
        GLStateManager::active->BindBuffer(*this);
        glUnmapBuffer(GLTypes::Map(GetType()));
    }
}


} // /namespace LLGL



// ================================================================================
