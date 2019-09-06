/*
 * GLVertexArrayObject.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_VERTEX_ARRAY_OBJECT_H
#define LLGL_GL_VERTEX_ARRAY_OBJECT_H


#include "../OpenGL.h"


namespace LLGL
{


struct VertexAttribute;

// Wrapper class for an OpenGL Vertex-Array-Object (VAO), for GL 3.0+.
class GLVertexArrayObject
{

    public:

        GLVertexArrayObject();
        ~GLVertexArrayObject();

        // Builds the specified attribute using a 'glVertexAttrib*Pointer' function.
        void BuildVertexAttribute(const VertexAttribute& attribute);

        // Returns the ID of the hardware vertex-array-object (VAO)
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
