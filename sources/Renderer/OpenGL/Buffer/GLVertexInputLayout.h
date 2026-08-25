/*
 * GLVertexInputLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_INPUT_LAYOUT_H
#define LLGL_GL_VERTEX_INPUT_LAYOUT_H


#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include "GLVertexAttribute.h"
#include <vector>


namespace LLGL
{


// Helpers class to manage the vertex shader input layout.
class GLVertexInputLayout
{

    public:

        GLVertexInputLayout() = default;

        // Resets all vertex attributes and the hash.
        void Reset();

        // Appends the specified vertex attributes. Call Finalize() after all invocations of Append().
        void SetAttribs(ArrayView<GLVertexAttribute> attributes);
        void SetAttribs(ArrayView<VertexAttribute> attributes);

        // Returns the array of input vertex attributes this shader was created with. This is a direct copy of the input attributes.
        inline ArrayView<GLVertexAttribute> GetAttribs() const
        {
            return attribs_;
        }

    public:

        static int CompareSWO(const GLVertexInputLayout& lhs, const GLVertexInputLayout& rhs);

    private:

        std::vector<GLVertexAttribute> attribs_;

};


} // /namespace LLGL


#endif



// ================================================================================
