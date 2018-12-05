/*
 * GLBufferWithVAO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(long bindFlags) :
    GLBuffer { bindFlags }
{
}

void GLBufferWithVAO::BuildVertexArray(const VertexFormat& vertexFormat)
{
    /* Bind VAO */
    GLStateManager::active->BindVertexArray(GetVaoID());
    {
        /* Bind VBO */
        GLStateManager::active->BindBuffer(GLBufferTarget::ARRAY_BUFFER, GetID());

        /* Build each vertex attribute */
        for (std::uint32_t i = 0, n = static_cast<std::uint32_t>(vertexFormat.attributes.size()); i < n; ++i)
            vao_.BuildVertexAttribute(vertexFormat.attributes[i], vertexFormat.stride, i);
    }
    GLStateManager::active->BindVertexArray(0);

    /* Store vertex format (required if this buffer is used in a buffer array) */
    vertexFormat_ = vertexFormat;
}


} // /namespace LLGL



// ================================================================================
