/*
 * GLVertexArrayHash.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_ARRAY_HASH_H
#define LLGL_GL_VERTEX_ARRAY_HASH_H


#include <LLGL/Container/ArrayView.h>
#include "GLVertexAttribute.h"


namespace LLGL
{


// Helper class to compute the hash over a vector of vertex attributes.
class GLVertexArrayHash
{

    public:

        GLVertexArrayHash() = default;

        // Initializes the hash with the specified attributes.
        GLVertexArrayHash(const ArrayView<GLVertexAttribute>& attributes);

        // Resets the hash.
        void Reset();

        // Updates the hash using the specified vertex attributes.
        void Update(const ArrayView<GLVertexAttribute>& attributes);

        // Returns the ID of the hardware vertex-array-object (VAO)
        inline std::size_t Get() const
        {
            return hash_;
        }

    private:

        std::size_t hash_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
