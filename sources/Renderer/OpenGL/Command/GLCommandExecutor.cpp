/*
 * GLCommandExecutor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCommandExecutor.h"
#include "GLCommand.h"
#include "GLDeferredCommandBuffer.h"

#include "../GLSwapChain.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"
#include "../Shader/GLShaderUniform.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"
#include "../Texture/GLMipGenerator.h"
#include "../Texture/GLFramebufferCapture.h"

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLPipelineState.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <algorithm>
#include <string.h>

#ifdef LLGL_ENABLE_JIT_COMPILER
#   include "../../../JIT/JITProgram.h"
#endif

#include <LLGL/Backend/OpenGL/NativeCommand.h>


namespace LLGL
{


static std::size_t ExecuteGLCommand(const GLOpcode opcode, const void* pc, GLStateManager*& stateMngr)
{
    switch (opcode)
    {
        case GLOpcodeBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdBufferSubData*>(pc);
            cmd->buffer->BufferSubData(cmd->offset, cmd->size, cmd + 1);
            return (sizeof(*cmd) + cmd->size);
        }
        case GLOpcodeCopyBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyBufferSubData*>(pc);
            cmd->writeBuffer->CopyBufferSubData(*(cmd->readBuffer), cmd->readOffset, cmd->writeOffset, cmd->size);
            return sizeof(*cmd);
        }
        case GLOpcodeClearBufferData:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBufferData*>(pc);
            cmd->buffer->ClearBufferData(cmd->data);
            return sizeof(*cmd);
        }
        case GLOpcodeClearBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBufferSubData*>(pc);
            cmd->buffer->ClearBufferSubData(cmd->offset, cmd->size, cmd->data);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageSubData*>(pc);
            cmd->dstTexture->CopyImageSubData(cmd->dstLevel, cmd->dstOffset, *(cmd->srcTexture), cmd->srcLevel, cmd->srcOffset, cmd->extent);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageToBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageBuffer*>(pc);
            cmd->texture->CopyImageToBuffer(cmd->region, cmd->bufferID, cmd->offset, cmd->size, cmd->rowLength, cmd->imageHeight);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageFromBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageBuffer*>(pc);
            cmd->texture->CopyImageFromBuffer(cmd->region, cmd->bufferID, cmd->offset, cmd->size, cmd->rowLength, cmd->imageHeight);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyFramebufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyFramebufferSubData*>(pc);
            GLFramebufferCapture::Get().CaptureFramebuffer(*stateMngr, *(cmd->dstTexture), cmd->dstLevel, cmd->dstOffset, cmd->srcOffset, cmd->extent);
            return sizeof(*cmd);
        }
        case GLOpcodeGenerateMipmap:
        {
            auto cmd = reinterpret_cast<const GLCmdGenerateMipmap*>(pc);
            GLMipGenerator::Get().GenerateMipsForTexture(*stateMngr, *(cmd->texture));
            return sizeof(*cmd);
        }
        case GLOpcodeGenerateMipmapSubresource:
        {
            auto cmd = reinterpret_cast<const GLCmdGenerateMipmapSubresource*>(pc);
            GLMipGenerator::Get().GenerateMipsRangeForTexture(*stateMngr, *(cmd->texture), cmd->baseMipLevel, cmd->numMipLevels, cmd->baseArrayLayer, cmd->numArrayLayers);
            return sizeof(*cmd);
        }
        case GLOpcodeExecute:
        {
            auto cmd = reinterpret_cast<const GLCmdExecute*>(pc);
            ExecuteGLDeferredCommandBuffer(*(cmd->commandBuffer), *stateMngr);
            return sizeof(*cmd);
        }
        case GLOpcodeViewport:
        {
            auto cmd = reinterpret_cast<const GLCmdViewport*>(pc);
            {
                stateMngr->SetViewport(cmd->viewport);
                stateMngr->SetDepthRange(cmd->depthRange);
            }
            return sizeof(*cmd);
        }
        case GLOpcodeViewportArray:
        {
            auto cmd = reinterpret_cast<const GLCmdViewportArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                stateMngr->SetViewportArray(cmd->first, cmd->count, reinterpret_cast<const GLViewport*>(cmdData));
                stateMngr->SetDepthRangeArray(cmd->first, cmd->count, reinterpret_cast<const GLDepthRange*>(cmdData + sizeof(GLViewport)*cmd->count));
            }
            return (sizeof(*cmd) + sizeof(GLViewport)*cmd->count + sizeof(GLDepthRange)*cmd->count);
        }
        case GLOpcodeScissor:
        {
            auto cmd = reinterpret_cast<const GLCmdScissor*>(pc);
            {
                stateMngr->SetScissor(cmd->scissor);
            }
            return sizeof(*cmd);
        }
        case GLOpcodeScissorArray:
        {
            auto cmd = reinterpret_cast<const GLCmdScissorArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                stateMngr->SetScissorArray(cmd->first, cmd->count, reinterpret_cast<const GLScissor*>(cmdData));
            }
            return (sizeof(*cmd) + sizeof(GLScissor)*cmd->count);
        }
        case GLOpcodeClearColor:
        {
            auto cmd = reinterpret_cast<const GLCmdClearColor*>(pc);
            glClearColor(cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3]);
            return sizeof(*cmd);
        }
        case GLOpcodeClearDepth:
        {
            auto cmd = reinterpret_cast<const GLCmdClearDepth*>(pc);
            GLProfile::ClearDepth(cmd->depth);
            return sizeof(*cmd);
        }
        case GLOpcodeClearStencil:
        {
            auto cmd = reinterpret_cast<const GLCmdClearStencil*>(pc);
            glClearStencil(cmd->stencil);
            return sizeof(*cmd);
        }
        case GLOpcodeClear:
        {
            auto cmd = reinterpret_cast<const GLCmdClear*>(pc);
            stateMngr->Clear(cmd->flags);
            return sizeof(*cmd);
        }
        case GLOpcodeClearAttachmentsWithRenderPass:
        {
            auto cmd = reinterpret_cast<const GLCmdClearAttachmentsWithRenderPass*>(pc);
            if (cmd->renderPass != nullptr)
                stateMngr->ClearAttachmentsWithRenderPass(*(cmd->renderPass), cmd->numClearValues, reinterpret_cast<const ClearValue*>(cmd + 1));
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case GLOpcodeClearBuffers:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBuffers*>(pc);
            stateMngr->ClearBuffers(cmd->numAttachments, reinterpret_cast<const AttachmentClear*>(cmd + 1));
            return (sizeof(*cmd) + sizeof(AttachmentClear)*cmd->numAttachments);
        }
        case GLOpcodeBindVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindVertexArray*>(pc);
            stateMngr->BindVertexArray(cmd->vao);
            return sizeof(*cmd);
        }
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        case GLOpcodeBindGL2XVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGL2XVertexArray*>(pc);
            cmd->vertexArrayGL2X->Bind(*stateMngr);
            return sizeof(*cmd);
        }
        #endif
        case GLOpcodeBindElementArrayBufferToVAO:
        {
            auto cmd = reinterpret_cast<const GLCmdBindElementArrayBufferToVAO*>(pc);
            stateMngr->BindElementArrayBufferToVAO(cmd->id, cmd->indexType16Bits);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBufferBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBufferBase*>(pc);
            stateMngr->BindBufferBase(cmd->target, cmd->index, cmd->id);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBuffersBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBuffersBase*>(pc);
            stateMngr->BindBuffersBase(cmd->target, cmd->first, cmd->count, reinterpret_cast<const GLuint*>(cmd + 1));
            return (sizeof(*cmd) + sizeof(GLuint)*cmd->count);
        }
        case GLOpcodeBeginTransformFeedback:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedback*>(pc);
            glBeginTransformFeedback(cmd->primitiveMove);
            return sizeof(*cmd);
        }
        case GLOpcodeBeginTransformFeedbackNV:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedbackNV*>(pc);
            #ifdef GL_NV_transform_feedback
            glBeginTransformFeedbackNV(cmd->primitiveMove);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeEndTransformFeedback:
        {
            glEndTransformFeedback();
            return 0;
        }
        case GLOpcodeEndTransformFeedbackNV:
        {
            #ifdef GL_NV_transform_feedback
            glEndTransformFeedbackNV();
            #endif
            return 0;
        }
        case GLOpcodeBindResourceHeap:
        {
            auto cmd = reinterpret_cast<const GLCmdBindResourceHeap*>(pc);
            cmd->resourceHeap->Bind(*stateMngr, cmd->descriptorSet);
            return sizeof(*cmd);
        }
        case GLOpcodeBindRenderTarget:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderTarget*>(pc);
            GLStateManager* nextStateMngr = stateMngr;
            stateMngr->BindRenderTarget(*(cmd->renderTarget), &nextStateMngr);
            stateMngr = nextStateMngr;
            return sizeof(*cmd);
        }
        case GLOpcodeBindPipelineState:
        {
            auto cmd = reinterpret_cast<const GLCmdBindPipelineState*>(pc);
            cmd->pipelineState->Bind(*stateMngr);
            return sizeof(*cmd);
        }
        case GLOpcodeSetBlendColor:
        {
            auto cmd = reinterpret_cast<const GLCmdSetBlendColor*>(pc);
            stateMngr->SetBlendColor(cmd->color);
            return sizeof(*cmd);
        }
        case GLOpcodeSetStencilRef:
        {
            auto cmd = reinterpret_cast<const GLCmdSetStencilRef*>(pc);
            stateMngr->SetStencilRef(cmd->ref, cmd->face);
            return sizeof(*cmd);
        }
        case GLOpcodeSetUniforms:
        {
            auto cmd = reinterpret_cast<const GLCmdSetUniforms*>(pc);
            GLSetUniformsByType(cmd->type, cmd->location, cmd->count, (cmd + 1));
            return (sizeof(*cmd) + cmd->size);
        }
        case GLOpcodeBeginQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginQuery*>(pc);
            cmd->queryHeap->Begin(cmd->query);
            return sizeof(*cmd);
        }
        case GLOpcodeEndQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdEndQuery*>(pc);
            cmd->queryHeap->End();
            return sizeof(*cmd);
        }
        case GLOpcodeBeginConditionalRender:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginConditionalRender*>(pc);
            #ifdef LLGL_GLEXT_CONDITIONAL_RENDER
            glBeginConditionalRender(cmd->id, cmd->mode);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeEndConditionalRender:
        {
            #ifdef LLGL_GLEXT_CONDITIONAL_RENDER
            glEndConditionalRender();
            #endif
            return 0;
        }
        case GLOpcodeDrawArrays:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArrays*>(pc);
            glDrawArrays(cmd->mode, cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawArraysInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstanced*>(pc);
            glDrawArraysInstanced(cmd->mode, cmd->first, cmd->count, cmd->instancecount);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawArraysInstancedBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstancedBaseInstance*>(pc);
            #ifdef LLGL_GLEXT_BASE_INSTANCE
            glDrawArraysInstancedBaseInstance(cmd->mode, cmd->first, cmd->count, cmd->instancecount, cmd->baseinstance);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysIndirect*>(pc);
            #ifdef LLGL_GLEXT_DRAW_INDIRECT
            stateMngr->BindBuffer(GLBufferTarget::DrawIndirectBuffer, cmd->id);
            GLintptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                glDrawArraysIndirect(cmd->mode, reinterpret_cast<const GLvoid*>(offset));
                offset += cmd->stride;
            }
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElements:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElements*>(pc);
            glDrawElements(cmd->mode, cmd->count, cmd->type, cmd->indices);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsBaseVertex*>(pc);
            #ifdef LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
            glDrawElementsBaseVertex(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->basevertex);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstanced*>(pc);
            glDrawElementsInstanced(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsInstancedBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertex*>(pc);
            #ifdef LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
            glDrawElementsInstancedBaseVertex(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsInstancedBaseVertexBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertexBaseInstance*>(pc);
            #ifdef LLGL_GLEXT_BASE_INSTANCE
            glDrawElementsInstancedBaseVertexBaseInstance(cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex, cmd->baseinstance);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsIndirect*>(pc);
            #ifdef LLGL_GLEXT_DRAW_INDIRECT
            stateMngr->BindBuffer(GLBufferTarget::DrawIndirectBuffer, cmd->id);
            GLintptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                glDrawElementsIndirect(cmd->mode, cmd->type, reinterpret_cast<const GLvoid*>(offset));
                offset += cmd->stride;
            }
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeMultiDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawArraysIndirect*>(pc);
            #ifdef LLGL_GLEXT_MULTI_DRAW_INDIRECT
            stateMngr->BindBuffer(GLBufferTarget::DrawIndirectBuffer, cmd->id);
            glMultiDrawArraysIndirect(cmd->mode, cmd->indirect, cmd->drawcount, cmd->stride);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeMultiDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawElementsIndirect*>(pc);
            #ifdef LLGL_GLEXT_MULTI_DRAW_INDIRECT
            stateMngr->BindBuffer(GLBufferTarget::DrawIndirectBuffer, cmd->id);
            glMultiDrawElementsIndirect(cmd->mode, cmd->type, cmd->indirect, cmd->drawcount, cmd->stride);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDispatchCompute:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchCompute*>(pc);
            #ifdef LLGL_GLEXT_COMPUTE_SHADER
            glDispatchCompute(cmd->numgroups[0], cmd->numgroups[1], cmd->numgroups[2]);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeDispatchComputeIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchComputeIndirect*>(pc);
            #ifdef LLGL_GLEXT_COMPUTE_SHADER
            stateMngr->BindBuffer(GLBufferTarget::DispatchIndirectBuffer, cmd->id);
            glDispatchComputeIndirect(cmd->indirect);
            #endif
            return sizeof(*cmd);
        }
        case GLOpcodeBindTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindTexture*>(pc);
            stateMngr->ActiveTexture(cmd->slot);
            stateMngr->BindGLTexture(*(cmd->texture));
            return sizeof(*cmd);
        }
        case GLOpcodeBindImageTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindImageTexture*>(pc);
            stateMngr->BindImageTexture(cmd->unit, cmd->level, cmd->format, cmd->texture);
            return sizeof(*cmd);
        }
        case GLOpcodeBindSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindSampler*>(pc);
            stateMngr->BindSampler(cmd->layer, cmd->sampler);
            return sizeof(*cmd);
        }
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        case GLOpcodeBindGL2XSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGL2XSampler*>(pc);
            stateMngr->BindGL2XSampler(cmd->layer, *(cmd->samplerGL2X));
            return sizeof(*cmd);
        }
        #endif
        case GLOpcodeUnbindResources:
        {
            auto cmd = reinterpret_cast<const GLCmdUnbindResources*>(pc);
            if (cmd->resetUBO)
                stateMngr->UnbindBuffersBase(GLBufferTarget::UniformBuffer, cmd->first, cmd->count);
            if (cmd->resetSSAO)
                stateMngr->UnbindBuffersBase(GLBufferTarget::ShaderStorageBuffer, cmd->first, cmd->count);
            if (cmd->resetTransformFeedback)
                stateMngr->UnbindBuffersBase(GLBufferTarget::TransformFeedbackBuffer, cmd->first, cmd->count);
            if (cmd->resetTextures)
                stateMngr->UnbindTextures(cmd->first, cmd->count);
            if (cmd->resetImages)
                stateMngr->UnbindImageTextures(cmd->first, cmd->count);
            if (cmd->resetSamplers)
                stateMngr->UnbindSamplers(cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        case GLOpcodePushDebugGroup:
        {
            auto cmd = reinterpret_cast<const GLCmdPushDebugGroup*>(pc);
            #ifdef LLGL_GLEXT_DEBUG
            glPushDebugGroup(cmd->source, cmd->id, cmd->length, reinterpret_cast<const GLchar*>(cmd + 1));
            #endif
            return (sizeof(*cmd) + cmd->length + 1);
        }
        case GLOpcodePopDebugGroup:
        {
            #ifdef LLGL_GLEXT_DEBUG
            glPopDebugGroup();
            #endif
            return 0;
        }
        default:
            return 0;
    }
}

static void ExecuteGLCommandsEmulated(const GLVirtualCommandBuffer& virtualCmdBuffer, GLStateManager* stateMngr)
{
    /* Initialize program counter to execute virtual GL commands */
    for (const auto& chunk : virtualCmdBuffer)
    {
        auto pc     = chunk.data;
        auto pcEnd  = chunk.data + chunk.size;

        while (pc < pcEnd)
        {
            /* Read opcode */
            const GLOpcode opcode = *reinterpret_cast<const GLOpcode*>(pc);
            pc += sizeof(GLOpcode);

            /* Execute command and increment program counter */
            pc += ExecuteGLCommand(opcode, pc, stateMngr);
        }
    }
}

#ifdef LLGL_ENABLE_JIT_COMPILER

static void ExecuteGLCommandsNatively(const JITProgram& exec, GLStateManager& stateMngr)
{
    /* Execute native program and pass pointer to state manager */
    exec.GetEntryPoint()(&stateMngr);
}

#endif // /LLGL_ENABLE_JIT_COMPILER

void ExecuteGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdBuffer, GLStateManager& stateMngr)
{
    #ifdef LLGL_ENABLE_JIT_COMPILER
    if (auto exec = cmdBuffer.GetExecutable().get())
    {
        /* Execute GL commands with native executable */
        ExecuteGLCommandsNatively(*exec, stateMngr);
    }
    else
    #endif // /LLGL_ENABLE_JIT_COMPILER
    {
        /* Emulate execution of GL commands */
        ExecuteGLCommandsEmulated(cmdBuffer.GetVirtualCommandBuffer(), &stateMngr);
    }
}

void ExecuteGLCommandBuffer(const GLCommandBuffer& cmdBuffer, GLStateManager& stateMngr)
{
    /* Is this a secondary command buffer? */
    if (!cmdBuffer.IsImmediateCmdBuffer())
    {
        auto& deferredCmdBufferGL = LLGL_CAST(const GLDeferredCommandBuffer&, cmdBuffer);
        if (!deferredCmdBufferGL.IsPrimary())
        {
            /* Execute deferred command buffer */
            ExecuteGLDeferredCommandBuffer(deferredCmdBufferGL, stateMngr);
        }
    }
}

void ExecuteNativeGLCommand(const OpenGL::NativeCommand& cmd, GLStateManager& stateMngr)
{
    switch (cmd.type)
    {
        case OpenGL::NativeCommandType::ClearCache:
        {
            stateMngr.Reset();
        }
        break;

        default:
        break;
    }
}


} // /namespace LLGL



// ================================================================================
