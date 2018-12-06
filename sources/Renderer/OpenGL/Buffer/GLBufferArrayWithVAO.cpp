/*
 * GLBufferArrayWithVAO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferArrayWithVAO.h"
#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../../CheckedCast.h"


namespace LLGL
{


GLBufferArrayWithVAO::GLBufferArrayWithVAO(long bindFlags) :
    GLBufferArray { bindFlags }
{
}

void GLBufferArrayWithVAO::BuildVertexArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    /* Bind VAO */
    GLStateManager::active->BindVertexArray(GetVaoID());
    {
        for (std::uint32_t i = 0; numBuffers > 0; --numBuffers)
        {
            if (((*bufferArray)->GetBindFlags() & BindFlags::VertexBuffer) != 0)
            {
                auto vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, (*bufferArray));
                {
                    const auto& vertexFormat = vertexBufferGL->GetVertexFormat();

                    /* Bind VBO */
                    GLStateManager::active->BindBuffer(GLBufferTarget::ARRAY_BUFFER, vertexBufferGL->GetID());

                    /* Build each vertex attribute */
                    for (std::uint32_t j = 0, n = static_cast<std::uint32_t>(vertexFormat.attributes.size()); j < n; ++j, ++i)
                        vao_.BuildVertexAttribute(vertexFormat.attributes[j], vertexFormat.stride, i);
                }
                ++bufferArray;
            }
            else
                throw std::invalid_argument("cannot build vertex array with buffer that was not created with the 'LLGL::BindFlags::VertexBuffer' flag");
        }
    }
    GLStateManager::active->BindVertexArray(0);
}


} // /namespace LLGL



// ================================================================================
