/*
 * GLBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

        GLBuffer(long bindFlags);
        ~GLBuffer();

        void BufferStorage(GLsizeiptr size, const void* data, GLbitfield flags, GLenum usage);
        void BufferSubData(GLintptr offset, GLsizeiptr size, const void* data);

        void ClearBufferData(std::uint32_t data);
        void ClearBufferSubData(GLintptr offset, GLsizeiptr size, std::uint32_t data);

        void CopyBufferSubData(const GLBuffer& readBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

        void* MapBuffer(GLenum access);
        void UnmapBuffer();

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
        void SetDataType(const DataType dataType)
        {
            dataType_ = dataType;
        }

        // Returns the base data type of buffer entries. This is only used for a resource that can be bound as index buffer.
        inline DataType GetDataType() const
        {
            return dataType_;
        }

    private:

        GLuint          id_         = 0;
        GLBufferTarget  target_     = GLBufferTarget::ARRAY_BUFFER;
        DataType        dataType_   = DataType::Int8;

};


} // /namespace LLGL


#endif



// ================================================================================
