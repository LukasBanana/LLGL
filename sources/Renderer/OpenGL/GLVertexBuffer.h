/*
 * GLVertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_VERTEX_BUFFER_H__
#define __LLGL_GL_VERTEX_BUFFER_H__


#include <LLGL/VertexBuffer.h>
#include "GLHardwareBuffer.h"


namespace LLGL
{


class GLVertexBuffer : public VertexBuffer
{

    public:

        GLVertexBuffer();
        ~GLVertexBuffer();

        void UpdateVertexFormat(const VertexFormat& vertexFormat);

        //! Returns the ID of the vertex-array-object (VAO)
        inline GLuint GetVaoID() const
        {
            return vaoID_;
        }

        GLHardwareBuffer hwBuffer;

    private:

        GLuint vaoID_ = 0; //!< Vertex array object ID.

};


} // /namespace LLGL


#endif



// ================================================================================
