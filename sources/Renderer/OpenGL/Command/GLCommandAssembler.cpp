/*
 * GLCommandAssembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_JIT_COMPILER

#include "GLCommandAssembler.h"
#include "GLCommandExecutor.h"
#include "GLCommand.h"
#include "GLDeferredCommandBuffer.h"
#include "../../../JIT/JITCompiler.h"

#include "../GLRenderContext.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionLoader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include "../Shader/GLShaderProgram.h"
#include "../Shader/GLShaderUniform.h"

#include "../Texture/GLTexture.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLRenderTarget.h"
#include "../Texture/GLMipGenerator.h"

#include "../Buffer/GLBufferWithVAO.h"
#include "../Buffer/GLBufferArrayWithVAO.h"

#include "../RenderState/GLStateManager.h"
#include "../RenderState/GLGraphicsPSO.h"
#include "../RenderState/GLResourceHeap.h"
#include "../RenderState/GLRenderPass.h"
#include "../RenderState/GLQueryHeap.h"

#include <LLGL/StaticLimits.h>
#include <algorithm>


namespace LLGL
{


static std::size_t AssembleGLCommand(const GLOpcode opcode, const void* pc, JITCompiler& compiler)
{
    /* Declare index of variadic argument of entry point */
    static const JITVarArg g_stateMngrArg{ 0 };

    /* Generate native CPU opcodes for emulated GLOpcode */
    switch (opcode)
    {
        case GLOpcodeBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdBufferSubData*>(pc);
            compiler.CallMember(&GLBuffer::BufferSubData, cmd->buffer, cmd->offset, cmd->size, (cmd + 1));
            return (sizeof(*cmd) + cmd->size);
        }
        case GLOpcodeCopyBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyBufferSubData*>(pc);
            compiler.CallMember(&GLBuffer::CopyBufferSubData, cmd->writeBuffer, cmd->readBuffer, cmd->readOffset, cmd->writeOffset, cmd->size);
            return sizeof(*cmd);
        }
        case GLOpcodeClearBufferData:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBufferData*>(pc);
            compiler.CallMember(&GLBuffer::ClearBufferData, cmd->buffer, cmd->data);
            return sizeof(*cmd);
        }
        case GLOpcodeClearBufferSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBufferSubData*>(pc);
            compiler.CallMember(&GLBuffer::ClearBufferSubData, cmd->buffer, cmd->offset, cmd->size, cmd->data);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageSubData:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageSubData*>(pc);
            compiler.CallMember(&GLTexture::CopyImageSubData, cmd->dstTexture, cmd->dstLevel, &(cmd->dstOffset), cmd->srcTexture, cmd->srcLevel, &(cmd->srcOffset), &(cmd->extent));
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageToBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageBuffer*>(pc);
            compiler.CallMember(&GLTexture::CopyImageToBuffer, cmd->texture, &(cmd->region), cmd->bufferID, cmd->offset, cmd->size, cmd->rowLength, cmd->imageHeight);
            return sizeof(*cmd);
        }
        case GLOpcodeCopyImageFromBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyImageBuffer*>(pc);
            compiler.CallMember(&GLTexture::CopyImageFromBuffer, cmd->texture, &(cmd->region), cmd->bufferID, cmd->offset, cmd->size, cmd->rowLength, cmd->imageHeight);
            return sizeof(*cmd);
        }
        case GLOpcodeGenerateMipmap:
        {
            auto cmd = reinterpret_cast<const GLCmdGenerateMipmap*>(pc);
            compiler.CallMember(&GLMipGenerator::GenerateMipsForTexture, &(GLMipGenerator::Get()), g_stateMngrArg, cmd->texture);
            return sizeof(*cmd);
        }
        case GLOpcodeGenerateMipmapSubresource:
        {
            auto cmd = reinterpret_cast<const GLCmdGenerateMipmapSubresource*>(pc);
            compiler.CallMember(&GLMipGenerator::GenerateMipsRangeForTexture, &(GLMipGenerator::Get()), g_stateMngrArg, cmd->texture, cmd->baseMipLevel, cmd->numMipLevels, cmd->baseArrayLayer, cmd->numArrayLayers);
            return sizeof(*cmd);
        }
        case GLOpcodeExecute:
        {
            auto cmd = reinterpret_cast<const GLCmdExecute*>(pc);
            compiler.Call(ExecuteGLDeferredCommandBuffer, cmd->commandBuffer, g_stateMngrArg);
            return sizeof(*cmd);
        }
        case GLOpcodeSetAPIDepState:
        {
            auto cmd = reinterpret_cast<const GLCmdSetAPIDepState*>(pc);
            compiler.CallMember(&GLStateManager::SetGraphicsAPIDependentState, g_stateMngrArg, &(cmd->desc));
            return sizeof(*cmd);
        }
        case GLOpcodeViewport:
        {
            auto cmd = reinterpret_cast<const GLCmdViewport*>(pc);
            {
                compiler.Call(::memcpy, JITStackPtr{ 0 }, &(cmd->viewport), sizeof(GLViewport));
                compiler.CallMember(&GLStateManager::SetViewport, g_stateMngrArg, JITStackPtr{ 0 });
                compiler.Call(::memcpy, JITStackPtr{ 0 }, &(cmd->depthRange), sizeof(GLDepthRange));
                compiler.CallMember(&GLStateManager::SetDepthRange, g_stateMngrArg, JITStackPtr{ 0 });
            }
            return sizeof(*cmd);
        }
        case GLOpcodeViewportArray:
        {
            auto cmd = reinterpret_cast<const GLCmdViewportArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                compiler.Call(::memcpy, JITStackPtr{ 0 }, cmdData, sizeof(GLViewport)*cmd->count);
                compiler.CallMember(&GLStateManager::SetViewportArray, g_stateMngrArg, cmd->first, cmd->count, JITStackPtr{ 0 });
                compiler.Call(::memcpy, JITStackPtr{ 0 }, cmdData + sizeof(GLViewport)*cmd->count, sizeof(GLDepthRange));
                compiler.CallMember(&GLStateManager::SetDepthRangeArray, g_stateMngrArg, cmd->first, cmd->count, JITStackPtr{ 0 });
            }
            return (sizeof(*cmd) + sizeof(GLViewport)*cmd->count + sizeof(GLDepthRange)*cmd->count);
        }
        case GLOpcodeScissor:
        {
            auto cmd = reinterpret_cast<const GLCmdScissor*>(pc);
            {
                compiler.Call(::memcpy, JITStackPtr{ 0 }, &(cmd->scissor), sizeof(GLScissor));
                compiler.CallMember(&GLStateManager::SetScissor, g_stateMngrArg, JITStackPtr{ 0 });
            }
            return sizeof(*cmd);
        }
        case GLOpcodeScissorArray:
        {
            auto cmd = reinterpret_cast<const GLCmdScissorArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                compiler.Call(::memcpy, JITStackPtr{ 0 }, cmdData, sizeof(GLScissor)*cmd->count);
                compiler.CallMember(&GLStateManager::SetScissorArray, g_stateMngrArg, cmd->first, cmd->count, JITStackPtr{ 0 });
            }
            return (sizeof(*cmd) + sizeof(GLScissor)*cmd->count);
        }
        case GLOpcodeClearColor:
        {
            auto cmd = reinterpret_cast<const GLCmdClearColor*>(pc);
            compiler.Call(glClearColor, cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3]);
            return sizeof(*cmd);
        }
        case GLOpcodeClearDepth:
        {
            auto cmd = reinterpret_cast<const GLCmdClearDepth*>(pc);
            compiler.Call(glClearDepth, cmd->depth);
            return sizeof(*cmd);
        }
        case GLOpcodeClearStencil:
        {
            auto cmd = reinterpret_cast<const GLCmdClearStencil*>(pc);
            compiler.Call(glClearStencil, cmd->stencil);
            return sizeof(*cmd);
        }
        case GLOpcodeClear:
        {
            auto cmd = reinterpret_cast<const GLCmdClear*>(pc);
            compiler.CallMember(&GLStateManager::Clear, g_stateMngrArg, cmd->flags);
            return sizeof(*cmd);
        }
        case GLOpcodeClearBuffers:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBuffers*>(pc);
            compiler.CallMember(&GLStateManager::ClearBuffers, g_stateMngrArg, cmd->numAttachments, (cmd + 1));
            return sizeof(*cmd);
        }
        case GLOpcodeBindVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindVertexArray*>(pc);
            compiler.CallMember(&GLStateManager::BindVertexArray, g_stateMngrArg, cmd->vao);
            return sizeof(*cmd);
        }
        case GLOpcodeBindGL2XVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGL2XVertexArray*>(pc);
            compiler.CallMember(&GL2XVertexArray::Bind, cmd->vertexArrayGL2X, g_stateMngrArg);
            return sizeof(*cmd);
        }
        case GLOpcodeBindElementArrayBufferToVAO:
        {
            auto cmd = reinterpret_cast<const GLCmdBindElementArrayBufferToVAO*>(pc);
            compiler.CallMember(&GLStateManager::BindElementArrayBufferToVAO, g_stateMngrArg, cmd->id, cmd->indexType16Bits);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBufferBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBufferBase*>(pc);
            compiler.CallMember(&GLStateManager::BindBufferBase, g_stateMngrArg, cmd->target, cmd->index, cmd->id);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBuffersBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBuffersBase*>(pc);
            compiler.CallMember(&GLStateManager::BindBuffersBase, g_stateMngrArg, cmd->target, cmd->first, cmd->count, (cmd + 1));
            return (sizeof(*cmd) + sizeof(GLuint)*cmd->count);
        }
        case GLOpcodeBeginTransformFeedback:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedback*>(pc);
            compiler.Call(glBeginTransformFeedback, cmd->primitiveMove);
            return sizeof(*cmd);
        }
        #ifdef GL_NV_transform_feedback
        case GLOpcodeBeginTransformFeedbackNV:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedbackNV*>(pc);
            compiler.Call(glBeginTransformFeedbackNV, cmd->primitiveMove);
            return sizeof(*cmd);
        }
        #endif // /GL_NV_transform_feedback
        case GLOpcodeEndTransformFeedback:
        {
            compiler.Call(glEndTransformFeedback);
            return 0;
        }
        #ifdef GL_NV_transform_feedback
        case GLOpcodeEndTransformFeedbackNV:
        {
            compiler.Call(glEndTransformFeedbackNV);
            return 0;
        }
        #endif // /GL_NV_transform_feedback
        case GLOpcodeBindResourceHeap:
        {
            auto cmd = reinterpret_cast<const GLCmdBindResourceHeap*>(pc);
            compiler.CallMember(&GLResourceHeap::Bind, cmd->resourceHeap, g_stateMngrArg, cmd->firstSet);
            return sizeof(*cmd);
        }
        case GLOpcodeBindRenderPass:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderPass*>(pc);
            compiler.CallMember(&GLStateManager::BindRenderPass, g_stateMngrArg, cmd->renderTarget, cmd->renderPass, cmd->numClearValues, (cmd + 1), &(cmd->defaultClearValue));
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case GLOpcodeBindPipelineState:
        {
            auto cmd = reinterpret_cast<const GLCmdBindPipelineState*>(pc);
            if (cmd->pipelineState->IsGraphicsPSO())
                compiler.CallMember(&GLGraphicsPSO::Bind, cmd->pipelineState, g_stateMngrArg);
            else
                compiler.CallMember(&GLPipelineState::Bind, cmd->pipelineState, g_stateMngrArg);
            return sizeof(*cmd);
        }
        case GLOpcodeSetBlendColor:
        {
            auto cmd = reinterpret_cast<const GLCmdSetBlendColor*>(pc);
            compiler.CallMember(&GLStateManager::SetBlendColor, g_stateMngrArg, cmd->color);
            return sizeof(*cmd);
        }
        case GLOpcodeSetStencilRef:
        {
            auto cmd = reinterpret_cast<const GLCmdSetStencilRef*>(pc);
            compiler.CallMember(&GLStateManager::SetStencilRef, g_stateMngrArg, cmd->ref, cmd->face);
            return sizeof(*cmd);
        }
        case GLOpcodeSetUniforms:
        {
            auto cmd = reinterpret_cast<const GLCmdSetUniforms*>(pc);
            compiler.Call(GLSetUniformsByLocation, cmd->program, cmd->location, cmd->count, (cmd + 1));
            return (sizeof(*cmd) + cmd->size);
        }
        case GLOpcodeBeginQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginQuery*>(pc);
            compiler.CallMember(&GLQueryHeap::Begin, cmd->queryHeap, cmd->query);
            return sizeof(*cmd);
        }
        case GLOpcodeEndQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdEndQuery*>(pc);
            compiler.CallMember(&GLQueryHeap::End, cmd->queryHeap, cmd->query);
            return sizeof(*cmd);
        }
        case GLOpcodeBeginConditionalRender:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginConditionalRender*>(pc);
            compiler.Call(glBeginConditionalRender, cmd->id, cmd->mode);
            return sizeof(*cmd);
        }
        case GLOpcodeEndConditionalRender:
        {
            compiler.Call(glEndConditionalRender);
            return 0;
        }
        case GLOpcodeDrawArrays:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArrays*>(pc);
            compiler.Call(glDrawArrays, cmd->mode, cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawArraysInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstanced*>(pc);
            compiler.Call(glDrawArraysInstanced, cmd->mode, cmd->first, cmd->count, cmd->instancecount);
            return sizeof(*cmd);
        }
        #ifdef GL_ARB_base_instance
        case GLOpcodeDrawArraysInstancedBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstancedBaseInstance*>(pc);
            compiler.Call(glDrawArraysInstancedBaseInstance, cmd->mode, cmd->first, cmd->count, cmd->instancecount, cmd->baseinstance);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_base_instance
        case GLOpcodeDrawArraysIndirect:
        {
            //TODO: generate loop in ASM
            auto cmd = reinterpret_cast<const GLCmdDrawArraysIndirect*>(pc);
            compiler.CallMember(&GLStateManager::BindBuffer, g_stateMngrArg, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            GLintptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                compiler.Call(glDrawArraysIndirect, cmd->mode, reinterpret_cast<const GLvoid*>(offset));
                offset += cmd->stride;
            }
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElements:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElements*>(pc);
            compiler.Call(glDrawElements, cmd->mode, cmd->count, cmd->type, cmd->indices);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsBaseVertex*>(pc);
            compiler.Call(glDrawElementsBaseVertex, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->basevertex);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstanced*>(pc);
            compiler.Call(glDrawElementsInstanced, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount);
            return sizeof(*cmd);
        }
        case GLOpcodeDrawElementsInstancedBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertex*>(pc);
            compiler.Call(glDrawElementsInstancedBaseVertex, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex);
            return sizeof(*cmd);
        }
        #ifdef GL_ARB_base_instance
        case GLOpcodeDrawElementsInstancedBaseVertexBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertexBaseInstance*>(pc);
            compiler.Call(glDrawElementsInstancedBaseVertexBaseInstance, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex, cmd->baseinstance);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_base_instance
        case GLOpcodeDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsIndirect*>(pc);
            {
                //TODO: generate loop in ASM
                compiler.CallMember(&GLStateManager::BindBuffer, g_stateMngrArg, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
                GLintptr offset = cmd->indirect;
                for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
                {
                    compiler.Call(glDrawElementsIndirect, cmd->mode, cmd->type, reinterpret_cast<const GLvoid*>(offset));
                    offset += cmd->stride;
                }
            }
            return sizeof(*cmd);
        }
        #ifdef GL_ARB_multi_draw_indirect
        case GLOpcodeMultiDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawArraysIndirect*>(pc);
            compiler.CallMember(&GLStateManager::BindBuffer, g_stateMngrArg, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glMultiDrawArraysIndirect, cmd->mode, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(*cmd);
        }
        case GLOpcodeMultiDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawElementsIndirect*>(pc);
            compiler.CallMember(&GLStateManager::BindBuffer, g_stateMngrArg, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glMultiDrawElementsIndirect, cmd->mode, cmd->type, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_multi_draw_indirect
        #ifdef GL_ARB_compute_shader
        case GLOpcodeDispatchCompute:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchCompute*>(pc);
            compiler.Call(glDispatchCompute, cmd->numgroups[0], cmd->numgroups[1], cmd->numgroups[2]);
            return sizeof(*cmd);
        }
        case GLOpcodeDispatchComputeIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchComputeIndirect*>(pc);
            compiler.CallMember(&GLStateManager::BindBuffer, g_stateMngrArg, GLBufferTarget::DISPATCH_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glDispatchComputeIndirect, cmd->indirect);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_compute_shader
        case GLOpcodeBindTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindTexture*>(pc);
            compiler.CallMember(&GLStateManager::ActiveTexture, g_stateMngrArg, cmd->slot);
            compiler.CallMember(&GLStateManager::BindGLTexture, g_stateMngrArg, cmd->texture);
            return sizeof(*cmd);
        }
        case GLOpcodeBindSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindSampler*>(pc);
            compiler.CallMember(&GLStateManager::BindSampler, g_stateMngrArg, cmd->slot, cmd->sampler);
            return sizeof(*cmd);
        }
        case GLOpcodeUnbindResources:
        {
            auto cmd = reinterpret_cast<const GLCmdUnbindResources*>(pc);
            if (cmd->resetUBO)
                compiler.CallMember(&GLStateManager::UnbindBuffersBase, g_stateMngrArg, GLBufferTarget::UNIFORM_BUFFER, cmd->first, cmd->count);
            if (cmd->resetSSAO)
                compiler.CallMember(&GLStateManager::UnbindBuffersBase, g_stateMngrArg, GLBufferTarget::SHADER_STORAGE_BUFFER, cmd->first, cmd->count);
            if (cmd->resetTransformFeedback)
                compiler.CallMember(&GLStateManager::UnbindBuffersBase, g_stateMngrArg, GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, cmd->first, cmd->count);
            if (cmd->resetTextures)
                compiler.CallMember(&GLStateManager::UnbindTextures, g_stateMngrArg, cmd->first, cmd->count);
            if (cmd->resetImages)
                compiler.CallMember(&GLStateManager::UnbindImageTextures, g_stateMngrArg, cmd->first, cmd->count);
            if (cmd->resetSamplers)
                compiler.CallMember(&GLStateManager::UnbindSamplers, g_stateMngrArg, cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        #ifdef GL_KHR_debug
        case GLOpcodePushDebugGroup:
        {
            auto cmd = reinterpret_cast<const GLCmdPushDebugGroup*>(pc);
            compiler.Call(glPushDebugGroup, cmd->source, cmd->id, cmd->length, reinterpret_cast<const GLchar*>(cmd + 1));
            return (sizeof(*cmd) + cmd->length + 1);
        }
        case GLOpcodePopDebugGroup:
        {
            compiler.Call(glPopDebugGroup);
            return 0;
        }
        #endif // /GL_KHR_debug
        default:
            return 0;
    }
}

// Determines the maximum requried stack size to execute the specified command buffer natively
static std::size_t RequiredLocalStackSize(const GLDeferredCommandBuffer& cmdBuffer)
{
    std::size_t maxSize = 0;

    maxSize = cmdBuffer.GetMaxNumViewports() * sizeof(GLViewport);
    maxSize = std::max(maxSize, cmdBuffer.GetMaxNumViewports() * sizeof(GLDepthRange));
    maxSize = std::max(maxSize, cmdBuffer.GetMaxNumScissors() * sizeof(GLScissor));

    return maxSize;
}

std::unique_ptr<JITProgram> AssembleGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdBuffer)
{
    /* Try to create a JIT-compiler for the active architecture (if supported) */
    if (auto compiler = JITCompiler::Create())
    {
        /* Initialize program counter to execute virtual GL commands */
        const auto& rawBuffer = cmdBuffer.GetRawBuffer();

        auto pc     = rawBuffer.data();
        auto pcEnd  = rawBuffer.data() + rawBuffer.size();

        GLOpcode opcode;

        /* Declare variadic arguments for entry point of JIT program */
        compiler->EntryPointVarArgs({ JIT::ArgType::Ptr });

        /* Declare stack allocation for temporary storage (viewports and scissors) */
        auto stackSize = static_cast<std::uint32_t>(RequiredLocalStackSize(cmdBuffer));
        if (stackSize > 0)
            compiler->StackAlloc(stackSize);

        /* Assemble GL commands into JIT program */
        compiler->Begin();

        while (pc < pcEnd)
        {
            /* Read opcode */
            opcode = *reinterpret_cast<const GLOpcode*>(pc);
            pc += sizeof(GLOpcode);

            /* Execute command and increment program counter */
            pc += AssembleGLCommand(opcode, pc, *compiler);
        }

        compiler->End();

        /* Build final program */
        return compiler->FlushProgram();
    }
    return nullptr;
}


} // /namespace LLGL


#endif // /LLGL_ENABLE_JIT_COMPILER



// ================================================================================
