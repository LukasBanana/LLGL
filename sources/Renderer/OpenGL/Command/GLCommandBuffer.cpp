/*
 * GLCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCommandBuffer.h"
#include "../Buffer/GLBufferWithXFB.h"
#include "../Buffer/GLVertexArrayCache.h"
#include "../RenderState/GLState.h"
#include "../RenderState/GLPipelineLayout.h"
#include "../RenderState/GLPipelineState.h"
#include "../RenderState/GLGraphicsPSO.h"
#include "../Texture/GLTexture.h"
#include "../../CheckedCast.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


void GLCommandBuffer::ResetRenderState()
{
    /* Reset pointers to bound pipeline objects */
    renderState_.boundPipelineLayout    = nullptr;
    renderState_.boundPipelineState     = nullptr;
    renderState_.boundBufferWithFxb     = nullptr;

    /* Reset vertex input state to ensure VAOs are bound correctly */
    vertexInputState_.vertexInputLayout.Reset();
    vertexInputState_.bufferInputLayout.Reset();
}

void GLCommandBuffer::SetIndexFormat(bool indexType16Bits, std::uint64_t offset)
{
    /* Store new index buffer data in global render state */
    if (indexType16Bits)
    {
        renderState_.indexBufferDataType    = GL_UNSIGNED_SHORT;
        renderState_.indexBufferStride      = 2;
    }
    else
    {
        renderState_.indexBufferDataType    = GL_UNSIGNED_INT;
        renderState_.indexBufferStride      = 4;
    }
    renderState_.indexBufferOffset = static_cast<GLsizeiptr>(offset);
}

void GLCommandBuffer::SetPipelineRenderState(const GLPipelineState& pipelineStateGL)
{
    /* Store pipeline state and layout */
    renderState_.boundPipelineLayout    = pipelineStateGL.GetPipelineLayout();
    renderState_.boundPipelineState     = &pipelineStateGL;

    /* Store draw and primitive mode */
    if (pipelineStateGL.IsGraphicsPSO())
    {
        auto& graphicsPSO = LLGL_CAST(const GLGraphicsPSO&, pipelineStateGL);
        renderState_.drawMode       = graphicsPSO.GetDrawMode();
        renderState_.primitiveMode  = graphicsPSO.GetPrimitiveMode();
        SetVertexInputLayout(graphicsPSO.GetVertexInputLayout());
    }

    /* Store barrier flags; These must be invalidated when a new resource or resource-heap is set */
    renderState_.implicitBarriers   = pipelineStateGL.GetBarriersBitfield();
    renderState_.dirtyBarriers      = 0;
}

void GLCommandBuffer::SetTransformFeedback(GLBufferWithXFB& bufferWithXfbGL)
{
    renderState_.boundBufferWithFxb = &bufferWithXfbGL;
}

void GLCommandBuffer::SetTransformFeedbackChecked(GLBufferWithVAO& bufferWithVaoGL)
{
    /* Store ID to transform feedback object */
    if ((bufferWithVaoGL.GetBindFlags() & BindFlags::StreamOutputBuffer) != 0)
    {
        auto& streamOutputBufferGL = LLGL_CAST(GLBufferWithXFB&, bufferWithVaoGL);
        SetTransformFeedback(streamOutputBufferGL);
    }
}

void GLCommandBuffer::InvalidateMemoryBarriers(GLbitfield barriers)
{
    renderState_.dirtyBarriers |= (renderState_.implicitBarriers & barriers);
}

void GLCommandBuffer::InvalidateMemoryBarriersForStorageResource(long resourceBindFlags, GLbitfield barriers)
{
    if ((resourceBindFlags & BindFlags::Storage) != 0)
        InvalidateMemoryBarriers(barriers);
}

void GLCommandBuffer::InvalidateMemoryBarriersForResources(
    std::uint32_t       numBuffers,
    Buffer* const *     buffers,
    std::uint32_t       numTextures,
    Texture* const *    textures)
{
    #if LLGL_GLEXT_MEMORY_BARRIERS

    for_range(i, numBuffers)
    {
        if (buffers[i] != nullptr)
        {
            auto* bufferGL = LLGL_CAST(GLBuffer*, buffers[i]);
            if ((bufferGL->GetBindFlags() & BindFlags::Storage) != 0)
            {
                renderState_.dirtyBarriers |= GL_SHADER_STORAGE_BARRIER_BIT;
                if ((bufferGL->GetBindFlags() & BindFlags::VertexBuffer) != 0)
                    renderState_.dirtyBarriers |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
                if ((bufferGL->GetBindFlags() & BindFlags::IndexBuffer) != 0)
                    renderState_.dirtyBarriers |= GL_ELEMENT_ARRAY_BARRIER_BIT;
            }
        }
    }

    for_range(i, numTextures)
    {
        if (textures[i] != nullptr)
        {
            auto* textureGL = LLGL_CAST(GLTexture*, textures[i]);
            if ((textureGL->GetBindFlags() & BindFlags::Storage) != 0)
            {
                renderState_.dirtyBarriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
                if ((textureGL->GetBindFlags() & BindFlags::Sampled) != 0)
                    renderState_.dirtyBarriers |= GL_TEXTURE_FETCH_BARRIER_BIT;
            }
        }
    }

    #endif // /LLGL_GLEXT_MEMORY_BARRIERS
}

LLGL_NODISCARD
GLbitfield GLCommandBuffer::FlushAndGetMemoryBarriers()
{
    GLbitfield barriers = renderState_.dirtyBarriers;
    renderState_.dirtyBarriers &= renderState_.implicitBarriers; // Only keep implicit barriers
    return barriers;
}

void GLCommandBuffer::SetVertexInputLayout(const GLVertexInputLayout& vertexInputLayout)
{
    if (GLVertexInputLayout::CompareSWO(vertexInputState_.vertexInputLayout, vertexInputLayout) != 0)
    {
        vertexInputState_.dirtyBit          = true;
        vertexInputState_.vertexInputLayout = vertexInputLayout;
    }
}

void GLCommandBuffer::SetBufferInputLayout(const GLBufferInputLayout& bufferInputLayout)
{
    if (GLBufferInputLayout::CompareSWO(vertexInputState_.bufferInputLayout, bufferInputLayout) != 0)
    {
        vertexInputState_.dirtyBit          = true;
        vertexInputState_.bufferInputLayout = bufferInputLayout;
    }
}

GLSharedContextVertexArray* GLCommandBuffer::FlushVertexInput()
{
    if (vertexInputState_.dirtyBit)
    {
        vertexInputState_.dirtyBit = false;
        return GLVertexArrayCache::Get().FindOrMakeVertexArray(vertexInputState_.vertexInputLayout, vertexInputState_.bufferInputLayout);
    }
    return nullptr;
}

void GLCommandBuffer::SetVertexBufferInternal(Buffer& buffer, std::uint64_t offset)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferGL = LLGL_CAST(GLBufferWithVAO&, buffer);
        SetBufferInputLayout(GLBufferInputLayout{ &vertexBufferGL, static_cast<GLintptr>(offset) });

        #if LLGL_GLEXT_TRANSFORM_FEEDBACK2
        SetTransformFeedbackChecked(vertexBufferGL);
        #endif // /LLGL_GLEXT_TRANSFORM_FEEDBACK2
    }
}

void GLCommandBuffer::SetVertexBuffersInternal(std::uint32_t numBufferViews, const VertexBufferView* bufferViews)
{
    /* Translate input arguments to OpenGL buffer views */
    SmallVector<GLBufferView> bufferViewsGL{ numBufferViews, UninitializeTag{} };

    for_range(i, numBufferViews)
    {
        Buffer* buffer = bufferViews[i].buffer;
        if (!(buffer != nullptr && (buffer->GetBindFlags() & BindFlags::VertexBuffer) != 0))
            return; // Invalid argument

        auto* vertexBufferGL = LLGL_CAST(GLBufferWithVAO*, buffer);
        bufferViewsGL[i].buffer = vertexBufferGL;
        bufferViewsGL[i].offset = static_cast<GLintptr>(bufferViews[i].offset);
    }

    SetBufferInputLayout(GLBufferInputLayout{ bufferViewsGL });

    /* Bind first input buffer as transform-feedback if it's binding flags enabled it */
    #if LLGL_GLEXT_TRANSFORM_FEEDBACK2
    if (numBufferViews > 0 && (bufferViews[0].buffer->GetBindFlags() & BindFlags::StreamOutputBuffer) != 0)
    {
        GLBufferWithXFB* bufferWithXbf = LLGL_CAST(GLBufferWithXFB*, bufferViews[0].buffer);
        SetTransformFeedback(*bufferWithXbf);
    }
    #endif // /LLGL_GLEXT_TRANSFORM_FEEDBACK2
}

/* ----- Extensions ----- */

bool GLCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return (nativeHandle == nullptr || nativeHandleSize == 0); // dummy
}


} // /namespace LLGL



// ================================================================================
