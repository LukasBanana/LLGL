/*
 * GLBufferArrayWithVAO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_WITH_VAO_H
#define LLGL_GL_BUFFER_ARRAY_WITH_VAO_H


#include "GLBufferArray.h"
#include "GLSharedContextVertexArray.h"
#include "GLBufferInputLayout.h"


namespace LLGL
{


class GLBufferArrayWithVAO final : public GLBufferArray
{

    public:

        void SetDebugName(const char* name) override;

    public:

        GLBufferArrayWithVAO(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the vertex array which can be shared across multiple GL contexts.
        inline GLSharedContextVertexArray* GetVertexArray()
        {
            return &vertexArray_;
        }

        // Returns the GL buffer input layout with hash over all buffers.
        inline const GLBufferInputLayout& GetInputLayout() const
        {
            return bufferInputLayout_;
        }

    private:

        GLSharedContextVertexArray  vertexArray_; //DEPRECATED
        GLBufferInputLayout         bufferInputLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
