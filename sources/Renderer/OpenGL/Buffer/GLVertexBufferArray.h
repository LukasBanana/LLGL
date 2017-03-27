/*
 * GLVertexBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_VERTEX_BUFFER_ARRAY_H
#define LLGL_GL_VERTEX_BUFFER_ARRAY_H


#include "GLBufferArray.h"
#include "GLVertexArrayObject.h"


namespace LLGL
{


class GLVertexBufferArray : public GLBufferArray
{

    public:

        GLVertexBufferArray();

        void BuildVertexArray(unsigned int numBuffers, Buffer* const * bufferArray);

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
