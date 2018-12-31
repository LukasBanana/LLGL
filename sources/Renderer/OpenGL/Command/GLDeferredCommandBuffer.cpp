/*
 * GLDeferredCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLDeferredCommandBuffer.h"
#include "GLCommand.h"

#include "../GLRenderContext.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../StaticLimits.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLGraphicsPipeline.h"
#include "../RenderState/GLComputePipeline.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <algorithm>

#ifdef LLGL_ENABLE_JIT_COMPILER
#   include "GLCommandAssembler.h"
#endif // /LLGL_ENABLE_JIT_COMPILER


namespace LLGL
{


GLDeferredCommandBuffer::GLDeferredCommandBuffer(long flags, std::size_t reservedSize)
{
    buffer_.reserve(reservedSize);
    #ifdef LLGL_ENABLE_JIT_COMPILER
    useJITCompiler_ = ((flags & CommandBufferFlags::MultiSubmit) != 0);
    #endif // /LLGL_ENABLE_JIT_COMPILER
}

bool GLDeferredCommandBuffer::IsImmediateCmdBuffer() const
{
    return false;
}

/* ----- Encoding ----- */

void GLDeferredCommandBuffer::Begin()
{
    buffer_.clear();
    #ifdef LLGL_ENABLE_JIT_COMPILER
    executable_.reset();
    #endif // /LLGL_ENABLE_JIT_COMPILER
}

void GLDeferredCommandBuffer::End()
{
    #ifdef LLGL_ENABLE_JIT_COMPILER
    if (useJITCompiler_)
        executable_ = AssembleGLDeferredCommandBuffer(*this);
    #endif // /LLGL_ENABLE_JIT_COMPILER
}

void GLDeferredCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto cmd = AllocCommand<GLCmdUpdateBuffer>(GLOpCodeUpdateBuffer, dataSize);
    {
        cmd->buffer = LLGL_CAST(GLBuffer*, &dstBuffer);
        cmd->offset = static_cast<GLintptr>(dstOffset);
        cmd->size   = static_cast<GLsizeiptr>(dataSize);
        ::memcpy(cmd + 1, data, dataSize);
    }
}

void GLDeferredCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto cmd = AllocCommand<GLCmdCopyBuffer>(GLOpCodeCopyBuffer);
    {
        cmd->writeBuffer    = LLGL_CAST(GLBuffer*, &dstBuffer);
        cmd->readBuffer     = LLGL_CAST(GLBuffer*, &srcBuffer);
        cmd->readOffset     = static_cast<GLintptr>(srcOffset);
        cmd->writeOffset    = static_cast<GLintptr>(dstOffset);
        cmd->size           = static_cast<GLsizeiptr>(size);
    }
}

void GLDeferredCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    throw std::runtime_error("deferred command buffer tried to execute another command buffer");
}

/* ----- Configuration ----- */

void GLDeferredCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    if (stateDesc != nullptr && stateDescSize == sizeof(OpenGLDependentStateDescriptor))
    {
        auto cmd = AllocCommand<GLCmdSetAPIDepState>(GLOpCodeSetAPIDepState);
        cmd->desc = *reinterpret_cast<const OpenGLDependentStateDescriptor*>(stateDesc);
    }
}

/* ----- Viewport and Scissor ----- */

void GLDeferredCommandBuffer::SetViewport(const Viewport& viewport)
{
    auto cmd = AllocCommand<GLCmdViewport>(GLOpCodeViewport);
    {
        cmd->viewport   = GLViewport{ viewport.x, viewport.y, viewport.width, viewport.height };
        cmd->depthRange = GLDepthRange{ viewport.minDepth, viewport.maxDepth };
    }
}

void GLDeferredCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    auto cmd = AllocCommand<GLCmdViewportArray>(GLOpCodeViewportArray, sizeof(GLViewport)*numViewports);
    {
        cmd->first = 0;
        cmd->count = static_cast<GLsizei>(std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

        auto viewportsGL = reinterpret_cast<GLViewport*>(cmd + 1);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            viewportsGL[i].x        = viewports[i].x;
            viewportsGL[i].y        = viewports[i].y;
            viewportsGL[i].width    = viewports[i].width;
            viewportsGL[i].height   = viewports[i].height;
        }

        auto depthRangesGL = reinterpret_cast<GLDepthRange*>(viewportsGL + numViewports);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            depthRangesGL[i].minDepth = static_cast<GLdouble>(viewports[i].minDepth);
            depthRangesGL[i].maxDepth = static_cast<GLdouble>(viewports[i].maxDepth);
        }
    }
}

void GLDeferredCommandBuffer::SetScissor(const Scissor& scissor)
{
    auto cmd = AllocCommand<GLCmdScissor>(GLOpCodeScissor);
    cmd->scissor = GLScissor{ scissor.x, scissor.y, scissor.width, scissor.height };
}

void GLDeferredCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    auto cmd = AllocCommand<GLCmdScissorArray>(GLOpCodeScissorArray, sizeof(GLScissor)*numScissors);
    {
        cmd->first = 0;
        cmd->count = static_cast<GLsizei>(std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS));

        auto scissorsGL = reinterpret_cast<GLScissor*>(cmd + 1);
        for (GLsizei i = 0; i < cmd->count; ++i)
        {
            scissorsGL[i].x         = static_cast<GLint>(scissors[i].x);
            scissorsGL[i].y         = static_cast<GLint>(scissors[i].y);
            scissorsGL[i].width     = static_cast<GLsizei>(scissors[i].width);
            scissorsGL[i].height    = static_cast<GLsizei>(scissors[i].height);
        }
    }
}

/* ----- Clear ----- */

void GLDeferredCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearColor>(GLOpCodeClearColor);
    {
        cmd->color[0] = color.r;
        cmd->color[1] = color.g;
        cmd->color[2] = color.b;
        cmd->color[3] = color.a;
    }

    /* Store as default clear value */
    clearValue_.color[0] = color.r;
    clearValue_.color[1] = color.g;
    clearValue_.color[2] = color.b;
    clearValue_.color[3] = color.a;
}

void GLDeferredCommandBuffer::SetClearDepth(float depth)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearDepth>(GLOpCodeClearDepth);
    cmd->depth = static_cast<GLdouble>(depth);

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLDeferredCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Encode clear command */
    auto cmd = AllocCommand<GLCmdClearStencil>(GLOpCodeClearStencil);
    cmd->stencil = static_cast<GLint>(stencil);

    /* Store as default clear value */
    clearValue_.stencil = cmd->stencil;
}

void GLDeferredCommandBuffer::Clear(long flags)
{
    auto cmd = AllocCommand<GLCmdClear>(GLOpCodeClear);
    cmd->flags = flags;
}

void GLDeferredCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    auto cmd = AllocCommand<GLCmdClearBuffers>(GLOpCodeClearBuffers, sizeof(AttachmentClear)*numAttachments);
    {
        cmd->numAttachments = numAttachments;
        ::memcpy(cmd + 1, attachments, sizeof(AttachmentClear)*numAttachments);
    }
}

/* ----- Input Assembly ------ */

void GLDeferredCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpCodeBindVertexArray);
        cmd->vao = LLGL_CAST(const GLBufferWithVAO&, buffer).GetVaoID();
    }
}

void GLDeferredCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpCodeBindVertexArray);
        cmd->vao = LLGL_CAST(const GLBufferArrayWithVAO&, bufferArray).GetVaoID();
    }
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    auto cmd = AllocCommand<GLCmdBindElementArrayBufferToVAO>(GLOpCodeBindElementArrayBufferToVAO);
    cmd->id = bufferGL.GetID();
    SetIndexFormat(renderState_, bufferGL.IsIndexType16Bits(), 0);
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    auto cmd = AllocCommand<GLCmdBindElementArrayBufferToVAO>(GLOpCodeBindElementArrayBufferToVAO);
    cmd->id = bufferGL.GetID();
    SetIndexFormat(renderState_, format == Format::R16UInt, offset);
}

/* ----- Stream Output Buffers ------ */

void GLDeferredCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    SetGenericBuffer(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, buffer, 0);
}

void GLDeferredCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
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

void GLDeferredCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    #ifdef __APPLE__
    auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpCodeBeginTransformFeedback);
    cmd->primitiveMove = GLTypes::Map(primitiveType);
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedback>(GLOpCodeBeginTransformFeedback);
        cmd->primitiveMove = GLTypes::Map(primitiveType);
    }
    else if (HasExtension(GLExt::NV_transform_feedback))
    {
        auto cmd = AllocCommand<GLCmdBeginTransformFeedbackNV>(GLOpCodeBeginTransformFeedbackNV);
        cmd->primitiveMove = GLTypes::Map(primitiveType);
    }
    else
        ErrTransformFeedbackNotSupported(__FUNCTION__);
    #endif
}

void GLDeferredCommandBuffer::EndStreamOutput()
{
    #ifdef __APPLE__
    AllocOpCode(GLOpCodeEndTransformFeedback);
    #else
    if (HasExtension(GLExt::EXT_transform_feedback))
        AllocOpCode(GLOpCodeEndTransformFeedback);
    else if (HasExtension(GLExt::NV_transform_feedback))
        AllocOpCode(GLOpCodeEndTransformFeedbackNV);
    else
        ErrTransformFeedbackNotSupported(__FUNCTION__);
    #endif
}

/* ----- Resource Heaps ----- */

void GLDeferredCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

void GLDeferredCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    SetResourceHeap(resourceHeap);
}

/* ----- Render Passes ----- */

void GLDeferredCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    auto cmd = AllocCommand<GLCmdBindRenderPass>(GLOpCodeBindRenderPass, sizeof(ClearValue)*numClearValues);
    {
        cmd->renderTarget       = &renderTarget;
        cmd->renderPass         = (renderPass != nullptr ? LLGL_CAST(const GLRenderPass*, renderPass) : nullptr);
        cmd->numClearValues     = numClearValues;
        cmd->defaultClearValue  = clearValue_;
        ::memcpy(cmd + 1, clearValues, sizeof(ClearValue)*numClearValues);
    }
}

void GLDeferredCommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void GLDeferredCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto cmd = AllocCommand<GLCmdBindGraphicsPipeline>(GLOpCodeBindGraphicsPipeline);
    cmd->graphicsPipeline = LLGL_CAST(GLGraphicsPipeline*, &graphicsPipeline);
    renderState_.drawMode = cmd->graphicsPipeline->GetDrawMode();
}

void GLDeferredCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto cmd = AllocCommand<GLCmdBindComputePipeline>(GLOpCodeBindComputePipeline);
    cmd->computePipeline = LLGL_CAST(GLComputePipeline*, &computePipeline);
}

/* ----- Queries ----- */

void GLDeferredCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto cmd = AllocCommand<GLCmdBeginQuery>(GLOpCodeBeginQuery);
    {
        cmd->queryHeap  = LLGL_CAST(GLQueryHeap*, &queryHeap);
        cmd->query      = query;
    }
}

void GLDeferredCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto cmd = AllocCommand<GLCmdEndQuery>(GLOpCodeEndQuery);
    {
        cmd->queryHeap  = LLGL_CAST(GLQueryHeap*, &queryHeap);
        cmd->query      = query;
    }
}

void GLDeferredCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto cmd = AllocCommand<GLCmdBeginConditionalRender>(GLOpCodeBeginConditionalRender);
    {
        cmd->id     = LLGL_CAST(const GLQueryHeap&, queryHeap).GetFirstID(query);
        cmd->mode   = GLTypes::Map(mode);
    }
}

void GLDeferredCommandBuffer::EndRenderCondition()
{
    AllocOpCode(GLOpCodeEndConditionalRender);
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type <GLintptr> to have the same size as a pointer address on either a 32-bit or 64-bit platform.
The indices actually store the index start offset, but must be passed to GL as a void-pointer, due to an obsolete API.
*/

void GLDeferredCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    auto cmd = AllocCommand<GLCmdDrawArrays>(GLOpCodeDrawArrays);
    {
        cmd->mode   = renderState_.drawMode;
        cmd->first  = static_cast<GLint>(firstVertex);
        cmd->count  = static_cast<GLsizei>(numVertices);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElements>(GLOpCodeDrawElements);
    {
        cmd->mode       = renderState_.drawMode;
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = renderState_.indexBufferDataType;
        cmd->indices    = reinterpret_cast<const GLvoid*>(indices);
    }
}

void GLDeferredCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsBaseVertex>(GLOpCodeDrawElementsBaseVertex);
    {
        cmd->mode       = renderState_.drawMode;
        cmd->count      = static_cast<GLsizei>(numIndices);
        cmd->type       = renderState_.indexBufferDataType;
        cmd->indices    = reinterpret_cast<const GLvoid*>(indices);
        cmd->basevertex = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    auto cmd = AllocCommand<GLCmdDrawArraysInstanced>(GLOpCodeDrawArraysInstanced);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->first          = static_cast<GLint>(firstVertex);
        cmd->count          = static_cast<GLsizei>(numVertices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
    }
}

void GLDeferredCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDrawArraysInstancedBaseInstance>(GLOpCodeDrawArraysInstancedBaseInstance);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->first          = static_cast<GLint>(firstVertex);
        cmd->count          = static_cast<GLsizei>(numVertices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->baseinstance   = firstInstance;
    }
    #else
    ErrUnsupportedGLProc("glDrawArraysInstancedBaseInstance");
    #endif
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstanced>(GLOpCodeDrawElementsInstanced);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertex>(GLOpCodeDrawElementsInstancedBaseVertex);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->basevertex     = vertexOffset;
    }
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    #ifndef __APPLE__
    const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    auto cmd = AllocCommand<GLCmdDrawElementsInstancedBaseVertexBaseInstance>(GLOpCodeDrawElementsInstancedBaseVertexBaseInstance);
    {
        cmd->mode           = renderState_.drawMode;
        cmd->count          = static_cast<GLsizei>(numIndices);
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indices        = reinterpret_cast<const GLvoid*>(indices);
        cmd->instancecount  = static_cast<GLsizei>(numInstances);
        cmd->basevertex     = vertexOffset;
        cmd->baseinstance   = firstInstance;
    }
    #else
    ErrUnsupportedGLProc("glDrawElementsInstancedBaseVertexBaseInstance");
    #endif
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto cmd = AllocCommand<GLCmdDrawArraysIndirect>(GLOpCodeDrawArraysIndirect);
    {
        cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
        cmd->numCommands    = 1;
        cmd->mode           = renderState_.drawMode;
        cmd->indirect       = static_cast<GLintptr>(offset);
        cmd->stride         = 0;
    }
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        const GLintptr indirect = static_cast<GLintptr>(offset);
        auto cmd = AllocCommand<GLCmdMultiDrawArraysIndirect>(GLOpCodeMultiDrawArraysIndirect);
        {
            cmd->id         = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->mode       = renderState_.drawMode;
            cmd->indirect   = reinterpret_cast<const GLvoid*>(indirect);
            cmd->drawcount  = static_cast<GLsizei>(numCommands);
            cmd->stride     = static_cast<GLsizei>(stride);
        }
    }
    else
    #endif // /__APPLE__
    {
        auto cmd = AllocCommand<GLCmdDrawArraysIndirect>(GLOpCodeDrawArraysIndirect);
        {
            cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->numCommands    = numCommands;
            cmd->mode           = renderState_.drawMode;
            cmd->indirect       = static_cast<GLintptr>(offset);
            cmd->stride         = stride;
        }
    }
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto cmd = AllocCommand<GLCmdDrawElementsIndirect>(GLOpCodeDrawElementsIndirect);
    {
        cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
        cmd->numCommands    = 1;
        cmd->mode           = renderState_.drawMode;
        cmd->type           = renderState_.indexBufferDataType;
        cmd->indirect       = static_cast<GLintptr>(offset);
        cmd->stride         = 0;
    }
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    #ifndef __APPLE__
    if (HasExtension(GLExt::ARB_multi_draw_indirect))
    {
        const GLintptr indirect = static_cast<GLintptr>(offset);
        auto cmd = AllocCommand<GLCmdMultiDrawElementsIndirect>(GLOpCodeMultiDrawElementsIndirect);
        {
            cmd->id         = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->mode       = renderState_.drawMode;
            cmd->type       = renderState_.indexBufferDataType;
            cmd->indirect   = reinterpret_cast<const GLvoid*>(indirect);
            cmd->drawcount  = static_cast<GLsizei>(numCommands);
            cmd->stride     = static_cast<GLsizei>(stride);
        }
    }
    else
    #endif // /__APPLE__
    {
        auto cmd = AllocCommand<GLCmdDrawElementsIndirect>(GLOpCodeDrawElementsIndirect);
        {
            cmd->id             = LLGL_CAST(GLBuffer&, buffer).GetID();
            cmd->numCommands    = numCommands;
            cmd->mode           = renderState_.drawMode;
            cmd->type           = renderState_.indexBufferDataType;
            cmd->indirect       = static_cast<GLintptr>(offset);
            cmd->stride         = stride;
        }
    }
}

/* ----- Compute ----- */

void GLDeferredCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDispatchCompute>(GLOpCodeDispatchCompute);
    {
        cmd->numgroups[0] = numWorkGroupsX;
        cmd->numgroups[1] = numWorkGroupsY;
        cmd->numgroups[2] = numWorkGroupsZ;
    }
    #else
    ErrUnsupportedGLProc("glDispatchCompute");
    #endif
}

void GLDeferredCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    #ifndef __APPLE__
    auto cmd = AllocCommand<GLCmdDispatchComputeIndirect>(GLOpCodeDispatchComputeIndirect);
    {
        cmd->id         = LLGL_CAST(const GLBuffer&, buffer).GetID();
        cmd->indirect   = static_cast<GLintptr>(offset);
    }
    #else
    ErrUnsupportedGLProc("glDispatchComputeIndirect");
    #endif
}

/* ----- Direct Resource Access ------ */

void GLDeferredCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::UNIFORM_BUFFER, buffer, slot);
}

void GLDeferredCommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLDeferredCommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*stageFlags*/)
{
    SetGenericBuffer(GLBufferTarget::SHADER_STORAGE_BUFFER, buffer, slot);
}

void GLDeferredCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*stageFlags*/)
{
    auto cmd = AllocCommand<GLCmdBindTexture>(GLOpCodeBindTexture);
    {
        cmd->slot       = slot;
        cmd->texture    = LLGL_CAST(const GLTexture*, &texture);
    }
}

void GLDeferredCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*stageFlags*/)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    auto cmd = AllocCommand<GLCmdBindSampler>(GLOpCodeBindSampler);
    {
        cmd->slot       = slot;
        cmd->sampler    = samplerGL.GetID();
    }
}

void GLDeferredCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                /*stageFlags*/)
{
    GLCmdUnbindResources cmd;
    cmd.resetFlags = 0;

    cmd.first = static_cast<GLuint>(std::min(firstSlot, GLStateManager::g_maxNumResourceSlots - 1u));
    cmd.count = static_cast<GLsizei>(std::min(numSlots, GLStateManager::g_maxNumResourceSlots - cmd.first));

    if (cmd.count > 0)
    {
        switch (resourceType)
        {
            case ResourceType::Undefined:
            break;

            case ResourceType::Buffer:
            {
                if ((bindFlags & BindFlags::ConstantBuffer) != 0)
                    cmd.resetUBO = 1;
                if ((bindFlags & (BindFlags::SampleBuffer | BindFlags::RWStorageBuffer)) != 0)
                    cmd.resetSSAO = 1;
                if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
                    cmd.resetTransformFeedback = 1;
            }
            break;

            case ResourceType::Texture:
            {
                if ((bindFlags & BindFlags::SampleBuffer) != 0)
                    cmd.resetTextures = 1;
                if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
                    cmd.resetImages = 1;
            }
            break;

            case ResourceType::Sampler:
            {
                cmd.resetSamplers = 1;
            }
            break;
        }

        if (cmd.resetFlags != 0)
            *AllocCommand<GLCmdUnbindResources>(GLOpCodeUnbindResources) = cmd;
    }
}


/*
 * ======= Private: =======
 */

void GLDeferredCommandBuffer::SetGenericBuffer(const GLBufferTarget bufferTarget, Buffer& buffer, std::uint32_t slot)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    auto cmd = AllocCommand<GLCmdBindBufferBase>(GLOpCodeBindBufferBase);
    {
        cmd->target = bufferTarget;
        cmd->index  = slot;
        cmd->id     = bufferGL.GetID();
    }
}

void GLDeferredCommandBuffer::SetGenericBufferArray(const GLBufferTarget bufferTarget, BufferArray& bufferArray, std::uint32_t startSlot)
{
    auto& bufferArrayGL = LLGL_CAST(GLBufferArray&, bufferArray);
    auto count = bufferArrayGL.GetIDArray().size();
    auto cmd = AllocCommand<GLCmdBindBuffersBase>(GLOpCodeBindBuffersBase, sizeof(GLuint)*count);
    {
        cmd->target = bufferTarget;
        cmd->first  = startSlot;
        cmd->count  = static_cast<GLsizei>(count);
        ::memcpy(cmd + 1, bufferArrayGL.GetIDArray().data(), sizeof(GLuint)*count);
    }
}

void GLDeferredCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap)
{
    auto cmd = AllocCommand<GLCmdBindResourceHeap>(GLOpCodeBindResourceHeap);
    cmd->resourceHeap = LLGL_CAST(GLResourceHeap*, &resourceHeap);
}

void GLDeferredCommandBuffer::AllocOpCode(const GLOpCode opcode)
{
    buffer_.push_back(opcode);
}

template <typename T>
T* GLDeferredCommandBuffer::AllocCommand(const GLOpCode opcode, std::size_t extraSize)
{
    /* Resize internal buffer for opcode, command structure, and extra size */
    auto offset = buffer_.size();
    {
        buffer_.resize(offset + sizeof(opcode) + sizeof(T) + extraSize);
        buffer_[offset] = opcode;
    }
    return reinterpret_cast<T*>(&(buffer_[offset + sizeof(opcode)]));
}


} // /namespace LLGL



// ================================================================================
