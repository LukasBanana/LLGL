/*
 * GLCommandAssembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_JIT_COMPILER

#include "GLCommandAssembler.h"
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


static void Wrapper_GLBuffer_UpdateSubData(GLBuffer* self, GLintptr arg0, GLsizeiptr arg1, const void* arg2)
{
    self->BufferSubData(arg0, arg1, arg2);
}

/*static void Wrapper_GLStateManager_BindBuffer(GLStateManager* self, GLBufferTarget arg0, GLuint arg1)
{
    self->BindBuffer(arg0, arg1);
}*/

static void Wrapper_GLStateManager_Clear(GLStateManager* self, long arg0)
{
    self->Clear(arg0);
}

static std::size_t AssembleGLCommand(const GLOpCode opcode, const void* pc, JITCompiler& compiler)
{
    switch (opcode)
    {
        case GLOpCodeUpdateBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdUpdateBuffer*>(pc);
            {
                compiler.PushPtr(cmd->buffer);
                compiler.PushSSizeT(cmd->offset);
                compiler.PushSSizeT(cmd->size);
                compiler.PushPtr(cmd + 1);
                compiler.FuncCall(reinterpret_cast<const void*>(Wrapper_GLBuffer_UpdateSubData));
            }
            return sizeof(*cmd) + cmd->size;
        }
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeCopyBuffer:
        {
            auto cmd = reinterpret_cast<const GLCmdCopyBuffer*>(pc);
            cmd->writeBuffer->CopyBufferSubData(*(cmd->readBuffer), cmd->readOffset, cmd->writeOffset, cmd->size);
            return sizeof(*cmd);
        }
        case GLOpCodeExecute:
        {
            auto cmd = reinterpret_cast<const GLCmdExecute*>(pc);
            ExecuteGLDeferredCommandBuffer(*(cmd->commandBuffer), stateMngr);
            return sizeof(*cmd);
        }
        case GLOpCodeSetAPIDepState:
        {
            auto cmd = reinterpret_cast<const GLCmdSetAPIDepState*>(pc);
            stateMngr.SetGraphicsAPIDependentState(cmd->desc);
            return sizeof(*cmd);
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
            return sizeof(*cmd);
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

                ::memcpy(viewports, cmdData, sizeof(GLViewport)*cmd->count);
                stateMngr.SetViewportArray(cmd->first, cmd->count, viewports);

                ::memcpy(depthRanges, cmdData + sizeof(GLViewport)*cmd->count, sizeof(GLDepthRange)*cmd->count);
                stateMngr.SetDepthRangeArray(cmd->first, cmd->count, depthRanges);
            }
            return (sizeof(*cmd) + sizeof(GLViewport)*cmd->count + sizeof(GLDepthRange)*cmd->count);
        }
        case GLOpCodeScissor:
        {
            auto cmd = reinterpret_cast<const GLCmdScissor*>(pc);
            {
                GLScissor scissor = cmd->scissor;
                stateMngr.SetScissor(scissor);
            }
            return sizeof(*cmd);
        }
        case GLOpCodeScissorArray:
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
        case GLOpCodeClearColor:
        {
            auto cmd = reinterpret_cast<const GLCmdClearColor*>(pc);
            compiler.Call(glClearColor, cmd->color[0], cmd->color[1], cmd->color[2], cmd->color[3]);
            return sizeof(*cmd);
        }
        case GLOpCodeClearDepth:
        {
            auto cmd = reinterpret_cast<const GLCmdClearDepth*>(pc);
            compiler.Call(glClearDepth, cmd->depth);
            return sizeof(*cmd);
        }
        case GLOpCodeClearStencil:
        {
            auto cmd = reinterpret_cast<const GLCmdClearStencil*>(pc);
            compiler.Call(glClearStencil, cmd->stencil);
            return sizeof(*cmd);
        }
        case GLOpCodeClear:
        {
            auto cmd = reinterpret_cast<const GLCmdClear*>(pc);
            compiler.Call(Wrapper_GLStateManager_Clear, JITVarArg{ 0 }, cmd->flags);
            return sizeof(*cmd);
        }
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeClearBuffers:
        {
            auto cmd = reinterpret_cast<const GLCmdClearBuffers*>(pc);
            stateMngr.ClearBuffers(cmd->numAttachments, reinterpret_cast<const AttachmentClear*>(cmd + 1));
            return sizeof(*cmd);
        }
        case GLOpCodeBindVertexArray:
        {
            auto cmd = reinterpret_cast<const GLCmdBindVertexArray*>(pc);
            stateMngr.BindVertexArray(cmd->vao);
            return sizeof(*cmd);
        }
        case GLOpCodeBindElementArrayBufferToVAO:
        {
            auto cmd = reinterpret_cast<const GLCmdBindElementArrayBufferToVAO*>(pc);
            stateMngr.BindElementArrayBufferToVAO(cmd->id);
            return sizeof(*cmd);
        }
        case GLOpCodeBindBufferBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBufferBase*>(pc);
            stateMngr.BindBufferBase(cmd->target, cmd->index, cmd->id);
            return sizeof(*cmd);
        }
        case GLOpCodeBindBuffersBase:
        {
            auto cmd = reinterpret_cast<const GLCmdBindBuffersBase*>(pc);
            stateMngr.BindBuffersBase(cmd->target, cmd->first, cmd->count, reinterpret_cast<const GLuint*>(cmd + 1));
            return (sizeof(*cmd) + sizeof(GLuint)*cmd->count);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeBeginTransformFeedback:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedback*>(pc);
            compiler.Call(glBeginTransformFeedback, cmd->primitiveMove);
            return sizeof(*cmd);
        }
        #ifdef GL_NV_transform_feedback
        case GLOpCodeBeginTransformFeedbackNV:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginTransformFeedbackNV*>(pc);
            compiler.Call(glBeginTransformFeedbackNV, cmd->primitiveMove);
            return sizeof(*cmd);
        }
        #endif // /GL_NV_transform_feedback
        case GLOpCodeEndTransformFeedback:
        {
            compiler.Call(glEndTransformFeedback);
            return 0;
        }
        #ifdef GL_NV_transform_feedback
        case GLOpCodeEndTransformFeedbackNV:
        {
            compiler.Call(glEndTransformFeedbackNV);
            return 0;
        }
        #endif // /GL_NV_transform_feedback
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeBindResourceHeap:
        {
            auto cmd = reinterpret_cast<const GLCmdBindResourceHeap*>(pc);
            cmd->resourceHeap->Bind(stateMngr);
            return sizeof(*cmd);
        }
        case GLOpCodeBindRenderPass:
        {
            auto cmd = reinterpret_cast<const GLCmdBindRenderPass*>(pc);
            stateMngr.BindRenderPass(*(cmd->renderTarget), cmd->renderPass, cmd->numClearValues, reinterpret_cast<const ClearValue*>(cmd + 1), cmd->defaultClearValue);
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case GLOpCodeBindGraphicsPipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindGraphicsPipeline*>(pc);
            cmd->graphicsPipeline->Bind(stateMngr);
            return sizeof(*cmd);
        }
        case GLOpCodeBindComputePipeline:
        {
            auto cmd = reinterpret_cast<const GLCmdBindComputePipeline*>(pc);
            cmd->computePipeline->Bind(stateMngr);
            return sizeof(*cmd);
        }
        case GLOpCodeBeginQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginQuery*>(pc);
            cmd->queryHeap->Begin(cmd->query);
            return sizeof(*cmd);
        }
        case GLOpCodeEndQuery:
        {
            auto cmd = reinterpret_cast<const GLCmdEndQuery*>(pc);
            cmd->queryHeap->End(cmd->query);
            return sizeof(*cmd);
        }
        #endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ /TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeBeginConditionalRender:
        {
            auto cmd = reinterpret_cast<const GLCmdBeginConditionalRender*>(pc);
            compiler.Call(glBeginConditionalRender, cmd->id, cmd->mode);
            return sizeof(*cmd);
        }
        case GLOpCodeEndConditionalRender:
        {
            compiler.Call(glEndConditionalRender);
            return 0;
        }
        case GLOpCodeDrawArrays:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArrays*>(pc);
            compiler.Call(glDrawArrays, cmd->mode, cmd->first, cmd->count);
            return sizeof(*cmd);
        }
        case GLOpCodeDrawArraysInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstanced*>(pc);
            compiler.Call(glDrawArraysInstanced, cmd->mode, cmd->first, cmd->count, cmd->instancecount);
            return sizeof(*cmd);
        }
        #ifdef GL_ARB_base_instance
        case GLOpCodeDrawArraysInstancedBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysInstancedBaseInstance*>(pc);
            compiler.Call(glDrawArraysInstancedBaseInstance, cmd->mode, cmd->first, cmd->count, cmd->instancecount, cmd->baseinstance);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_base_instance
        #if 0 //TODO
        case GLOpCodeDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawArraysIndirect*>(pc);
            compiler.Call(Wrapper_GLBuffer_UpdateSubData, stateMngr, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            GLintptr offset = cmd->indirect;
            for (std::uint32_t i = 0; i < cmd->numCommands; ++i)
            {
                compiler.Call(glDrawArraysIndirect, cmd->mode, reinterpret_cast<const GLvoid*>(offset));
                offset += cmd->stride;
            }
            return sizeof(*cmd);
        }
        #endif // /TODO
        case GLOpCodeDrawElements:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElements*>(pc);
            compiler.Call(glDrawElements, cmd->mode, cmd->count, cmd->type, cmd->indices);
            return sizeof(*cmd);
        }
        case GLOpCodeDrawElementsBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsBaseVertex*>(pc);
            compiler.Call(glDrawElementsBaseVertex, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->basevertex);
            return sizeof(*cmd);
        }
        case GLOpCodeDrawElementsInstanced:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstanced*>(pc);
            compiler.Call(glDrawElementsInstanced, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount);
            return sizeof(*cmd);
        }
        case GLOpCodeDrawElementsInstancedBaseVertex:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertex*>(pc);
            compiler.Call(glDrawElementsInstancedBaseVertex, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex);
            return sizeof(*cmd);
        }
        #ifdef GL_ARB_base_instance
        case GLOpCodeDrawElementsInstancedBaseVertexBaseInstance:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsInstancedBaseVertexBaseInstance*>(pc);
            compiler.Call(glDrawElementsInstancedBaseVertexBaseInstance, cmd->mode, cmd->count, cmd->type, cmd->indices, cmd->instancecount, cmd->basevertex, cmd->baseinstance);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_base_instance
        #if 0 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TODO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case GLOpCodeDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDrawElementsIndirect*>(pc);
            {
                compiler.Call(Wrapper_GLStateManager_BindBuffer, JITVarArg{ 0 }, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
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
        case GLOpCodeMultiDrawArraysIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawArraysIndirect*>(pc);
            compiler.Call(Wrapper_GLStateManager_BindBuffer, stateMngr, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glMultiDrawArraysIndirect, cmd->mode, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(*cmd);
        }
        case GLOpCodeMultiDrawElementsIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdMultiDrawElementsIndirect*>(pc);
            compiler.Call(Wrapper_GLStateManager_BindBuffer, stateMngr, GLBufferTarget::DRAW_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glMultiDrawElementsIndirect, cmd->mode, cmd->type, cmd->indirect, cmd->drawcount, cmd->stride);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_multi_draw_indirect
        #ifdef GL_ARB_compute_shader
        case GLOpCodeDispatchCompute:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchCompute*>(pc);
            compiler.Call(glDispatchCompute, cmd->numgroups[0], cmd->numgroups[1], cmd->numgroups[2]);
            return sizeof(*cmd);
        }
        case GLOpCodeDispatchComputeIndirect:
        {
            auto cmd = reinterpret_cast<const GLCmdDispatchComputeIndirect*>(pc);
            compiler.Call(Wrapper_GLStateManager_BindBuffer, stateMngr, GLBufferTarget::DISPATCH_INDIRECT_BUFFER, cmd->id);
            compiler.Call(glDispatchComputeIndirect, cmd->indirect);
            return sizeof(*cmd);
        }
        #endif // /GL_ARB_compute_shader
        case GLOpCodeBindTexture:
        {
            auto cmd = reinterpret_cast<const GLCmdBindTexture*>(pc);
            stateMngr.ActiveTexture(cmd->slot);
            stateMngr.BindTexture(*(cmd->texture));
            return sizeof(*cmd);
        }
        case GLOpCodeBindSampler:
        {
            auto cmd = reinterpret_cast<const GLCmdBindSampler*>(pc);
            stateMngr.BindSampler(cmd->slot, cmd->sampler);
            return sizeof(*cmd);
        }
        case GLOpCodeUnbindResources:
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

    GLOpCode opcode;
    
    compiler->EntryPointParams({ JIT::ArgType::Ptr });

    compiler->Begin();
    
    while (pc < pcEnd)
    {
        /* Read opcode */
        opcode = *reinterpret_cast<const GLOpCode*>(pc);
        pc += sizeof(GLOpCode);

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
