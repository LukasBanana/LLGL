/*
 * GLStagingBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_STAGING_BUFFER_POOL_H
#define LLGL_GL_STAGING_BUFFER_POOL_H


#include "../OpenGL.h"

#if LLGL_GL_BUFFER_HAZARD_TRACKING


#include <vector>


namespace LLGL
{


class GLBuffer;
class GLStateManager;

/*
Pool to allocate staging buffers.
This is only used as a workaround for a bug in the WebGL implementation "ANGLE" (see LLGL_GL_BUFFER_HAZARD_TRACKING).
*/
class GLStagingBufferPool
{

    public:

        GLStagingBufferPool();

        GLStagingBufferPool(const GLStagingBufferPool&) = delete;
        GLStagingBufferPool& operator = (const GLStagingBufferPool&) = delete;

        // Updates the specified GLBuffer with a pooled staging buffer.
        void BufferSubData(GLStateManager& stateMngr, GLBuffer& buffer, GLintptr offset, GLsizeiptr size, const void* data);

        // Releases all staging buffers.
        // This must only be called when the GL context is about to be deleted since this function doesn't notify the GLStateManager about relased buffers.
        void Clear();

        // Resets all staging buffer offsets to be ready for the next frame.
        void Reset();

    private:

        struct GLStagingBuffer
        {
            GLuint      id          = 0;
            GLsizeiptr  size        = 0;
            GLsizeiptr  capacity    = 0;

            inline bool Fits(GLsizeiptr size) const
            {
                return (this->size + size <= this->capacity);
            }
        };

    private:

        GLsizeiptr GetNextCapacity(GLsizeiptr minSize) const;

        GLStagingBuffer* GetOrCreateStagingBufferAndBind(GLStateManager& stateMngr, GLsizeiptr size);

    private:

        std::vector<GLStagingBuffer>    stagingBuffers_;
        std::size_t                     currentStagingBuffer_   = 0;
        GLsizeiptr                      nextCapacity_           = 0;

};


} // /namespace LLGL


#endif // /LLGL_GL_BUFFER_HAZARD_TRACKING

#endif



// ================================================================================
