/*
 * GLStagingBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLStagingBufferPool.h"

#if LLGL_GL_BUFFER_HAZARD_TRACKING


#include "GLBuffer.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static constexpr GLsizeiptr k_minGLStagingBufferCapacity = 1024;

GLStagingBufferPool::GLStagingBufferPool() :
    nextCapacity_ { k_minGLStagingBufferCapacity }
{
}

void GLStagingBufferPool::Clear()
{
    for (const GLStagingBuffer& stagingBuffer : stagingBuffers_)
        glDeleteBuffers(1, &(stagingBuffer.id));
}

void GLStagingBufferPool::BufferSubData(GLStateManager& stateMngr, GLBuffer& buffer, GLintptr offset, GLsizeiptr size, const void* data)
{
    /* Find staging buffer for size */
    GLStagingBuffer* stagingBuffer = GetOrCreateStagingBufferAndBind(stateMngr, size);

    /* Write CPU data to staging buffer and advance its offset */
    const GLintptr stagingBufferOffset = static_cast<GLintptr>(stagingBuffer->size);
    glBufferSubData(GL_COPY_READ_BUFFER, stagingBufferOffset, size, data);
    stagingBuffer->size += size;

    /* Copy staging buffer range to destination GPU buffer */
    stateMngr.BindBuffer(GLBufferTarget::CopyWriteBuffer, buffer.GetID());
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, stagingBufferOffset, offset, size);
}

void GLStagingBufferPool::Reset()
{
    for_range(i, std::min(stagingBuffers_.size(), currentStagingBuffer_ + 1))
        stagingBuffers_[i].size = 0;
    currentStagingBuffer_ = 0;
}


/*
 * ======= Private: =======
 */

static GLsizeiptr GrowStagingBufferCapacity(GLsizeiptr capacity)
{
    return (capacity + capacity/2);
}

GLsizeiptr GLStagingBufferPool::GetNextCapacity(GLsizeiptr minSize) const
{
    return std::max(
        minSize,
        stagingBuffers_.empty()
            ? k_minGLStagingBufferCapacity
            : GrowStagingBufferCapacity(stagingBuffers_.back().capacity)
    );
}

GLStagingBufferPool::GLStagingBuffer* GLStagingBufferPool::GetOrCreateStagingBufferAndBind(GLStateManager& stateMngr, GLsizeiptr size)
{
    /* Try to find existing buffer that can still fit the requested data size */
    while (currentStagingBuffer_ < stagingBuffers_.size() && !stagingBuffers_[currentStagingBuffer_].Fits(size))
        ++currentStagingBuffer_;

    if (currentStagingBuffer_ < stagingBuffers_.size())
    {
        /* Only bind buffer */
        stateMngr.BindBuffer(GLBufferTarget::CopyReadBuffer, stagingBuffers_[currentStagingBuffer_].id);
    }
    else
    {
        /* Allocate new staging buffer */
        GLStagingBuffer newStagingBuffer;
        {
            newStagingBuffer.capacity = GetNextCapacity(size);

            GLBuffer::CreateNativeGLBuffers(1, &(newStagingBuffer.id));
            stateMngr.BindBuffer(GLBufferTarget::CopyReadBuffer, newStagingBuffer.id);

            #if GL_ARB_buffer_storage
            if (HasExtension(GLExt::ARB_buffer_storage))
            {
                /* Allocate buffer with immutable storage (GL 4.4+) */
                glBufferStorage(GL_COPY_READ_BUFFER, newStagingBuffer.capacity, nullptr, GL_DYNAMIC_STORAGE_BIT);
            }
            else
            #endif // /GL_ARB_buffer_storage
            {
                /* Allocate buffer with mutable storage */
                glBufferData(GL_COPY_READ_BUFFER, newStagingBuffer.capacity, nullptr, GL_DYNAMIC_COPY);
            }
        }
        stagingBuffers_.push_back(newStagingBuffer);
    }

    return &(stagingBuffers_[currentStagingBuffer_]);
}


} // /namespace LLGL


#endif // /LLGL_GL_BUFFER_HAZARD_TRACKING



// ================================================================================
