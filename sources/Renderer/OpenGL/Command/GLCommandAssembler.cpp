/*
 * GLCommandAssembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_JIT_COMPILER

#include "GLCommandAssembler.h"
#include "GLCommandExecutor.h"
#include "GLCommand.h"
#include "GLDeferredCommandBuffer.h"
#include "../../../JIT/JITCompiler.h"

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


namespace LLGL
{


static std::size_t AssembleGLCommand(const GLOpcode opcode, const void* pc, JITCompiler& compiler)
{
    /* Declare index of variadic argument of entry point */
    static const JITVarArg g_stateMngrArg{ 0 };
    
    /* Generate native CPU opcodes for emulated GLOpcode */
    switch (opcode)
    {
        case GLOpcodeUpdateBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdUpdateBuffer*>(pc);
            compiler.CallMember(&GLBuffer::BufferSubData, cmd->buffer, cmd->offset, cmd->size, (cmd + 1));
            return sizeof(*cmd) + cmd->size;
        }
        case GLOpcodeCopyBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyBuffer*>(pc);
            compiler.CallMember(&GLBuffer::CopyBufferSubData, cmd->writeBuffer, cmd->readBuffer, cmd->readOffset, cmd->writeOffset, cmd->size);
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
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpcodeViewport:
        {
            auto cmd = reinterpret_cast<const GLCmdViewport*>(pc);
            {
                GLViewport viewport = cmd->viewport;
                stateMngr.SetViewport(viewport);

                GLDepthRange depthRange = cmd->depthRange;
                stateMngr.SetDepthRange(depthRange);
            }
            return sizeof(*cmd);
        }
        case GLOpcodeViewportArray:
        {
            auto cmd = reinterpret_cast<const GLCmdViewportArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                union
                {
                    GLViewport viewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                    GLDepthRange depthRanges[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                };

                ::memcpy(viewports, cmdData, sizeof(GLViewport)*cmd->count);
                stateMngr.SetViewportArray(cmd->first, cmd->count, viewports);

                ::memcpy(depthRanges, cmdData + sizeof(GLViewport)*cmd->count, sizeof(GLDepthRange)*cmd->count);
                stateMngr.SetDepthRangeArray(cmd->first, cmd->count, depthRanges);
            }
            return (sizeof(*cmd) + sizeof(GLViewport)*cmd->count + sizeof(GLDepthRange)*cmd->count);
        }
        case GLOpcodeScissor:
        {
            auto cmd = reinterpret_cast<const GLCmdScissor*>(pc);
            {
                GLScissor scissor = cmd->scissor;
                stateMngr.SetScissor(scissor);
            }
            return sizeof(*cmd);
        }
        case GLOpcodeScissorArray:
        {
            auto cmd = reinterpret_cast<const GLCmdScissorArray*>(pc);
            auto cmdData = reinterpret_cast<const std::int8_t*>(cmd + 1);
            {
                GLScissor scissors[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
                ::memcpy(scissors, cmdData, sizeof(GLScissor)*cmd->count);
                stateMngr.SetScissorArray(cmd->first, cmd->count, scissors);
            }
            return (sizeof(*cmd) + sizeof(GLScissor)*cmd->count);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpcodeClearBuffers:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBuffers*>(pc);
            stateMngr.ClearBuffers(cmd->numAttachments, reinterpret_cast<const AttachmentClear*>(cmd + 1));
            return sizeof(*cmd);
        }
        case GLOpcodeBindVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindVertexArray*>(pc);
            stateMngr.BindVertexArray(cmd->vao);
            return sizeof(*cmd);
        }
        case GLOpcodeBindElementArrayBufferToVAO:
        {
            auto cmd = reinterpret_cast<const GLCmdBindElementArrayBufferToVAO*>(pc);
            stateMngr.BindElementArrayBufferToVAO(cmd->id);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBufferBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBufferBase*>(pc);
            stateMngr.BindBufferBase(cmd->target, cmd->index, cmd->id);
            return sizeof(*cmd);
        }
        case GLOpcodeBindBuffersBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBuffersBase*>(pc);
            stateMngr.BindBuffersBase(cmd->target, cmd->first, cmd->count, reinterpret_cast<const GLuint*>(cmd + 1));
            return (sizeof(*cmd) + sizeof(GLuint)*cmd->count);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
            compiler.CallMember(&GLResourceHeap::Bind, cmd->resourceHeap, g_stateMngrArg);
            return sizeof(*cmd);
        }
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpcodeBindRenderPass:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderPass*>(pc);
            stateMngr.BindRenderPass(*(cmd->renderTarget), cmd->renderPass, cmd->numClearValues, reinterpret_cast<const ClearValue*>(cmd + 1), cmd->defaultClearValue);
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpcodeBindGraphicsPipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGraphicsPipeline*>(pc);
            compiler.CallMember(&GLGraphicsPipeline::Bind, cmd->graphicsPipeline, g_stateMngrArg);
            return sizeof(*cmd);
        }
        case GLOpcodeBindComputePipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindComputePipeline*>(pc);
            compiler.CallMember(&GLComputePipeline::Bind, cmd->computePipeline, g_stateMngrArg);
            return sizeof(*cmd);
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
        #if 0 //TODO
        case GLOpcodeDrawArraysIndirect:
        {
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
        #endif // /TODO
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
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpcodeDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsIndirect*>(pc);
            {
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
            compiler.Call(Wrapper_GLStateManager_BindBuffer, stateMngr, GLBufferTarget::DISPATCH_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glDispatchComputeIndirect, cmd->indirect);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_compute_shader
        case GLOpcodeBindTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindTexture*>(pc);
            stateMngr.ActiveTexture(cmd->slot);
            stateMngr.BindTexture(*(cmd->texture));
            return sizeof(*cmd);
        }
        case GLOpcodeBindSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindSampler*>(pc);
            stateMngr.BindSampler(cmd->slot, cmd->sampler);
            return sizeof(*cmd);
        }
        case GLOpcodeUnbindResources:
        {
            auto cmd = reinterpret_cast<const GLCmdUnbindResources*>(pc);
            if (cmd->resetUBO)
                stateMngr.UnbindBuffersBase(GLBufferTarget::UNIFORM_BUFFER, cmd->first, cmd->count);
            if (cmd->resetSSAO)
                stateMngr.UnbindBuffersBase(GLBufferTarget::SHADER_STORAGE_BUFFER, cmd->first, cmd->count);
            if (cmd->resetTransformFeedback)
                stateMngr.UnbindBuffersBase(GLBufferTarget::TRANSFORM_FEEDBACK_BUFFER, cmd->first, cmd->count);
            if (cmd->resetTextures)
                stateMngr.UnbindTextures(cmd->first, cmd->count);
            #if 0//TODO
            if (cmd->resetImages)
                stateMngr.UnbindImages(cmd->first, cmd->count);
            #endif
            if (cmd->resetSamplers)
                stateMngr.UnbindSamplers(cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        default:
            return 0;
    }
}

std::unique_ptr<JITProgram> AssembleGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdBuffer)
{
    auto compiler = JITCompiler::Create();
    
    /* Initialize program counter to execute virtual GL commands */
    const auto& rawBuffer = cmdBuffer.GetRawBuffer();
    
    auto pc     = rawBuffer.data();
    auto pcEnd  = rawBuffer.data() + rawBuffer.size();

    GLOpcode opcode;
    
    compiler->EntryPointParams({ JIT::ArgType::Ptr });

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


} // /namespace LLGL


#endif // /LLGL_ENABLE_JIT_COMPILER



// ================================================================================
