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

/*struct GLVertexBufferBinding
{
    GLBuffer*   buffer;
    GLintptr    offset;
};*/

// Stores an array of GL buffers and a hash over their pointers.
class GLBufferInputLayout
{

    public:

        GLBufferInputLayout() = default;

        // Initializes the hash with a single GL buffer.
        GLBufferInputLayout(GLBuffer* buffer);

        // Initializes the hash with an array of GL buffers.
        GLBufferInputLayout(ArrayView<GLBuffer*> buffers);

        // Reset the buffers.
        void Reset();

        // Appends the specified vertex attributes. Call Finalize() after all invocations of Append().
        void SetBuffers(ArrayView<GLBuffer*> buffers);

        // Returns the array of input vertex attributes this shader was created with. This is a direct copy of the input attributes.
        inline ArrayView<GLBuffer*> GetBuffers() const
        {
            return buffers_;
        }

    public:

        static int CompareSWO(const GLBufferInputLayout& lhs, const GLBufferInputLayout& rhs);

    private:

        SmallVector<GLBuffer*, 1> buffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
