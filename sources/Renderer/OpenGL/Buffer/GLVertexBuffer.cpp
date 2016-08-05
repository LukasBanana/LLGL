/*
 * GLVertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexBuffer.h"
#include "../GLExtensions.h"
#include "../GLTypeConversion.h"
#include "../GLStateManager.h"


namespace LLGL
{


GLVertexBuffer::GLVertexBuffer() :
    hwBuffer( GL_ARRAY_BUFFER )
{
    glGenVertexArrays(1, &vaoID_);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteVertexArrays(1, &vaoID_);
}

void GLVertexBuffer::UpdateVertexFormat(const VertexFormat& vertexFormat)
{
    /* Bind this vertex buffer to the vertex-array-object */
    GLStateManager::active->BindVertexArray(GetVaoID());
    GLStateManager::active->ForcedBindBuffer(GLBufferTarget::ARRAY_BUFFER, hwBuffer.GetID());

    /* Setup each vertex attribute */
    GLuint index = 0;

    for (const auto& attrib : vertexFormat.GetAttributes())
    {
        glEnableVertexAttribArray(index);

        glVertexAttribPointer(
            index,
            attrib.components,
            GLTypeConversion::Map(attrib.dataType),
            GL_FALSE,
            vertexFormat.GetFormatSize(),
            reinterpret_cast<const char*>(0) + attrib.offset
        );

        ++index;
    }

    GLStateManager::active->BindVertexArray(0);

    /* Store new vertex format */
    SetVertexFormat(vertexFormat);
}


} // /namespace LLGL



// ================================================================================
