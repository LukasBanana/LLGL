/*
 * GLBufferInputLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_HASH_H
#define LLGL_GL_BUFFER_ARRAY_HASH_H


#include "../OpenGL.h"
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <vector>


namespace LLGL
{


class GLBuffer;

struct GLBufferView
{
    GLBuffer*   buffer;
    GLintptr    offset;
};

// Stores an array of GL buffers and a hash over their pointers.
class GLBufferInputLayout
{

    public:

        GLBufferInputLayout() = default;

        // Initializes the layout with a single GL buffer and optional base offset.
        GLBufferInputLayout(GLBuffer* buffer, GLintptr offset = 0);

        // Initializes the layout with an array of GL buffers and puts all offsets to 0.
        GLBufferInputLayout(ArrayView<GLBuffer*> buffers);

        // Initializes the layout with an array of GL buffer views.
        GLBufferInputLayout(ArrayView<GLBufferView> bufferViews);

        // Reset the buffers.
        void Reset();

        // Appends the specified vertex attributes. Call Finalize() after all invocations of Append().
        void SetBufferViews(ArrayView<GLBufferView> bufferViews);

        // Returns the array of input vertex attributes this shader was created with. This is a direct copy of the input attributes.
        inline ArrayView<GLBufferView> GetBufferViews() const
        {
            return bufferViews_;
        }

    public:

        static int CompareSWO(const GLBufferInputLayout& lhs, const GLBufferInputLayout& rhs);

    private:

        SmallVector<GLBufferView, 1> bufferViews_;

};


} // /namespace LLGL


#endif



// ================================================================================
