/*
 * GLVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_VERTEX_BUFFER_H__
#define __LLGL_GL_VERTEX_BUFFER_H__


#include "GLBuffer.h"


namespace LLGL
{


class GLVertexBuffer_ : public GLBuffer
{

    public:

        GLVertexBuffer_();
        ~GLVertexBuffer_();

        void BuildVertexArray(const VertexFormat& vertexFormat);

        //! Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vaoID_;
        }

    private:

        GLuint vaoID_ = 0; //!< Vertex array object ID.

};


} // /namespace LLGL


#endif



// ================================================================================
