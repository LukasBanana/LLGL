/*
 * GLBufferWithVAO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_BUFFER_WITH_VAO_H
#define LLGL_GL_BUFFER_WITH_VAO_H


#include "GLBuffer.h"
#include "GLVertexArrayObject.h"
#include "GLSharedContextVertexArray.h"
#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


class GLBufferWithVAO : public GLBuffer
{

    public:

        GLBufferWithVAO(const BufferDescriptor& bufferDesc);

        void BuildVertexArray(const ArrayView<GLVertexAttribute>& vertexAttribs);
        void BuildVertexArray(const ArrayView<VertexAttribute>& vertexAttribs);

        // Returns the list of vertex attributes.
        inline const std::vector<GLVertexAttribute>& GetVertexAttribs() const
        {
            return vertexAttribs_;
        }

        // Returns the vertex array which can be shared across multiple GL contexts.
        inline GLSharedContextVertexArray* GetVertexArray()
        {
            return &vertexArray_;
        }

    private:

        std::vector<GLVertexAttribute>  vertexAttribs_;
        GLSharedContextVertexArray      vertexArray_;

};


} // /namespace LLGL


#endif



// ================================================================================
