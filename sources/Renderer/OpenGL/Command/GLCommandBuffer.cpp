/*
 * GLCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCommandBuffer.h"
#include "../RenderState/GLState.h"


namespace LLGL
{


void GLCommandBuffer::SetIndexFormat(GLRenderState& renderState, bool indexType16Bits, std::uint64_t offset)
{
    /* Store new index buffer data in global render state */
    if (indexType16Bits)
    {
        renderState.indexBufferDataType = GL_UNSIGNED_SHORT;
        renderState.indexBufferStride   = 2;
    }
    else
    {
        renderState.indexBufferDataType = GL_UNSIGNED_INT;
        renderState.indexBufferStride   = 4;
    }
    renderState.indexBufferOffset = static_cast<GLsizeiptr>(offset);
}


} // /namespace LLGL



// ================================================================================
