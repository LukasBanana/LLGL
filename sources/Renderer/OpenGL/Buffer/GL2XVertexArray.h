/*
 * GL2XVertexArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL2X_VERTEX_ARRAY_H
#define LLGL_GL2X_VERTEX_ARRAY_H


#include <LLGL/VertexFormat.h>
#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


class GLStateManager;

// This class emulates the Vertex-Array-Object (VAO) functionality, for GL 2.x.
class GL2XVertexArray
{

    public:

        // Builds the specified attribute using a 'glVertexAttrib*Pointer' function.
        void BuildVertexAttribute(GLuint bufferID, const VertexAttribute& attribute, GLsizei stride, GLuint index);

        // Binds this vertex array.
        void Bind(GLStateManager& stateMngr) const;

    private:

        struct GL2XVertexAttrib
        {
            GLuint          buffer;
            GLuint          index;
            GLint           size;
            GLenum          type;
            GLboolean       normalized;
            GLsizei         stride;
            const GLvoid*   pointer;
        };

    private:

        std::vector<GL2XVertexAttrib> attribs_;

};


} // /namespace LLGL


#endif



// ================================================================================
