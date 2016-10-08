/*
 * GLCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCommandBuffer.h"
#include "GLRenderContext.h"
#include "GLTypes.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionLoader.h"
#include "../Assertion.h"
#include "../CheckedCast.h"

#include "Shader/GLShaderProgram.h"
#include "Texture/GLTexture.h"
#include "Texture/GLTextureArray.h"
#include "Texture/GLRenderTarget.h"
#include "Texture/GLSampler.h"
#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLVertexBufferArray.h"
#include "RenderState/GLStateManager.h"
#include "RenderState/GLGraphicsPipeline.h"
#include "RenderState/GLComputePipeline.h"
#include "RenderState/GLQuery.h"


namespace LLGL
{


GLCommandBuffer::GLCommandBuffer(const std::shared_ptr<GLStateManager>& stateMngr) :
    stateMngr_( stateMngr )
{
}

/* ----- Configuration ----- */

void GLCommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    stateMngr_->SetGraphicsAPIDependentState(state);
}

void GLCommandBuffer::SetViewport(const Viewport& viewport)
{
    /* Setup GL viewport and depth-range */
    GLViewport viewportGL { viewport.x, viewport.y, viewport.width, viewport.height };
    GLDepthRange depthRangeGL { viewport.minDepth, viewport.maxDepth };

    /* Set final state */
    stateMngr_->SetViewport(viewportGL);
    stateMngr_->SetDepthRange(depthRangeGL);
}

void GLCommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    /* Setup GL viewports and depth-ranges */
    std::vector<GLViewport> viewportsGL;
    viewportsGL.reserve(numViewports);

    std::vector<GLDepthRange> depthRangesGL;
    depthRangesGL.reserve(numViewports);

    for (unsigned int i = 0; i < numViewports; ++i)
    {
        const auto& vp = viewportArray[i];
        viewportsGL.push_back({ vp.x, vp.y, vp.width, vp.height });
        depthRangesGL.push_back({ static_cast<GLdouble>(vp.minDepth), static_cast<GLdouble>(vp.maxDepth) });
    }

    /* Submit viewports and depth-ranges to state manager */
    stateMngr_->SetViewportArray(std::move(viewportsGL));
    stateMngr_->SetDepthRangeArray(std::move(depthRangesGL));
}

void GLCommandBuffer::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL { scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLCommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
{
    /* Setup GL scissors */
    std::vector<GLScissor> scissorsGL;
    scissorsGL.reserve(numScissors);

    for (unsigned int i = 0; i < numScissors; ++i)
    {
        const auto& sc = scissorArray[i];
        scissorsGL.push_back({ sc.x, sc.y, sc.width, sc.height });
    }

    /* Submit scissors to state manager */
    stateMngr_->SetScissorArray(std::move(scissorsGL));
}

void GLCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void GLCommandBuffer::SetClearDepth(float depth)
{
    glClearDepth(depth);
}

void GLCommandBuffer::SetClearStencil(int stencil)
{
    glClearStencil(stencil);
}

void GLCommandBuffer::ClearBuffers(long flags)
{
    GLbitfield mask = 0;

    if ((flags & ClearBuffersFlags::Color) != 0)
    {
        //stateMngr_->SetColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    
    if ((flags & ClearBuffersFlags::Depth) != 0)
    {
        stateMngr_->SetDepthMask(GL_TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearBuffersFlags::Stencil) != 0)
    {
        //stateMngr_->SetStencilMask(GL_TRUE);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

/* ----- Hardware Buffers ------ */

void GLCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, buffer);
    stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
}

void GLCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    /* Bind vertex buffer */
    auto& vertexBufferArrayGL = LLGL_CAST(GLVertexBufferArray&, bufferArray);
    stateMngr_->BindVertexArray(vertexBufferArrayGL.GetVaoID());
}

void GLCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, buffer);
    stateMngr_->DeferredBindIndexBuffer(indexBufferGL.GetID());

    /* Store new index buffer data in global render state */
    const auto& format = indexBufferGL.GetIndexFormat();
    renderState_.indexBufferDataType    = GLTypes::Map(format.GetDataType());
    renderState_.indexBufferStride      = format.GetFormatSize();
}

void GLCommandBuffer::SetConstantBuffer(Buffer& buffer, unsigned int slot, long /*shaderStageFlags*/)
{
    /* Bind constant buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(GLBufferTarget::UNIFORM_BUFFER, slot, bufferGL.GetID());
}

void GLCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long /*shaderStageFlags*/)
{
    /* Bind constant buffers with BindBuffersBase */
    auto& bufferArrayGL = LLGL_CAST(GLBufferArray&, bufferArray);
    stateMngr_->BindBuffersBase(
        GLBufferTarget::UNIFORM_BUFFER,
        startSlot,
        static_cast<GLsizei>(bufferArrayGL.GetIDArray().size()),
        bufferArrayGL.GetIDArray().data()
    );
}

void GLCommandBuffer::SetStorageBuffer(Buffer& buffer, unsigned int slot, long /*shaderStageFlags*/)
{
    /* Bind storage buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(GLBufferTarget::SHADER_STORAGE_BUFFER, slot, bufferGL.GetID());
}

/* ----- Textures ----- */

void GLCommandBuffer::SetTexture(Texture& texture, unsigned int slot, long /*shaderStageFlags*/)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

void GLCommandBuffer::SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long /*shaderStageFlags*/)
{
    /* Bind texture array to layers */
    auto& textureArrayGL = LLGL_CAST(GLTextureArray&, textureArray);
    stateMngr_->BindTextures(
        startSlot,
        static_cast<GLsizei>(textureArrayGL.GetIDArray().size()),
        textureArrayGL.GetTargetArray().data(),
        textureArrayGL.GetIDArray().data()
    );
}

/* ----- Sampler States ----- */

void GLCommandBuffer::SetSampler(Sampler& sampler, unsigned int slot, long /*shaderStageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

/* ----- Render Targets ----- */

//private
void GLCommandBuffer::BlitBoundRenderTarget()
{
    if (boundRenderTarget_)
        boundRenderTarget_->BlitOntoFrameBuffer();
}

void GLCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    /* Blit previously bound render target (in case mutli-sampling is used) */
    BlitBoundRenderTarget();

    /* Bind framebuffer object */
    auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFrameBuffer().GetID());

    /* Notify state manager about new render target height */
    stateMngr_->NotifyRenderTargetHeight(renderTarget.GetResolution().y);

    /* Store current render target */
    boundRenderTarget_ = &renderTargetGL;
}

void GLCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextGL = LLGL_CAST(GLRenderContext&, renderContext);

    /* Blit previously bound render target (in case mutli-sampling is used) */
    BlitBoundRenderTarget();

    /* Unbind framebuffer object */
    stateMngr_->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);

    /*
    Ensure the specified render context is the active one,
    and notify the state manager about new render target (the default frame buffer) height
    */
    GLRenderContext::GLMakeCurrent(&renderContextGL);

    /* Reset reference to render target */
    boundRenderTarget_ = nullptr;
}

/* ----- Pipeline States ----- */

void GLCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    renderState_.drawMode = graphicsPipelineGL.GetDrawMode();
    graphicsPipelineGL.Bind(*stateMngr_);
}

void GLCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineGL = LLGL_CAST(GLComputePipeline&, computePipeline);
    computePipelineGL.Bind(*stateMngr_);
}

/* ----- Queries ----- */

void GLCommandBuffer::BeginQuery(Query& query)
{
    /* Begin query with internal target */
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glBeginQuery(queryGL.GetTarget(), queryGL.GetID());
}

void GLCommandBuffer::EndQuery(Query& query)
{
    /* Begin query with internal target */
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glEndQuery(queryGL.GetTarget());
}

bool GLCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);

    /* Check if query result is available */
    GLint available = 0;
    glGetQueryObjectiv(queryGL.GetID(), GL_QUERY_RESULT_AVAILABLE, &available);

    if (available != GL_FALSE)
    {
        if (HasExtension(GLExt::ARB_timer_query))
        {
            /* Get query result with 64-bit version */
            glGetQueryObjectui64v(queryGL.GetID(), GL_QUERY_RESULT, &result);
        }
        else
        {
            /* Get query result with 32-bit version and convert to 64-bit */
            GLuint result32 = 0;
            glGetQueryObjectuiv(queryGL.GetID(), GL_QUERY_RESULT, &result32);
            result = result32;
        }
        return true;
    }

    return false;
}

void GLCommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glBeginConditionalRender(queryGL.GetID(), GLTypes::Map(mode));
}

void GLCommandBuffer::EndRenderCondition()
{
    glEndConditionalRender();
}

/* ----- Drawing ----- */

void GLCommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride))
    );
}

void GLCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        vertexOffset
    );
}

void GLCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    glDrawArraysInstanced(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    #ifndef __APPLE__
    glDrawArraysInstancedBaseInstance(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        instanceOffset
    );
    #endif
}

void GLCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances)
    );
}

void GLCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
}

void GLCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    #ifndef __APPLE__
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride)),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        instanceOffset
    );
    #endif
}

/* ----- Compute ----- */

void GLCommandBuffer::DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    #ifndef __APPLE__
    glDispatchCompute(groupSizeX, groupSizeY, groupSizeZ);
    #endif
}

/* ----- Misc ----- */

void GLCommandBuffer::SyncGPU()
{
    glFinish();
}


} // /namespace LLGL



// ================================================================================
