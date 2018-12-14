/*
 * GLImmediateCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLImmediateCommandBuffer.h"
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


GLImmediateCommandBuffer::GLImmediateCommandBuffer(const std::shared_ptr<GLStateManager>& stateMngr) :
    stateMngr_ { stateMngr }
{
}

bool GLImmediateCommandBuffer::IsImmediateCmdBuffer() const
{
    return true;
}

/* ----- Encoding ----- */

void GLImmediateCommandBuffer::Begin()
{
    // dummy
}

void GLImmediateCommandBuffer::End()
{
    // dummy
}

void GLImmediateCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferGL = LLGL_CAST(GLBuffer&, dstBuffer);
    dstBufferGL.BufferSubData(static_cast<GLintptr>(dstOffset), static_cast<GLsizeiptr>(dataSize), data);
}

void GLImmediateCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
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

void GLImmediateCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    if (stateDesc != nullptr && stateDescSize == sizeof(OpenGLDependentStateDescriptor))
    {
        stateMngr_->SetGraphicsAPIDependentState(
            *reinterpret_cast<const OpenGLDependentStateDescriptor*>(stateDesc)
        );
    }
}

/* ----- Viewport and Scissor ----- */

void GLImmediateCommandBuffer::SetViewport(const Viewport& viewport)
{
    /* Setup GL viewport and depth-range */
    GLViewport viewportGL{ viewport.x, viewport.y, viewport.width, viewport.height };
    GLDepthRange depthRangeGL{ viewport.minDepth, viewport.maxDepth };

    /* Set final state */
    stateMngr_->SetViewport(viewportGL);
    stateMngr_->SetDepthRange(depthRangeGL);
}

void GLImmediateCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
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

void GLImmediateCommandBuffer::SetScissor(const Scissor& scissor)
{
    /* Setup and submit GL scissor to state manager */
    GLScissor scissorGL{ scissor.x, scissor.y, scissor.width, scissor.height };
    stateMngr_->SetScissor(scissorGL);
}

void GLImmediateCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
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

void GLImmediateCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    /* Submit clear value to GL */
    glClearColor(color.r, color.g, color.b, color.a);

    /* Store as default clear value */
    clearValue_.color[0] = color.r;
    clearValue_.color[1] = color.g;
    clearValue_.color[2] = color.b;
    clearValue_.color[3] = color.a;
}

void GLImmediateCommandBuffer::SetClearDepth(float depth)
{
    /* Submit clear value to GL */
    glClearDepth(depth);

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLImmediateCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Submit clear value to GL */
    glClearStencil(static_cast<GLint>(stencil));

    /* Store as default clear value */
    clearValue_.stencil = static_cast<GLint>(stencil);
}

void GLImmediateCommandBuffer::Clear(long flags)
{
    stateMngr_->Clear(flags);
}

void GLImmediateCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    stateMngr_->ClearBuffers(numAttachments, attachments);
}

/* ----- Input Assembly ------ */

void GLImmediateCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferGL = LLGL_CAST(GLBufferWithVAO&, buffer);
        stateMngr_->BindVertexArray(vertexBufferGL.GetVaoID());
    }
}

void GLImmediateCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        /* Bind vertex buffer */
        auto& vertexBufferArrayGL = LLGL_CAST(GLBufferArrayWithVAO&, bufferArray);
        stateMngr_->BindVertexArray(vertexBufferArrayGL.GetVaoID());
    }
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID());
    SetIndexFormat(renderState_, bufferGL.IsIndexType16Bits(), 0);
}

void GLImmediateCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    /* Bind index buffer deferred (can only be bound to the active VAO) */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindElementArrayBufferToVAO(bufferGL.GetID());
    SetIndexFormat(renderState_, format == Format::R16UInt, offset);
}

/* ----- Stream Output Buffers ------ */

void GLImmediateCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    SetGenericBuffer(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, buffer, 0);
}

void GLImmediateCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
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

void GLImmediateCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
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

void GLImmediateCommandBuffer::EndStreamOutput()
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

void GLImmediateCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

void GLImmediateCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

/* ----- Render Passes ----- */

void GLImmediateCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    stateMngr_->BindRenderPass(renderTarget, renderPass, numClearValues, clearValues, clearValue_);
}

void GLImmediateCommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void GLImmediateCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Bind graphics pipeline render states */
    auto& graphicsPipelineGL = LLGL_CAST(GLGraphicsPipeline&, graphicsPipeline);
    graphicsPipelineGL.Bind(*stateMngr_);

    /* Store draw mode */
    renderState_.drawMode = graphicsPipelineGL.GetDrawMode();
}

void GLImmediateCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineGL = LLGL_CAST(GLComputePipeline&, computePipeline);
    computePipelineGL.Bind(*stateMngr_);
}

/* ----- Queries ----- */

void GLImmediateCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.Begin(query);
}

void GLImmediateCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    /* Begin query with internal target */
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    queryHeapGL.End(query);
}

void GLImmediateCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapGL = LLGL_CAST(GLQueryHeap&, queryHeap);
    glBeginConditionalRender(queryHeapGL.GetFirstID(query), GLTypes::Map(mode));
}

void GLImmediateCommandBuffer::EndRenderCondition()
{
    glEndConditionalRender();
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type <GLintptr> to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

void GLImmediateCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    glDrawArrays(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElements(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices)
    );
}

void GLImmediateCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        vertexOffset
    );
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    glDrawArraysInstanced(
        renderState_.drawMode,
        static_cast<GLint>(firstVertex),
        static_cast<GLsizei>(numVertices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLImmediateCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
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

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances)
    );
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );
}

void GLImmediateCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
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

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawArraysIndirect(
        renderState_.drawMode,
        reinterpret_cast<const GLvoid*>(indirect)
    );
}

void GLImmediateCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
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

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLintptr indirect = static_cast<GLintptr>(offset);
    glDrawElementsIndirect(
        renderState_.drawMode,
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indirect)
    );
}

void GLImmediateCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    /* Bind indirect argument buffer */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    GLintptr indirect = static_cast<GLintptr>(offset);
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

void GLImmediateCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #ifndef __APPLE__
    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
    #else
    ErrUnsupportedGLProc("glDispatchCompute");
    #endif
}

void GLImmediateCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
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

void GLImmediateCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::UNIFORM_BUFFER, buffer, slot);
}

void GLImmediateCommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLImmediateCommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLImmediateCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*stageFlags*/)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    stateMngr_->ActiveTexture(slot);
    stateMngr_->BindTexture(textureGL);
}

void GLImmediateCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*stageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    stateMngr_->BindSampler(slot, samplerGL.GetID());
}

void GLImmediateCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                /*stageFlags*/)
{
    if (numSlots > 0)
    {
        auto first = static_cast<GLuint>(std::min(firstSlot, GLStateManager::g_maxNumResourceSlots - 1u));
        auto count = static_cast<GLsizei>(std::min(numSlots, GLStateManager::g_maxNumResourceSlots - first));

        switch (resourceType)
        {
            case ResourceType::Undefined:
            break;

            case ResourceType::Buffer:
            {
                if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::UNIFORM_BUFFER, first, count);
                if ((bindFlags & (BindFlags::SampleBuffer | BindFlags::RWStorageBuffer)) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::SHADER_STORAGE_BUFFER, first, count);
                if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
                    stateMngr_->UnbindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, first, count);
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
                stateMngr_->UnbindSamplers(first, count);
            }
            break;
        }
    }
}


/*
 * ======= Private: =======
 */

void GLImmediateCommandBuffer::SetGenericBuffer(const GLBufferTarget bufferTarget, Buffer& buffer, std::uint32_t slot)
{
    /* Bind buffer with BindBufferBase */
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBufferBase(bufferTarget, slot, bufferGL.GetID());
}

void GLImmediateCommandBuffer::SetGenericBufferArray(const GLBufferTarget bufferTarget, BufferArray& bufferArray, std::uint32_t startSlot)
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

void GLImmediateCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap)
{
    auto& resourceHeapGL = LLGL_CAST(GLResourceHeap&, resourceHeap);
    resourceHeapGL.Bind(*stateMngr_);
}


} // /namespace LLGL



// ================================================================================
