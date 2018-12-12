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
#include "../StaticLimits.h"
#include "../../Core/Assertion.h"

#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLSampler.h"
#include "Texture/GLRenderTarget.h"

#include "Buffer/GLBufferWithVAO.h"
#include "Buffer/GLBufferArrayWithVAO.h"

#include "RenderState/GLStateManager.h"
#include "RenderState/GLGraphicsPipeline.h"
#include "RenderState/GLComputePipeline.h"
#include "RenderState/GLResourceHeap.h"
#include "RenderState/GLRenderPass.h"
#include "RenderState/GLQueryHeap.h"


namespace LLGL
{


// Global array of null pointers to unbind resource slots
static const std::uint32_t  g_maxNumResourceSlots                   = 64;
static GLuint               g_nullResources[g_maxNumResourceSlots]  = {};


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
    GLViewport viewportsGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    GLDepthRange depthRangesGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Setup GL viewports and depth-ranges */
    auto count = static_cast<GLsizei>(std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

    for (GLsizei i = 0; i < count; ++i)
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
    stateMngr_->SetViewportArray(0, count, viewportsGL);
    stateMngr_->SetDepthRangeArray(0, count, depthRangesGL);
}

void GLCommandBuffer::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL { scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    GLScissor scissorsGL[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Setup GL scissors */
    auto count = static_cast<GLsizei>(std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

    for (GLsizei i = 0; i < count; ++i)
    {
        /* Copy GL scissor data */
        scissorsGL[i].x         = static_cast<GLint>(scissors[i].x);
        scissorsGL[i].y         = static_cast<GLint>(scissors[i].y);
        scissorsGL[i].width     = static_cast<GLsizei>(scissors[i].width);
        scissorsGL[i].height    = static_cast<GLsizei>(scissors[i].height);
    }

    /* Submit scissors to state manager */
    stateMngr_->SetScissorArray(0, count, scissorsGL);
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

void GLCommandBuffer::Clear(long flags)
{
    /* Setup GL clear mask and clear respective buffer */
    GLbitfield mask = 0;

    if ((flags & ClearFlags::Color) != 0)
    {
        stateMngr_->PushColorMaskAndEnable();
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Depth) != 0)
    {
        stateMngr_->PushDepthMaskAndEnable();
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if ((flags & ClearFlags::Stencil) != 0)
    {
        //stateMngr_->SetStencilMask(GL_TRUE);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    /* Clear buffers */
    glClear(mask);

    /* Restore framebuffer masks */
    if ((flags & ClearFlags::Depth) != 0)
        stateMngr_->PopDepthMask();
    if ((flags & ClearFlags::Color) != 0)
        stateMngr_->PopColorMask();
}

void GLCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    bool clearedDepth = false, clearedColor = false;

    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            /* Enable color mask temporarily */
            stateMngr_->PushColorMaskAndEnable();
            clearedColor = true;

            /* Clear color buffer */
            glClearBufferfv(
                GL_COLOR,
                static_cast<GLint>(attachments->colorAttachment),
                attachments->clearValue.color.Ptr()
            );
        }
        else if ((attachments->flags & ClearFlags::DepthStencil) == ClearFlags::DepthStencil)
        {
            /* Enable depth mask temporarily */
            stateMngr_->PushDepthMaskAndEnable();
            clearedDepth = true;

            /* Clear depth and stencil buffer simultaneously */
            glClearBufferfi(
                GL_DEPTH_STENCIL,
                0,
                attachments->clearValue.depth,
                static_cast<GLint>(attachments->clearValue.stencil)
            );
        }
        else if ((attachments->flags & ClearFlags::Depth) != 0)
        {
            /* Enable depth mask temporarily */
            stateMngr_->PushDepthMaskAndEnable();
            clearedDepth = true;

            /* Clear only depth buffer */
            glClearBufferfv(GL_DEPTH, 0, &(attachments->clearValue.depth));
        }
        else if ((attachments->flags & ClearFlags::Stencil) != 0)
        {
            /* Clear only stencil buffer */
            GLint stencil = static_cast<GLint>(attachments->clearValue.stencil);
            glClearBufferiv(GL_STENCIL, 0, &stencil);
        }
    }

    if (clearedDepth)
        stateMngr_->PopDepthMask();
    if (clearedColor)
        stateMngr_->PopColorMask();
}

/* ----- Input Assembly ------ */

void GLCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferGL = LLGL_CAST(GLBufferWithVAO&, buffer);
        stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
    }
}

void GLCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferArrayGL = LLGL_CAST(GLBufferArrayWithVAO&, bufferArray);
        stateMngr_->BindVertexArray(vertexBufferArrayGL.GetVaoID());
    }
}

void GLCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID());

    /* Store new index buffer data in global render state */
    if (bufferGL.IsIndexType16Bits())
    {
        renderState_.indexBufferDataType    = GL_UNSIGNED_SHORT;
        renderState_.indexBufferStride      = 2;
    }
    else
    {
        renderState_.indexBufferDataType    = GL_UNSIGNED_INT;
        renderState_.indexBufferStride      = 4;
    }
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

void GLCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.Begin(query);
}

void GLCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.End(query);
}

void GLCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    glBeginConditionalRender(queryHeapGL.GetFirstID(query), GLTypes::Map(mode));
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

void GLCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    glDrawArraysIndirect(
        renderState_.drawMode,
        reinterpret_cast<const GLvoid*>(indirect)
    );
}

void GLCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawArraysIndirect(
            renderState_.drawMode,
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif // /__APPLE__
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawArraysIndirect(
                renderState_.drawMode,
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
}

void GLCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    glDrawElementsIndirect(
        renderState_.drawMode,
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indirect)
    );
}

void GLCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        /* Use native multi draw command */
        glMultiDrawElementsIndirect(
            renderState_.drawMode,
            renderState_.indexBufferDataType,
            reinterpret_cast<const GLvoid*>(indirect),
            static_cast<GLsizei>(numCommands),
            static_cast<GLsizei>(stride)
        );
    }
    else
    #endif // /__APPLE__
    {
        /* Emulate multi draw command */
        while (numCommands-- > 0)
        {
            glDrawElementsIndirect(
                renderState_.drawMode,
                renderState_.indexBufferDataType,
                reinterpret_cast<const GLvoid*>(indirect)
            );
            indirect += stride;
        }
    }
}

/* ----- Compute ----- */

void GLCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #ifndef __APPLE__
    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
    #else
    ErrUnsupportedGLProc("glDispatchCompute");
    #endif
}

void GLCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifndef __APPLE__
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DISPATCH_INDIRECT_BUFFER, bufferGL.GetID());
    glDispatchComputeIndirect(static_cast<GLintptr>(offset));
    #else
    ErrUnsupportedGLProc("glDispatchComputeIndirect");
    #endif
}

/* ----- Direct Resource Access ------ */

void GLCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::UNIFORM_BUFFER, buffer, slot);
}

void GLCommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLCommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*stageFlags*/)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

void GLCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*stageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

void GLCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                /*stageFlags*/)
{
    if (numSlots > 0)
    {
        auto first = static_cast<GLuint>(std::min(firstSlot, g_maxNumResourceSlots - 1u));
        auto count = static_cast<GLsizei>(std::min(numSlots, g_maxNumResourceSlots - first));

        switch (resourceType)
        {
            case ResourceType::Undefined:
                break;

            case ResourceType::Buffer:
            {
                if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                    stateMngr_->BindBuffersBase(GLBufferTarget::UNIFORM_BUFFER, first, count, g_nullResources);
                if ((bindFlags & (BindFlags::SampleBuffer | BindFlags::RWStorageBuffer)) != 0)
                    stateMngr_->BindBuffersBase(GLBufferTarget::SHADER_STORAGE_BUFFER, first, count, g_nullResources);
                if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
                    stateMngr_->BindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, first, count, g_nullResources);
            }
            break;

            case ResourceType::Texture:
            {
                if ((bindFlags & BindFlags::SampleBuffer) != 0)
                    stateMngr_->UnbindTextures(first, count);
                #if 0//TODO
                if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
                    stateMngr_->UnbindImages(first, count);
                #endif
            }
            break;

            case ResourceType::Sampler:
            {
                stateMngr_->BindSamplers(first, count, g_nullResources);
            }
            break;
        }
    }
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
    stateMngr_->BindRenderTarget(&renderTargetGL);

    /* Notify state manager about new render target height */
    stateMngr_->NotifyRenderTargetHeight(static_cast<GLint>(renderTargetGL.GetResolution().height));

    /* Store current render target */
    boundRenderTarget_  = (&renderTargetGL);
    numDrawBuffers_     = renderTargetGL.GetNumColorAttachments();

    //TODO: maybe use 'glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE)' to allow better compatibility to D3D
}

void GLCommandBuffer::BindRenderContext(GLRenderContext& renderContextGL)
{
    /* Blit previously bound render target (in case mutli-sampling is used) */
    BlitBoundRenderTarget();

    /* Unbind framebuffer object */
    stateMngr_->BindRenderTarget(nullptr);

    /*
    Ensure the specified render context is the active one,
    and notify the state manager about new render target (the default framebuffer) height
    */
    GLRenderContext::GLMakeCurrent(&renderContextGL);

    /* Reset reference to render target */
    boundRenderTarget_  = nullptr;
    numDrawBuffers_     = 1;

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
    {
        if (ClearColorBuffers(renderPassGL.GetClearColorAttachments(), numClearValues, clearValues, idx) > 0)
            stateMngr_->PopColorMask();
    }

    /* Clear depth-stencil attachment */
    static const GLbitfield g_maskDepthStencil = (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    if ((mask & g_maskDepthStencil) == g_maskDepthStencil)
    {
        stateMngr_->PushDepthMaskAndEnable();
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
        stateMngr_->PushDepthMaskAndEnable();
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

std::uint32_t GLCommandBuffer::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx)
{
    std::uint32_t i = 0, n = 0;

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            stateMngr_->PushColorMaskAndEnable();
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValues[idx++].color.Ptr());
            ++n;
        }
        else
            return n;
    }

    /* Use default clear values */
    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            stateMngr_->PushColorMaskAndEnable();
            glClearBufferfv(GL_COLOR, static_cast<GLint>(colorBuffers[i]), clearValue_.color);
            ++n;
        }
        else
            return n;
    }

    return n;
}


} // /namespace LLGL



// ================================================================================
