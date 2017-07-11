/*
 * GLVertexBufferArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexBufferArray.h"
#include "GLVertexBuffer.h"
#include "../RenderState/GLStateManager.h"
#include "../../CheckedCast.h"


namespace LLGL
{


GLVertexBufferArray::GLVertexBufferArray() :
    GLBufferArray { BufferType::Vertex }
{
}

void GLVertexBufferArray::BuildVertexArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    /* Bind VAO */
    GLStateManager::active->BindVertexArray(GetVaoID());
    {
        for (unsigned int i = 0; numBuffers > 0; --numBuffers)
        {
            auto vertexBufferGL = LLGL_CAST(GLVertexBuffer*, (*bufferArray));
            {
                const auto& vertexFormat = vertexBufferGL->GetVertexFormat();

                /* Bind VBO */
                GLStateManager::active->BindBuffer(GLBufferTarget::ARRAY_BUFFER, vertexBufferGL->GetID());

                /* Build each vertex attribute */
                for (unsigned int j = 0, n = static_cast<unsigned int>(vertexFormat.attributes.size()); j < n; ++j, ++i)
                    vao_.BuildVertexAttribute(vertexFormat.attributes[j], vertexFormat.stride, i);
            }
            ++bufferArray;
        }
    }
    GLStateManager::active->BindVertexArray(0);
}


} // /namespace LLGL



// ================================================================================
