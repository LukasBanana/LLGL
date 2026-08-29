/*
 * GLBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_H
#define LLGL_GL_BUFFER_ARRAY_H


#include "../OpenGL.h"
#include <LLGL/BufferArray.h>
#include <LLGL/Container/ArrayView.h>
#include "GLBufferInputLayout.h"


namespace LLGL
{


class Buffer;
class GLBufferWithXFB;

// GL implementation of BufferArray interface for vertex buffer input layouts.
class GLBufferArray : public BufferArray
{

    public:

        GLBufferArray(ArrayView<VertexBufferView> bufferViews);

        // Returns the GL buffer input layout with hash over all buffers.
        inline const GLBufferInputLayout& GetInputLayout() const
        {
            return bufferInputLayout_;
        }

        // Returns a pointer to a transform-feedback buffer in slot 0, if the buffer array was created with one. Otherwise, null.
        inline GLBufferWithXFB* GetBufferSlot0WithXFB() const
        {
            return bufferSlot0WithXFB_;
        }

    private:

        GLBufferInputLayout bufferInputLayout_;
        GLBufferWithXFB*    bufferSlot0WithXFB_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
