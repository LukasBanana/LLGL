/*
 * GLBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBuffer.h"
#include "../GLProfile.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/Helper.h"
#include <memory>


namespace LLGL
{


// Finds the primary buffer target used for a buffer with the specified binding flags
static GLBufferTarget FindPrimaryBufferTarget(long bindFlags)
{
    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        return GLBufferTarget::ARRAY_BUFFER;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        return GLBufferTarget::ELEMENT_ARRAY_BUFFER;
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        return GLBufferTarget::UNIFORM_BUFFER;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        return GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER;
    if ((bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
        return GLBufferTarget::SHADER_STORAGE_BUFFER;
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        return GLBufferTarget::DRAW_INDIRECT_BUFFER;
    return GLBufferTarget::ARRAY_BUFFER;
}

GLBuffer::GLBuffer(long bindFlags) :
    Buffer  { bindFlags                          },
    target_ { FindPrimaryBufferTarget(bindFlags) }
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
    GLStateManager::Get().NotifyBufferRelease(*this);
}

void GLBuffer::SetName(const char* name)
{
    GLSetObjectLabel(GL_BUFFER, GetID(), name);
}

BufferDescriptor GLBuffer::GetDesc() const
{
    /* Get buffer parameters */
    GLint size = 0, usage = 0, storageFlags = 0;
    GetBufferParams(&size, &usage, &storageFlags);

    /* Convert to buffer descriptor */
    BufferDescriptor bufferDesc;
    bufferDesc.size         = static_cast<std::uint64_t>(size);
    bufferDesc.bindFlags    = GetBindFlags();

    #ifdef GL_ARB_buffer_storage
    if (HasExtension(GLExt::ARB_buffer_storage))
    {
        /* Convert buffer storage flags */
        if ((storageFlags & GL_MAP_READ_BIT) != 0)
            bufferDesc.cpuAccessFlags |= CPUAccessFlags::Read;
        if ((storageFlags & GL_MAP_WRITE_BIT) != 0)
            bufferDesc.cpuAccessFlags |= CPUAccessFlags::Write;
    }
    else
    #endif // /GL_ARB_buffer_storage
    {
        /* When the buffer was created with <glBufferData>, it can be used for CPU read/write access implicitly */
        bufferDesc.cpuAccessFlags |= CPUAccessFlags::ReadWrite;
    }

    if (usage == GL_DYNAMIC_DRAW)
        bufferDesc.miscFlags |= MiscFlags::DynamicUsage;

    return bufferDesc;
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
        GLStateManager::Get().BindGLBuffer(*this);
        glBufferStorage(GetGLTarget(), size, data, flags);
    }
    else
    #endif // /GL_ARB_buffer_storage
    {
        /* Bind and allocate buffer with mutable storage */
        GLStateManager::Get().BindGLBuffer(*this);
        glBufferData(GetGLTarget(), size, data, usage);
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
        GLStateManager::Get().BindGLBuffer(*this);
        glBufferSubData(GetGLTarget(), offset, size, data);
    }
}

void GLBuffer::ClearBufferData(std::uint32_t data)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glClearNamedBufferData(GetID(), GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
    }
    else
    #endif // /GL_ARB_direct_state_access
    #ifdef GL_ARB_clear_buffer_object
    if (HasExtension(GLExt::ARB_clear_buffer_object))
    {
        GLStateManager::Get().BindGLBuffer(*this);
        glClearBufferData(GetGLTarget(), GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
    }
    else
    #endif // /GL_ARB_clear_buffer_object
    {
        /* Emulate buffer fill operation */
        GLStateManager::Get().BindGLBuffer(*this);

        /* Query buffer size */
        GLenum  bufferTarget    = GetGLTarget();
        GLint   bufferSize      = 0;

        glGetBufferParameteriv(bufferTarget, GL_BUFFER_SIZE, &bufferSize);

        /* Allocate intermediate buffer to fill the GPU buffer with */
        std::vector<std::uint32_t> intermediateBuffer(static_cast<std::size_t>(bufferSize + 3) / 4, data);

        /* Submit intermeidate buffer to GPU buffer */
        glBufferSubData(bufferTarget, 0, static_cast<GLintptr>(bufferSize), intermediateBuffer.data());
    }
}

void GLBuffer::ClearBufferSubData(GLintptr offset, GLsizeiptr size, std::uint32_t data)
{
    #if 0 // TODO: does not work properly here with DSA version???
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glClearNamedBufferSubData(GetID(), GL_R32UI, offset, size, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
    }
    else
    #endif // /GL_ARB_direct_state_access
    #endif // /TODO
    #ifdef GL_ARB_clear_buffer_object
    if (HasExtension(GLExt::ARB_clear_buffer_object))
    {
        GLStateManager::Get().BindGLBuffer(*this);
        glClearBufferSubData(GetGLTarget(), GL_R32UI, offset, size, GL_RED_INTEGER, GL_UNSIGNED_INT, &data);
    }
    else
    #endif // /GL_ARB_clear_buffer_object
    {
        /* Emulate buffer fill operation */
        GLStateManager::Get().BindGLBuffer(*this);

        /* Allocate intermediate buffer to fill the GPU buffer with */
        std::vector<std::uint32_t> intermediateBuffer(static_cast<std::size_t>(size + 3) / 4, data);

        /* Submit intermeidate buffer to GPU buffer */
        glBufferSubData(GetGLTarget(), offset, size, intermediateBuffer.data());
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
        GLStateManager::Get().BindBuffer(GLBufferTarget::COPY_READ_BUFFER, readBuffer.GetID());
        GLStateManager::Get().BindBuffer(GLBufferTarget::COPY_WRITE_BUFFER, GetID());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, readOffset, writeOffset, size);
    }
    else
    #endif // /GL_ARB_copy_buffer
    {
        /* Emulate buffer copy operation */
        auto intermediateBuffer = MakeUniqueArray<char>(size);

        /* Read source buffer data */
        GLStateManager::Get().BindGLBuffer(readBuffer);
        GLProfile::GetBufferSubData(readBuffer.GetGLTarget(), readOffset, size, intermediateBuffer.get());

        /* Write destination buffer data */
        GLStateManager::Get().BindGLBuffer(*this);
        glBufferSubData(GetGLTarget(), writeOffset, size, intermediateBuffer.get());
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
        GLStateManager::Get().BindGLBuffer(*this);
        return GLProfile::MapBuffer(GetGLTarget(), access);
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
        GLStateManager::Get().BindGLBuffer(*this);
        glUnmapBuffer(GetGLTarget());
    }
}

void GLBuffer::GetBufferParams(GLint* size, GLint* usage, GLint* storageFlags) const
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query buffer attributes directly using DSA */
        if (size != nullptr)
            glGetNamedBufferParameteriv(GetID(), GL_BUFFER_SIZE, size);

        if (usage != nullptr)
            glGetNamedBufferParameteriv(GetID(), GL_BUFFER_USAGE, usage);

        if (storageFlags != nullptr)
            glGetNamedBufferParameteriv(GetID(), GL_BUFFER_STORAGE_FLAGS, storageFlags);
    }
    else
    #endif // /GL_ARB_direct_state_access
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::Get().PushBoundBuffer(GetTarget());
        {
            /* Bind buffer and query attributes */
            const GLenum bufferTarget = GetGLTarget();
            GLStateManager::Get().BindGLBuffer(*this);

            if (size != nullptr)
                glGetBufferParameteriv(bufferTarget, GL_BUFFER_SIZE, size);

            if (usage != nullptr)
                glGetBufferParameteriv(bufferTarget, GL_BUFFER_USAGE, usage);

            if (storageFlags != nullptr)
            {
                #ifdef GL_ARB_buffer_storage
                if (HasExtension(GLExt::ARB_buffer_storage))
                {
                    /* Query storage flags (GL_MAP_READ_BIT etc.) */
                    glGetBufferParameteriv(bufferTarget, GL_BUFFER_STORAGE_FLAGS, storageFlags);
                }
                else
                #endif
                {
                    /* Reset to output parameter */
                    *storageFlags = 0;
                }
            }
        }
        GLStateManager::Get().PopBoundBuffer();
    }
}

void GLBuffer::SetIndexType(const Format format)
{
    indexType16Bits_ = (format == Format::R16UInt);
}


} // /namespace LLGL



// ================================================================================
