/*
 * GLBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_H
#define LLGL_GL_BUFFER_H


#include <LLGL/Buffer.h>
#include <LLGL/Format.h>
#include "../OpenGL.h"
#include "../RenderState/GLStateManager.h"
#include <cstdint>


namespace LLGL
{


class GLBuffer : public Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        void SetDebugName(const char* name) override final;

    public:

        GLBuffer(long bindFlags, const char* debugName = nullptr);
        ~GLBuffer();

        void BufferStorage(GLsizeiptr size, const void* data, GLbitfield flags, GLenum usage);
        void BufferSubData(GLintptr offset, GLsizeiptr size, const void* data);

        void GetBufferSubData(GLintptr offset, GLsizeiptr size, void* data);

        void ClearBufferData(std::uint32_t data);
        void ClearBufferSubData(GLintptr offset, GLsizeiptr size, std::uint32_t data);

        void CopyBufferSubData(const GLBuffer& readBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

        void* MapBuffer(GLenum access);
        void* MapBufferRange(GLintptr offset, GLsizeiptr length, GLbitfield access);
        void UnmapBuffer();

        // Returns the specified buffer parameters; null pointers are ignored.
        void GetBufferParams(GLint* size, GLint* usage, GLint* storageFlags) const;

        // Creates the proxy texture for a sampler or image buffer (GL_TEXTURE_BUFFER).
        // If texture buffers are not supported, this function has no effect.
        // No error is reported, since platforms without sampler buffer support cannot make use of them in shaders anyway.
        void CreateTexBuffer(GLenum internalFormat);

        // Creates a proxy texture for a sampler or image buffer (GL_TEXTURE_BUFFER).
        // If 'texID' is non-zero, it will be reused. Otherwise, a new texture will be created.
        void CreateTexBufferRange(GLuint& texID, GLintptr offset, GLsizeiptr size) const;

        // Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns the primary buffer target. In case the buffer was created with multiple binding flags, other targets can be used, too.
        inline GLBufferTarget GetTarget() const
        {
            return target_;
        }

        // Returns the GL target type for the primary buffer target.
        inline GLenum GetGLTarget() const
        {
            return GLStateManager::ToGLBufferTarget(GetTarget());
        }

        // Sets the base data type of buffer entries. This is only used for a resource that can be bound as index buffer.
        void SetIndexType(const Format format);

        // Returns the base data type of buffer entries. This is only used for a resource that can be bound as index buffer.
        inline bool IsIndexType16Bits() const
        {
            return indexType16Bits_;
        }

        // Returns the hardware texture ID if this buffer represents a sampler or image buffer. Otherwise, returns 0.
        // This texture gets its data from the buffer and can be accessed in GLSL via a 'samplerBuffer' type.
        inline GLuint GetTexID() const
        {
            return texID_;
        }

        // Returns the internal GL format of the proxy texture when this buffer represents a sampler or image buffer.
        inline GLenum GetTexGLInternalFormat() const
        {
            return texInternalFormat_;
        }

    private:

        GLuint          id_                 = 0;
        GLBufferTarget  target_             = GLBufferTarget::ArrayBuffer;
        bool            indexType16Bits_    = false;
        GLuint          texID_              = 0; // Used for sampler and image buffers
        GLenum          texInternalFormat_  = 0; // Used for sampler and image buffers

};


} // /namespace LLGL


#endif



// ================================================================================
