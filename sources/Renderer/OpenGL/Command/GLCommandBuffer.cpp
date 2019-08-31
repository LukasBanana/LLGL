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


void GLCommandBuffer::SetIndexFormat(GLRenderState& renderState, bool index16Bits, std::uint64_t offset)
{
    /* Store new index buffer data in global render state */
    if (index16Bits)
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


/*
 * ======= Protected: =======
 */

void GLCommandBuffer::InitializeGLRenderState(GLRenderState& dst)
{
    dst.drawMode            = GL_TRIANGLES;
    dst.indexBufferDataType = GL_UNSIGNED_INT;
    dst.indexBufferStride   = 4;
    dst.indexBufferOffset   = 0;
}

void GLCommandBuffer::InitializeGLClearValue(GLClearValue& dst)
{
    dst.color[0]    = 0.0f;
    dst.color[1]    = 0.0f;
    dst.color[2]    = 0.0f;
    dst.color[3]    = 0.0f;
    dst.depth       = 1.0f;
    dst.stencil     = 0;
}


} // /namespace LLGL



// ================================================================================
