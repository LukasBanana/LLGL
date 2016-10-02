/*
 * GLVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_VERTEX_BUFFER_H__
#define __LLGL_GL_VERTEX_BUFFER_H__


#include "GLBuffer.h"
#include "GLVertexArrayObject.h"


namespace LLGL
{


class GLVertexBuffer : public GLBuffer
{

    public:

        GLVertexBuffer();

        void BuildVertexArray(const VertexFormat& vertexFormat);

        //! Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vao_.GetID();
        }

    private:

        GLVertexArrayObject vao_;

};


} // /namespace LLGL


#endif



// ================================================================================
