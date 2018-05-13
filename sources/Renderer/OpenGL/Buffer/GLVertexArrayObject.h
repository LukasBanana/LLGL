/*
 * GLVertexArrayObject.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_VERTEX_ARRAY_OBJECT_H
#define LLGL_GL_VERTEX_ARRAY_OBJECT_H


#include <LLGL/VertexFormat.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLVertexArrayObject
{

    public:

        GLVertexArrayObject();
        ~GLVertexArrayObject();

        void BuildVertexAttribute(const VertexAttribute& attribute, std::uint32_t stride, std::uint32_t index);

        //! Returns the ID of the hardware vertex-array-object (VAO)
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0; //!< Vertex array object ID.

};


} // /namespace LLGL


#endif



// ================================================================================
