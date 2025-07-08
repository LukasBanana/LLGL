/*
 * GLVertexInputLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_INPUT_LAYOUT_H
#define LLGL_GL_VERTEX_INPUT_LAYOUT_H


#include <LLGL/VertexAttribute.h>
#include "GLVertexArrayHash.h"
#include <vector>


namespace LLGL
{


// Helpers class to manage the vertex shader input layout.
class GLVertexInputLayout
{

    public:

        GLVertexInputLayout() = default;

        // Restes all vertex attributes and the hash.
        void Reset();

        // Appends the specified vertex attributes and the hash over all of them.
        void Append(const ArrayView<GLVertexAttribute>& attributes);
        void Append(GLuint bufferID, const ArrayView<VertexAttribute>& attributes);

        // Finalizes the input layout by updating the hash.
        void Finalize();

        // Returns the array of input vertex attributes this shader was created with. This is a direct copy of the input attributes.
        inline const std::vector<GLVertexAttribute>& GetAttribs() const
        {
            return attribs_;
        }

        // Returns the hash over all vertex attributes.
        inline std::size_t GetHash() const
        {
            return attribsHash_.Get();
        }

    private:

        std::vector<GLVertexAttribute>  attribs_;
        GLVertexArrayHash               attribsHash_;

};


} // /namespace LLGL


#endif



// ================================================================================
