/*
 * GLCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCommandBuffer.h"
#include "GLRenderContext.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/GLCore.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionLoader.h"
#include "../CheckedCast.h"
#include "../../Core/Assertion.h"

#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLSampler.h"
#include "Texture/GLRenderTarget.h"

#include "Buffer/GLVertexBuffer.h"
#include "Buffer/GLIndexBuffer.h"
#include "Buffer/GLVertexBufferArray.h"

#include "RenderState/GLStateManager.h"
#include "RenderState/GLGraphicsPipeline.h"
#include "RenderState/GLComputePipeline.h"
#include "RenderState/GLResourceHeap.h"
#include "RenderState/GLRenderPass.h"
#include "RenderState/GLQuery.h"


namespace LLGL
{


// Maximal number of viewports for the GL renderer.
static const std::uint32_t g_maxNumViewportsGL = 16;

GLCommandBuffer::GLCommandBuffer(const std::shared_ptr<GLStateManager>& stateMngr) :
    stateMngr_ { stateMngr }
{
}

/* ----- Encoding ----- */

void GLCommandBuffer::Begin()
{
    // dummy
}

void GLCommandBuffer::End()
{
    // dummy
}

void GLCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    dstBufferGL.BufferSubData(static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(dataSize), data);
}

void GLCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    auto& srcBufferGL = LLGL_CAST(GLBuffer&, srcBuffer);
    dstBufferGL.CopyBufferSubData(
        srcBufferGL,
        static_cast<GLintptr>(srcOffset),
        static_cast<GLintptr>(dstOffset),
        static_cast<GLsizeiptr>(size)
    );
}

/* ----- Configuration ----- */

void GLCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    if (stateDesc != nullptr && stateDescSize == sizeof(OpenGLDependentStateDescriptor))
    {
        stateMngr_->SetGraphicsAPIDependentState(
            *reinterpret_cast<const OpenGLDependentStateDescriptor*>(stateDesc)
        );
    }
}

/* ----- Viewport and Scissor ----- */

void GLCommandBuffer::SetViewport(const Viewport& viewport)
{
    /* Setup GL viewport and depth-range */
    GLViewport viewportGL { viewport.x, viewport.y, viewport.width, viewport.height };
    GLDepthRange depthRangeGL { viewport.minDepth, viewport.maxDepth };

    /* Set final state */
    stateMngr_->SetViewport(viewportGL);
    stateMngr_->SetDepthRange(depthRangeGL);
}

void GLCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    GLViewport viewportsGL[g_maxNumViewportsGL];
    GLDepthRange depthRangesGL[g_maxNumViewportsGL];

    for (std::uint32_t offset = 0; offset < numViewports; offset += g_maxNumViewportsGL)
    {
        /* Setup GL viewports and depth-ranges */
        auto n = std::min(numViewports - offset, g_maxNumViewportsGL);

        for (std::uint32_t i = 0; i < n; ++i)
        {
            /* Copy GL viewport data */
            viewportsGL[i].x        = viewports[i].x;
            viewportsGL[i].y        = viewports[i].y;
            viewportsGL[i].width    = viewports[i].width;
            viewportsGL[i].height   = viewports[i].height;

            /* Copy GL depth-range data */
            depthRangesGL[i].minDepth = static_cast<GLdouble>(viewports[i].minDepth);
            depthRangesGL[i].maxDepth = static_cast<GLdouble>(viewports[i].maxDepth);
        }

        /* Submit viewports and depth-ranges to state manager */
        stateMngr_->SetViewportArray(offset, n, viewportsGL);
        stateMngr_->SetDepthRangeArray(offset, n, depthRangesGL);
    }
}

void GLCommandBuffer::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL { scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    GLScissor scissorsGL[g_maxNumViewportsGL];

    for (std::uint32_t offset = 0; offset < numScissors; offset += g_maxNumViewportsGL)
    {
        /* Setup GL scissors */
        auto n = std::min(numScissors - offset, g_maxNumViewportsGL);

        for (std::uint32_t i = 0; i < n; ++i)
        {
            /* Copy GL scissor data */
            scissorsGL[i].x         = static_cast<GLint>(scissors[i].x);
            scissorsGL[i].y         = static_cast<GLint>(scissors[i].y);
            scissorsGL[i].width     = static_cast<GLsizei>(scissors[i].width);
            scissorsGL[i].height    = static_cast<GLsizei>(scissors[i].height);
        }

        /* Submit scissors to state manager */
        stateMngr_->SetScissorArray(offset, n, scissorsGL);
    }
}

/* ----- Clear ----- */

void GLCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    /* Submit clear value to GL */
    glClearColor(color.r, color.g, color.b, color.a);

    /* Store as default clear value */
    clearValue_.color[0] = color.r;
    clearValue_.color[1] = color.g;
    clearValue_.color[2] = color.b;
    clearValue_.color[3] = color.a;
}

void GLCommandBuffer::SetClearDepth(float depth)
{
    /* Submit clear value to GL */
    glClearDepth(depth);

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Submit clear value to GL */
    glClearStencil(static_cast<GLint>(stencil));

    /* Store as default clear value */
    clearValue_.stencil = static_cast<GLint>(stencil);
}

//TODO: maybe glColorMask must be set to (1, 1, 1, 1) to clear color correctly
void GLCommandBuffer::Clear(long flags)
{
    stateMngr_->PushDepthMask();
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

        /* Clear buffers */
        glClear(mask);
    }
    stateMngr_->PopDepthMask();
}

//TODO: maybe glColorMask must be set to (1, 1, 1, 1) to clear color correctly
void GLCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Clear color buffer */
            glClearBufferfv(
                GL_COLOR,
                static_cast<GLint>(attachments->colorAttachment),
                attachments->clearValue.color.Ptr()
            );
        }
        else if ((attachments->flags & ClearFlags::DepthStencil) == ClearFlags::DepthStencil)
        {
            /* Clear depth and stencil buffer simultaneously */
            stateMngr_->PushDepthMask();
            stateMngr_->SetDepthMask(GL_TRUE);
            {
                glClearBufferfi(
                    GL_DEPTH_STENCIL,
                    0,
                    attachments->clearValue.depth,
                    static_cast<GLint>(attachments->clearValue.stencil)
                );
            }
            stateMngr_->PopDepthMask();
        }
        else if ((attachments->flags & ClearFlags::Depth) != 0)
        {
            /* Clear only depth buffer */
            stateMngr_->PushDepthMask();
            stateMngr_->SetDepthMask(GL_TRUE);
            {
                glClearBufferfv(GL_DEPTH, 0, &(attachments->clearValue.depth));
            }
            stateMngr_->PopDepthMask();
        }
        else if ((attachments->flags & ClearFlags::Stencil) != 0)
        {
            /* Clear only stencil buffer */
            GLint stencil = static_cast<GLint>(attachments->clearValue.stencil);
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
    }
}

/* ----- Input Assembly ------ */

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
    stateMngr_->BindElementArrayBufferToVAO(indexBufferGL.GetID());

    /* Store new index buffer data in global render state */
    const auto& format = indexBufferGL.GetIndexFormat();
    renderState_.indexBufferDataType    = GLTypes::Map(format.GetDataType());
    renderState_.indexBufferStride      = static_cast<GLsizeiptr>(format.GetFormatSize());
}

/* ----- Constant Buffers ------ */

void GLCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::UNIFORM_BUFFER, buffer, slot);
}

/* ----- Storage Buffers ------ */

void GLCommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

/* ----- Stream Output Buffers ------ */

void GLCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    SetGenericBuffer(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, buffer, 0);
}

void GLCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    SetGenericBufferArray(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, bufferArray, 0);
}

#ifndef __APPLE__

[[noreturn]]
static void ErrTransformFeedbackNotSupported(const char* funcName)
{
    ThrowNotSupportedExcept(funcName, "stream-outputs (GL_EXT_transform_feedback, NV_transform_feedback)");
}

#endif

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
        ErrTransformFeedbackNotSupported(__FUNCTION__);
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
        ErrTransformFeedbackNotSupported(__FUNCTION__);
    #endif
}

/* ----- Textures ----- */

void GLCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*stageFlags*/)
{
    /* Bind texture to layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

/* ----- Sampler States ----- */

void GLCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*stageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

/* ----- Resource Heaps ----- */

void GLCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

void GLCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

/* ----- Render Passes ----- */

void GLCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    /* Bind render target/context */
    if (renderTarget.IsRenderContext())
        BindRenderContext(LLGL_CAST(GLRenderContext&, renderTarget));
    else
        BindRenderTarget(LLGL_CAST(GLRenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassGL, numClearValues, clearValues);
    }
}

void GLCommandBuffer::EndRenderPass()
{
    // dummy
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
    queryGL.Begin();
}

void GLCommandBuffer::EndQuery(Query& query)
{
    /* Begin query with internal target */
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    queryGL.End();
}

bool GLCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);

    /* Check if query result is available */
    GLint available = 0;
    glGetQueryObjectiv(queryGL.GetFirstID(), GL_QUERY_RESULT_AVAILABLE, &available);

    if (available != GL_FALSE)
    {
        if (HasExtension(GLExt::ARB_timer_query))
        {
            /* Get query result with 64-bit version */
            glGetQueryObjectui64v(queryGL.GetFirstID(), GL_QUERY_RESULT, &result);
        }
        else
        {
            /* Get query result with 32-bit version and convert to 64-bit */
            GLuint result32 = 0;
            glGetQueryObjectuiv(queryGL.GetFirstID(), GL_QUERY_RESULT, &result32);
            result = result32;
        }
        return true;
    }

    return false;
}

bool GLCommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);

    if (HasExtension(GLExt::ARB_pipeline_statistics_query))
    {
        /* Check if query result is available for all query objects */
        GLint available = 0;
        for (auto id : queryGL.GetIDs())
        {
            glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &available);
            if (available == GL_FALSE)
                return false;
        }

        /* Parameter setup for 32-bit and 64-bit version of query function */
        static const std::size_t memberCount = sizeof(QueryPipelineStatistics) / sizeof(std::uint64_t);

        union
        {
            GLuint      ui32;
            GLuint64    ui64;
        }
        params[memberCount];

        const auto numResults = std::min(queryGL.GetIDs().size(), memberCount);

        if (HasExtension(GLExt::ARB_timer_query))
        {
            /* Get query result with 64-bit version */
            for (std::size_t i = 0; i < numResults; ++i)
            {
                params[i].ui64 = 0;
                glGetQueryObjectui64v(queryGL.GetIDs()[i], GL_QUERY_RESULT, &(params[i].ui64));
            }
        }
        else
        {
            /* Get query result with 32-bit version and convert to 64-bit */
            for (std::size_t i = 0; i < numResults; ++i)
            {
                params[i].ui64 = 0;
                glGetQueryObjectuiv(queryGL.GetIDs()[i], GL_QUERY_RESULT, &(params[i].ui32));
            }
        }

        /* Reset remaining output parameters (just for safety) */
        for (auto i = numResults; i < memberCount; ++i)
            params[i].ui64 = 0;

        /* Copy result to output parameter */
        result.numPrimitivesGenerated               = params[0].ui64;
        result.numVerticesSubmitted                 = params[1].ui64;
        result.numPrimitivesSubmitted               = params[2].ui64;
        result.numVertexShaderInvocations           = params[3].ui64;
        result.numTessControlShaderInvocations      = params[4].ui64;
        result.numTessEvaluationShaderInvocations   = params[5].ui64;
        result.numGeometryShaderInvocations         = params[6].ui64;
        result.numFragmentShaderInvocations         = params[7].ui64;
        result.numComputeShaderInvocations          = params[8].ui64;
        result.numGeometryPrimitivesGenerated       = params[9].ui64;
        result.numClippingInputPrimitives           = params[10].ui64;
        result.numClippingOutputPrimitives          = params[11].ui64;
    }
    else
    {
        /* Return only result of first query object (of type GL_PRIMITIVES_GENERATED) */
        return QueryResult(query, result.numPrimitivesGenerated);
    }

    return true;
}

void GLCommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryGL = LLGL_CAST(GLQuery&, query);
    glBeginConditionalRender(queryGL.GetFirstID(), GLTypes::Map(mode));
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

void GLCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices)
    );
}

void GLCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
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

void GLCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    glDrawArraysInstancedBaseInstance(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances),
        firstInstance
    );
    #else
    ErrUnsupportedGLProc("glDrawArraysInstancedBaseInstance");
    #endif
}

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto indices = static_cast<GLsizeiptr>(firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
}

void GLCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        firstInstance
    );
    #else
    ErrUnsupportedGLProc("glDrawElementsInstancedBaseVertexBaseInstance");
    #endif
}

/* ----- Compute ----- */

void GLCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    #ifndef __APPLE__
    glDispatchCompute(groupSizeX, groupSizeY, groupSizeZ);
    #endif
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

void GLCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap)
{
    auto& resourceHeapGL = LLGL_CAST(GLResourceHeap&, resourceHeap);
    resourceHeapGL.Bind(*stateMngr_);
}

void GLCommandBuffer::BlitBoundRenderTarget()
{
    if (boundRenderTarget_)
        boundRenderTarget_->BlitOntoFramebuffer();
}

void GLCommandBuffer::BindRenderTarget(GLRenderTarget& renderTargetGL)
{
    /* Blit previously bound render target (in case mutli-sampling is used) */
    BlitBoundRenderTarget();

    /* Bind framebuffer object */
    stateMngr_->BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, renderTargetGL.GetFramebuffer().GetID());

    /* Notify state manager about new render target height */
    stateMngr_->NotifyRenderTargetHeight(static_cast<GLint>(renderTargetGL.GetResolution().height));

    /* Store current render target */
    boundRenderTarget_ = &renderTargetGL;

    //TODO: maybe use 'glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE)' to allow better compatibility to D3D
}

void GLCommandBuffer::BindRenderContext(GLRenderContext& renderContextGL)
{
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

    //TODO: maybe use 'glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE)' to allow better compatibility to D3D
}

void GLCommandBuffer::ClearAttachmentsWithRenderPass(
    const GLRenderPass& renderPassGL,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    auto mask = renderPassGL.GetClearMask();

    /* Clear color attachments */
    std::uint32_t idx = 0;
    if ((mask & GL_COLOR_BUFFER_BIT) != 0)
        ClearColorBuffers(renderPassGL.GetClearColorAttachments(), numClearValues, clearValues, idx);

    /* Clear depth-stencil attachment */
    static const GLbitfield g_maskDepthStencil = (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    if ((mask & g_maskDepthStencil) == g_maskDepthStencil)
    {
        stateMngr_->PushDepthMask();
        stateMngr_->SetDepthMask(GL_TRUE);
        {
            /* Clear depth and stencil buffer simultaneously */
            if (idx < numClearValues)
                glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValues[idx].depth, static_cast<GLint>(clearValues[idx].stencil));
            else
                glClearBufferfi(GL_DEPTH_STENCIL, 0, clearValue_.depth, clearValue_.stencil);
        }
        stateMngr_->PopDepthMask();
    }
    if ((mask & GL_DEPTH_BUFFER_BIT) != 0)
    {
        stateMngr_->PushDepthMask();
        stateMngr_->SetDepthMask(GL_TRUE);
        {
            /* Clear only depth buffer */
            if (idx < numClearValues)
                glClearBufferfv(GL_DEPTH, 0, &(clearValues[idx].depth));
            else
                glClearBufferfv(GL_DEPTH, 0, &(clearValue_.depth));
        }
        stateMngr_->PopDepthMask();
    }
    else if ((mask & GL_STENCIL_BUFFER_BIT) != 0)
    {
        /* Clear only stencil buffer */
        if (idx < numClearValues)
        {
            GLint stencil = static_cast<GLint>(clearValues[idx].stencil);
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
        else
            glClearBufferiv(GL_STENCIL, 0, &(clearValue_.stencil));
    }
}

void GLCommandBuffer::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx)
{
    std::uint32_t i = 0;

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValues[idx++].color.Ptr());
        else
            return;
    }

    /* Use default clear values */
    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValue_.color);
        else
            return;
    }
}


} // /namespace LLGL



// ================================================================================
