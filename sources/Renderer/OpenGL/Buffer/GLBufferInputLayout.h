/*
 * GLBufferInputLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_ARRAY_HASH_H
#define LLGL_GL_BUFFER_ARRAY_HASH_H


#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <vector>


namespace LLGL
{


class GLBuffer;

// Stores an array of GL buffers and a hash over their pointers.
class GLBufferInputLayout
{

    public:

        GLBufferInputLayout() = default;

        // Initializes the hash with a single GL buffer.
        GLBufferInputLayout(GLBuffer* buffer);

        // Initializes the hash with an array of GL buffers.
        GLBufferInputLayout(ArrayView<GLBuffer*> buffers);

        // Resets the hash.
        void Reset();

        // Appends the specified vertex attributes. Call Finalize() after all invocations of Append().
        void Append(ArrayView<GLBuffer*> buffers);

        // Finalizes the input layout by updating the hash.
        void Finalize();

        // Returns the array of input vertex attributes this shader was created with. This is a direct copy of the input attributes.
        inline ArrayView<GLBuffer*> GetBuffers() const
        {
            return buffers_;
        }

        // Returns the hash over all vertex attributes.
        inline std::size_t GetHash() const
        {
            return hash_;
        }

    private:

        SmallVector<GLBuffer*, 1>   buffers_;
        std::size_t                 hash_       = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
