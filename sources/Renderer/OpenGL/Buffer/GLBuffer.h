/*
 * GLBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        void SetName(const char* name) override final;

        BufferDescriptor GetDesc() const override;

    public:

        GLBuffer(long bindFlags);
        ~GLBuffer();

        void BufferStorage(GLsizeiptr size, const void* data, GLbitfield flags, GLenum usage);
        void BufferSubData(GLintptr offset, GLsizeiptr size, const void* data);

        void ClearBufferData(std::uint32_t data);
        void ClearBufferSubData(GLintptr offset, GLsizeiptr size, std::uint32_t data);

        void CopyBufferSubData(const GLBuffer& readBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

        void* MapBuffer(GLenum access);
        void UnmapBuffer();

        // Returns the specified buffer parameters; null pointers are ignored.
        void GetBufferParams(GLint* size, GLint* usage, GLint* storageFlags) const;

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

    private:

        GLuint          id_                 = 0;
        GLBufferTarget  target_             = GLBufferTarget::ARRAY_BUFFER;
        bool            indexType16Bits_    = false;

};


} // /namespace LLGL


#endif



// ================================================================================
