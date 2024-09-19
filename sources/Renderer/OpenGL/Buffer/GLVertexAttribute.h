/*
 * GLVertexAttribute.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_VERTEX_ATTRIBUTE_H
#define LLGL_GL_VERTEX_ATTRIBUTE_H


#include "../OpenGL.h"


namespace LLGL
{


struct VertexAttribute;

struct GLVertexAttribute
{
    GLuint      buffer;
    GLuint      index;
    GLint       size;
    GLenum      type;
    GLboolean   normalized;
    GLsizei     stride;
    GLsizeiptr  offsetPtrSized;
    GLuint      divisor; // for use with glVertexAttribDivisor()
    bool        isInteger; // meta data for use with glVertexAttribIPointer()
};

// Converts the specified vertex attribute into a GL specific attributes.
void GLConvertVertexAttrib(GLVertexAttribute& dst, const VertexAttribute& src, GLuint srcBuffer);


} // /namespace LLGL


#endif



// ================================================================================
