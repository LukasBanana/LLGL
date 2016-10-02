/*
 * GLVertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexBuffer.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"


namespace LLGL
{


GLVertexBuffer::GLVertexBuffer() :
    GLBuffer( BufferType::Vertex )
{
}

void GLVertexBuffer::BuildVertexArray(const VertexFormat& vertexFormat)
{
    /* Bind VAO */
    GLStateManager::active->BindVertexArray(GetVaoID());
    {
        /* Bind VBO */
        GLStateManager::active->BindBuffer(GLBufferTarget::ARRAY_BUFFER, GetID());

        /* Build each vertex attribute */
        for (unsigned int i = 0, n = static_cast<unsigned int>(vertexFormat.GetAttributes().size()); i < n; ++i)
            vao_.BuildVertexAttribute(vertexFormat, i);
    }
    GLStateManager::active->BindVertexArray(0);
}


} // /namespace LLGL



// ================================================================================
