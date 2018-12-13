/*
 * GLDeferredCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLDeferredCommandBuffer.h"
#include "GLCommand.h"

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


GLDeferredCommandBuffer::GLDeferredCommandBuffer(std::size_t reservedSize)
{
    buffer_.reserve(reservedSize);
}

bool GLDeferredCommandBuffer::IsImmediateCmdBuffer() const
{
    return false;
}

/* ----- Encoding ----- */

void GLDeferredCommandBuffer::Begin()
{
    // dummy
}

void GLDeferredCommandBuffer::End()
{
    // dummy
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
    //TODO
}

void GLDeferredCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    //TODO
}

void GLDeferredCommandBuffer::SetScissor(const Scissor& scissor)
{
    //TODO
}

void GLDeferredCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    //TODO
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
    cmd->depth = depth;

    /* Store as default clear value */
    clearValue_.depth = depth;
}

void GLDeferredCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    /* Submit clear value to GL */
    glClearStencil(static_cast<GLint>(stencil));

    /* Store as default clear value */
    clearValue_.stencil = static_cast<GLint>(stencil);
}

void GLDeferredCommandBuffer::Clear(long flags)
{
    //TODO
}

void GLDeferredCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //TODO
}

/* ----- Input Assembly ------ */

void GLDeferredCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if ((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpCodeBindVertexArray);
        cmd->vao = LLGL_CAST(const GLBufferWithVAO*, &buffer)->GetVaoID();
    }
}

void GLDeferredCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if ((bufferArray.GetBindFlags() & BindFlags::VertexBuffer) != 0)
    {
        auto cmd = AllocCommand<GLCmdBindVertexArray>(GLOpCodeBindVertexArray);
        cmd->vao = LLGL_CAST(const GLBufferArrayWithVAO*, &bufferArray)->GetVaoID();
    }
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    //TODO
}

void GLDeferredCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    //TODO
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
    //TODO
}

void GLDeferredCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*startSlot*/)
{
    //TODO
}

/* ----- Render Passes ----- */

void GLDeferredCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    //TODO
}

void GLDeferredCommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void GLDeferredCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    //TODO
}

void GLDeferredCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //TODO
}

/* ----- Queries ----- */

void GLDeferredCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //TODO
}

void GLDeferredCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //TODO
}

void GLDeferredCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //TODO
}

void GLDeferredCommandBuffer::EndRenderCondition()
{
    AllocOpCode(GLOpCodeEndConditionalRender);
}

/* ----- Drawing ----- */

/*
NOTE:
In the following Draw* functions, 'indices' is from type 'GLsizeiptr' to have the same size as a pointer address on either a 32-bit or 64-bit platform.
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
    const GLsizeiptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
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
    const GLsizeiptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
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
    //TODO
    /*const GLsizeiptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstanced(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances)
    );*/
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    //TODO
    /*const GLsizeiptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertex(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset
    );*/
}

void GLDeferredCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    //TODO
    #ifndef __APPLE__
    /*const GLsizeiptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
    glDrawElementsInstancedBaseVertexBaseInstance(
        renderState_.drawMode,
        static_cast<GLsizei>(numIndices),
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indices),
        static_cast<GLsizei>(numInstances),
        vertexOffset,
        firstInstance
    );*/
    #else
    ErrUnsupportedGLProc("glDrawElementsInstancedBaseVertexBaseInstance");
    #endif
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    //TODO
    /*auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    glDrawArraysIndirect(
        renderState_.drawMode,
        reinterpret_cast<const GLvoid*>(indirect)
    );*/
}

void GLDeferredCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    //TODO
    #if 0
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
    #endif
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    //TODO
    /*auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    stateMngr_->BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, bufferGL.GetID());

    const GLsizeiptr indirect = static_cast<GLsizeiptr>(offset);
    glDrawElementsIndirect(
        renderState_.drawMode,
        renderState_.indexBufferDataType,
        reinterpret_cast<const GLvoid*>(indirect)
    );*/
}

void GLDeferredCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    //TODO
    #if 0
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
    #endif
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
        cmd->id         = LLGL_CAST(const GLBuffer*, &buffer)->GetID();
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
    //TODO
}

void GLDeferredCommandBuffer::Execute(GLStateManager& stateMngr)
{
    GLOpCode opcode;

    /* Initialize program counter to execute virtual GL commands */
    auto pc = buffer_.data();
    auto pcEnd = buffer_.data() + buffer_.size();

    while (pc < pcEnd)
    {
        /* Read opcode */
        opcode = *reinterpret_cast<const GLOpCode*>(pc);
        pc += sizeof(GLOpCode);

        /* Execute command and increment program counter */
        pc += ExecuteCommand(opcode, pc, stateMngr);
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
    auto cmd = AllocCommand<GLCmdBindBuffersBase>(GLOpCodeBindBuffersBase, count * sizeof(GLuint));
    {
        cmd->target = bufferTarget;
        cmd->first  = startSlot;
        cmd->count  = static_cast<GLsizei>(count);
        ::memcpy(cmd + 1, bufferArrayGL.GetIDArray().data(), count * sizeof(GLuint));
    }
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

std::size_t GLDeferredCommandBuffer::ExecuteCommand(const GLOpCode opcode, const void* pc, GLStateManager& stateMngr)
{
    switch (opcode)
    {
        case GLOpCodeUpdateBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdUpdateBuffer*>(pc);
            cmd->buffer->BufferSubData(cmd->offset, cmd->size, cmd + 1);
            return sizeof(cmd) + cmd->size;
        }
        case GLOpCodeCopyBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyBuffer*>(pc);
            cmd->writeBuffer->CopyBufferSubData(*(cmd->readBuffer), cmd->readOffset, cmd->writeOffset, cmd->size);
            return sizeof(cmd);
        }
        case GLOpCodeSetAPIDepState:
        {
            auto cmd = reinterpret_cast<const GLCmdSetAPIDepState*>(pc);
            stateMngr.SetGraphicsAPIDependentState(cmd->desc);
            return sizeof(cmd);
        }
        case GLOpCodeViewport:
        {
            auto cmd = reinterpret_cast<const GLCmdViewport*>(pc);
            {
                GLViewport viewport = cmd->viewport;
                stateMngr.SetViewport(viewport);

                GLDepthRange depthRange = cmd->depthRange;
                stateMngr.SetDepthRange(depthRange);
            }
            return sizeof(cmd);
        }
        case GLOpCodeViewportArray:
        {
            auto cmd = reinterpret_cast<const GLCmdViewportArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                union
                {
                    GLViewport viewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                    GLDepthRange depthRanges[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                };

                ::memcpy(viewports, cmdData, sizeof(GLViewport) * cmd->count);
                stateMngr.SetViewportArray(cmd->first, cmd->count, viewports);

                ::memcpy(depthRanges, cmdData + sizeof(GLViewport) * cmd->count, sizeof(GLDepthRange) * cmd->count);
                stateMngr.SetDepthRangeArray(cmd->first, cmd->count, depthRanges);
            }
            return sizeof(cmd);
        }
        case GLOpCodeScissor:
        {
            auto cmd = reinterpret_cast<const GLCmdScissor*>(pc);
            {
                GLScissor scissor = cmd->scissor;
                stateMngr.SetScissor(scissor);
            }
            return sizeof(cmd);
        }
        case GLOpCodeScissorArray:
        {
            auto cmd = reinterpret_cast<const GLCmdScissorArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                GLScissor scissors[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                ::memcpy(scissors, cmdData, sizeof(GLScissor) * cmd->count);
                stateMngr.SetScissorArray(cmd->first, cmd->count, scissors);
            }
            return sizeof(cmd);
        }
        case GLOpCodeClearColor:
        {
            auto cmd = reinterpret_cast<const GLCmdClearColor*>(pc);
            glClearColor(cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3]);
            return sizeof(cmd);
        }
        case GLOpCodeClearDepth:
        {
            auto cmd = reinterpret_cast<const GLCmdClearDepth*>(pc);
            glClearDepth(cmd->depth);
            return sizeof(cmd);
        }
        case GLOpCodeClearStencil:
        {
            auto cmd = reinterpret_cast<const GLCmdClearStencil*>(pc);
            glClearStencil(cmd->stencil);
            return sizeof(cmd);
        }
        case GLOpCodeClear:
        {
            auto cmd = reinterpret_cast<const GLCmdClear*>(pc);
            {
                auto mask = cmd->mask;

                /* Setup GL clear mask and clear respective buffer */
                if ((mask & GL_COLOR_BUFFER_BIT) != 0)
                    stateMngr.PushColorMaskAndEnable();
                if ((mask & GL_DEPTH_BUFFER_BIT) != 0)
                    stateMngr.PushDepthMaskAndEnable();
                #if 0//TODO
                if ((mask & GL_STENCIL_BUFFER_BIT) != 0)
                    stateMngr.SetStencilMask(GL_TRUE);
                #endif

                /* Clear buffers */
                glClear(mask);

                /* Restore framebuffer masks */
                #if 0//TODO
                if ((mask & GL_STENCIL_BUFFER_BIT) != 0)
                    stateMngr.PopStencilMask();
                #endif
                if ((mask & GL_COLOR_BUFFER_BIT) != 0)
                    stateMngr.PopDepthMask();
                if ((mask & GL_DEPTH_BUFFER_BIT) != 0)
                    stateMngr.PopColorMask();
            }
            return sizeof(cmd);
        }
        case GLOpCodeClearBuffers:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBuffers*>(pc);
            //TODO...
            return sizeof(cmd);
        }
        case GLOpCodeBindVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindVertexArray*>(pc);
            stateMngr.BindVertexArray(cmd->vao);
            return sizeof(cmd);
        }
        case GLOpCodeBindElementArrayBufferToVAO:
        {
            auto cmd = reinterpret_cast<const GLCmdBindElementArrayBufferToVAO*>(pc);
            stateMngr.BindElementArrayBufferToVAO(cmd->id);
            return sizeof(cmd);
        }
        case GLOpCodeBindBufferBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBufferBase*>(pc);
            stateMngr.BindBufferBase(cmd->target, cmd->index, cmd->id);
            return sizeof(cmd);
        }
        case GLOpCodeBindBuffersBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBuffersBase*>(pc);
            stateMngr.BindBuffersBase(cmd->target, cmd->first, cmd->count, reinterpret_cast<const GLuint*>(cmd + 1));
            return sizeof(cmd) + cmd->count;
        }
        case GLOpCodeBeginTransformFeedback:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedback*>(pc);
            glBeginTransformFeedback(cmd->primitiveMove);
            return sizeof(cmd);
        }
        case GLOpCodeBeginTransformFeedbackNV:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedbackNV*>(pc);
            glBeginTransformFeedbackNV(cmd->primitiveMove);
            return sizeof(cmd);
        }
        case GLOpCodeEndTransformFeedback:
        {
            glEndTransformFeedback();
            return 0;
        }
        case GLOpCodeEndTransformFeedbackNV:
        {
            glEndTransformFeedbackNV();
            return 0;
        }
        case GLOpCodeBindResourceHeap:
        {
            auto cmd = reinterpret_cast<const GLCmdBindResourceHeap*>(pc);
            cmd->resourceHeap->Bind(stateMngr);
            return sizeof(cmd);
        }
        case GLOpCodeBindRenderContext:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderContext*>(pc);
            //TODO...
            return sizeof(cmd);
        }
        case GLOpCodeBindRenderTarget:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderTarget*>(pc);
            //TODO...
            return sizeof(cmd);
        }
        case GLOpCodeClearAttachmentsWithRenderPass:
        {
            auto cmd = reinterpret_cast<const GLCmdClearAttachmentsWithRenderPass*>(pc);
            //TODO...
            return sizeof(cmd);
        }
        case GLOpCodeBindGraphicsPipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGraphicsPipeline*>(pc);
            cmd->graphicsPipeline->Bind(stateMngr);
            return sizeof(cmd);
        }
        case GLOpCodeBindComputePipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindComputePipeline*>(pc);
            cmd->computePipeline->Bind(stateMngr);
            return sizeof(cmd);
        }
        case GLOpCodeBeginQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginQuery*>(pc);
            cmd->queryHeap->Begin(cmd->query);
            return sizeof(cmd);
        }
        case GLOpCodeEndQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdEndQuery*>(pc);
            cmd->queryHeap->End(cmd->query);
            return sizeof(cmd);
        }
        case GLOpCodeBeginConditionalRender:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginConditionalRender*>(pc);
            glBeginConditionalRender(cmd->id, cmd->mode);
            return sizeof(cmd);
        }
        case GLOpCodeEndConditionalRender:
        {
            glEndConditionalRender();
            return 0;
        }
        case GLOpCodeDrawArrays:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArrays*>(pc);
            glDrawArrays(cmd->mode, cmd->first, cmd->count);
            return sizeof(cmd);
        }
        case GLOpCodeDrawArraysInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstanced*>(pc);
            glDrawArraysInstanced(cmd->mode, cmd->first, cmd->count, cmd->instancecount);
            return sizeof(cmd);
        }
        case GLOpCodeDrawArraysInstancedBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstancedBaseInstance*>(pc);
            glDrawArraysInstancedBaseInstance(cmd->mode, cmd->first, cmd->count, cmd->instancecount, cmd->baseinstance);
            return sizeof(cmd);
        }
        case GLOpCodeDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysIndirect*>(pc);
            stateMngr.BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            GLsizeiptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                glDrawArraysIndirect(cmd->mode, reinterpret_cast<const void*>(offset));
                offset += cmd->stride;
            }
            return sizeof(cmd);
        }
        case GLOpCodeDrawElements:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElements*>(pc);
            glDrawElements(cmd->mode, cmd->count, cmd->type, cmd->indices);
            return sizeof(cmd);
        }
        case GLOpCodeDrawElementsBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsBaseVertex*>(pc);
            glDrawElementsBaseVertex(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->basevertex);
            return sizeof(cmd);
        }
        case GLOpCodeDrawElementsInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstanced*>(pc);
            glDrawElementsInstanced(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount);
            return sizeof(cmd);
        }
        case GLOpCodeDrawElementsInstancedBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertex*>(pc);
            glDrawElementsInstancedBaseVertex(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex);
            return sizeof(cmd);
        }
        case GLOpCodeDrawElementsInstancedBaseVertexBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertexBaseInstance*>(pc);
            glDrawElementsInstancedBaseVertexBaseInstance(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex, cmd->baseinstance);
            return sizeof(cmd);
        }
        case GLOpCodeDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsIndirect*>(pc);
            stateMngr.BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            GLsizeiptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                glDrawElementsIndirect(cmd->mode, cmd->type, reinterpret_cast<const void*>(offset));
                offset += cmd->stride;
            }
            return sizeof(cmd);
        }
        case GLOpCodeMultiDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawArraysIndirect*>(pc);
            stateMngr.BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            glMultiDrawArraysIndirect(cmd->mode, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(cmd);
        }
        case GLOpCodeMultiDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawElementsIndirect*>(pc);
            stateMngr.BindBuffer(GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            glMultiDrawElementsIndirect(cmd->mode, cmd->type, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(cmd);
        }
        case GLOpCodeDispatchCompute:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchCompute*>(pc);
            glDispatchCompute(cmd->numgroups[0], cmd->numgroups[1], cmd->numgroups[2]);
            return sizeof(cmd);
        }
        case GLOpCodeDispatchComputeIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchComputeIndirect*>(pc);
            stateMngr.BindBuffer(GLBufferTarget::DISPATCH_INDIRECT_BUFFER, cmd->id);
            glDispatchComputeIndirect(cmd->indirect);
            return sizeof(cmd);
        }
        case GLOpCodeBindTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindTexture*>(pc);
            stateMngr.ActiveTexture(cmd->slot);
            stateMngr.BindTexture(*(cmd->texture));
            return sizeof(cmd);
        }
        case GLOpCodeBindSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindSampler*>(pc);
            stateMngr.BindSampler(cmd->slot, cmd->sampler);
            return sizeof(cmd);
        }
        case GLOpCodeResetResources:
        {
            auto cmd = reinterpret_cast<const GLCmdResetResources*>(pc);
            //TODO...
            return sizeof(cmd);
        }
        default:
            return 0;
    }
}


} // /namespace LLGL



// ================================================================================
