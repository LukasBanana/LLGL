/*
 * GLVertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexBuffer.h"
#include "../Ext/GLExtensions.h"
#include "../GLTypes.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLVertexBuffer::GLVertexBuffer() :
    GLBuffer( BufferType::Vertex )
{
    glGenVertexArrays(1, &vaoID_);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteVertexArrays(1, &vaoID_);
}

void GLVertexBuffer::BuildVertexArray(const VertexFormat& vertexFormat)
{
    /* Bind this vertex buffer to the vertex-array-object */
    GLStateManager::active->BindVertexArray(GetVaoID());
    GLStateManager::active->ForcedBindBuffer(GLBufferTarget::ARRAY_BUFFER, GetID());

    /* Setup each vertex attribute */
    GLuint index = 0;

    for (const auto& attrib : vertexFormat.GetAttributes())
    {
        glEnableVertexAttribArray(index);

        if (!attrib.conversion && attrib.dataType != DataType::Float && attrib.dataType != DataType::Double)
        {
            //if (!glVertexAttribIPointer)
            //    throw std::runtime_error("integral vertex attributes not supported by renderer");

            glVertexAttribIPointer(
                index,
                attrib.components,
                GLTypes::Map(attrib.dataType),
                vertexFormat.GetFormatSize(),
                reinterpret_cast<const char*>(0) + attrib.offset
            );
        }
        else
        {
            glVertexAttribPointer(
                index,
                attrib.components,
                GLTypes::Map(attrib.dataType),
                GL_FALSE,
                vertexFormat.GetFormatSize(),
                reinterpret_cast<const char*>(0) + attrib.offset
            );
        }

        ++index;
    }

    GLStateManager::active->BindVertexArray(0);
}


} // /namespace LLGL



// ================================================================================
