/*
 * GLVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_VERTEX_BUFFER_H
#define LLGL_GL_VERTEX_BUFFER_H


#include "GLBuffer.h"
#include "GLVertexArrayObject.h"


namespace LLGL
{


class GLVertexBuffer final : public GLBuffer
{

    public:

        GLVertexBuffer();

        void BuildVertexArray(const VertexFormat& vertexFormat);

        //! Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vao_.GetID();
        }

        //! Returns the vertex format.
        inline const VertexFormat& GetVertexFormat() const
        {
            return vertexFormat_;
        }

    private:

        GLVertexArrayObject vao_;
        VertexFormat        vertexFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
