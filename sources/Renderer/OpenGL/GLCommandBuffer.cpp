/*
 * GLCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCommandBuffer.h"
#include "GLRenderContext.h"
#include "../GLCommon/GLTypes.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionLoader.h"
#include "../Assertion.h"
#include "../CheckedCast.h"
#include "../../Core/Exception.h"

#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLTextureArray.h"
#include "Texture/GLSampler.h"
#include "Texture/GLSamplerArray.h"
#include "Texture/GLRenderTarget.h"

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
    stateMngr_ { stateMngr }
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

void GLCommandBuffer::SetViewportArray(std::uint32_t numViewports, const Viewport* viewportArray)
{
    /* Setup GL viewports and depth-ranges */
    std::vector<GLViewport> viewportsGL;
    viewportsGL.reserve(numViewports);

    std::vector<GLDepthRange> depthRangesGL;
    depthRangesGL.reserve(numViewports);

    for (std::uint32_t i = 0; i < numViewports; ++i)
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

void GLCommandBuffer::SetScissorArray(std::uint32_t numScissors, const Scissor* scissorArray)
{
    /* Setup GL scissors */
    std::vector<GLScissor> scissorsGL;
    scissorsGL.reserve(numScissors);

    for (std::uint32_t i = 0; i < numScissors; ++i)
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

void GLCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    glClearStencil(static_cast<GLint>(stencil));
}

void GLCommandBuffer::Clear(long flags)
{
    /* Setup GL clear mask and clear respective buffer */
    GLbitfield mask = 0;

    if ((flags & ClearFlags::Color) != 0)
    {
        //stateMngr_->SetColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    
    if ((flags & ClearFlags::Depth) != 0)
    {
        stateMngr_->SetDepthMask(GL_TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Stencil) != 0)
    {
        //stateMngr_->SetStencilMask(GL_TRUE);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

void GLCommandBuffer::ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color)
{
    /* Clear target color buffer */
    glClearBufferfv(GL_COLOR, static_cast<GLint>(targetIndex), color.Ptr());
}

/* ----- Buffers ------ */

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
    renderState_.indexBufferStride      = static_cast<GLsizeiptr>(format.GetFormatSize());
}

void GLCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*shaderStageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::UNIFORM_BUFFER, buffer, slot);
}

void GLCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    SetGenericBufferArray(GLBufferTarget::UNIFORM_BUFFER, bufferArray, startSlot);
}

void GLCommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*shaderStageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLCommandBuffer::SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    SetGenericBufferArray(GLBufferTarget::SHADER_STORAGE_BUFFER, bufferArray, startSlot);
}

void GLCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    SetGenericBuffer(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, buffer, 0);
}

void GLCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    SetGenericBufferArray(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, bufferArray, 0);
}

void GLCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    #ifdef __APPLE__
    glBeginTransformFeedback(GLTypes::Map(primitiveType));
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glBeginTransformFeedback(GLTypes::Map(primitiveType));
    else if (HasExtension(GLExt::NV_transform_feedback))
        glBeginTransformFeedbackNV(GLTypes::Map(primitiveType));
    else
        ThrowNotSupported("stream-outputs");
    #endif
}

void GLCommandBuffer::EndStreamOutput()
{
    #ifdef __APPLE__
    glEndTransformFeedback();
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        glEndTransformFeedback();
    else if (HasExtension(GLExt::NV_transform_feedback))
        glEndTransformFeedbackNV();
    else
        ThrowNotSupported("stream-outputs");
    #endif
}

/* ----- Textures ----- */

void GLCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*shaderStageFlags*/)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

void GLCommandBuffer::SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
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

void GLCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*shaderStageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

void GLCommandBuffer::SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    auto& samplerArrayGL = LLGL_CAST(GLSamplerArray&, samplerArray);
    stateMngr_->BindSamplers(
        startSlot,
        static_cast<std::uint32_t>(samplerArrayGL.GetIDArray().size()),
        samplerArrayGL.GetIDArray().data()
    );
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
    stateMngr_->BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFramebuffer().GetID());

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
    stateMngr_->BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);

    /*
    Ensure the specified render context is the active one,
    and notify the state manager about new render target (the default framebuffer) height
    */
    GLRenderContext::GLMakeCurrent(&renderContextGL);

    /* Reset reference to render target */
    boundRenderTarget_ = nullptr;
}

/* ----- Pipeline States ----- */

void GLCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics pipeline render states */
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    graphicsPipelineGL.Bind(*stateMngr_);

    /* Store draw modes */
    renderState_.drawMode = graphicsPipelineGL.GetDrawMode();
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

/*
NOTE:
In the following Draw* functions, 'indices' is from type 'GLsizeiptr' to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

void GLCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices)
    );
}

void GLCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        vertexOffset
    );
}

void GLCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    glDrawArraysInstanced(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset)
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

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto indices = static_cast<GLsizeiptr>(firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
}

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset)
{
    #ifndef __APPLE__
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numVertices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        instanceOffset
    );
    #endif
}

/* ----- Compute ----- */

void GLCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
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


/*
 * ======= Private: =======
 */

void GLCommandBuffer::SetGenericBuffer(const GLBufferTarget bufferTarget, Buffer& buffer, std::uint32_t slot)
{
    /* Bind buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(bufferTarget, slot, bufferGL.GetID());
}

void GLCommandBuffer::SetGenericBufferArray(const GLBufferTarget bufferTarget, BufferArray& bufferArray, std::uint32_t startSlot)
{
    /* Bind buffers with BindBuffersBase */
    auto& bufferArrayGL = LLGL_CAST(GLBufferArray&, bufferArray);
    stateMngr_->BindBuffersBase(
        bufferTarget,
        startSlot,
        static_cast<GLsizei>(bufferArrayGL.GetIDArray().size()),
        bufferArrayGL.GetIDArray().data()
    );
}


} // /namespace LLGL



// ================================================================================
