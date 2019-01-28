/*
 * GLCommandOpcode.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_OPCODE_H
#define LLGL_GL_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum GLOpcode : std::uint8_t
{
    GLOpcodeUpdateBuffer = 1,
    GLOpcodeCopyBuffer,
    GLOpcodeSetAPIDepState,
    GLOpcodeExecute,
    GLOpcodeViewport,
    GLOpcodeViewportArray,
    GLOpcodeScissor,
    GLOpcodeScissorArray,
    GLOpcodeClearColor,
    GLOpcodeClearDepth,
    GLOpcodeClearStencil,
    GLOpcodeClear,
    GLOpcodeClearBuffers,
    GLOpcodeBindVertexArray,
    GLOpcodeBindGL2XVertexArray,
    GLOpcodeBindElementArrayBufferToVAO,
    GLOpcodeBindBufferBase,
    GLOpcodeBindBuffersBase,
    GLOpcodeBeginTransformFeedback,
    GLOpcodeBeginTransformFeedbackNV,
    GLOpcodeEndTransformFeedback,
    GLOpcodeEndTransformFeedbackNV,
    GLOpcodeBindResourceHeap,
    GLOpcodeBindRenderPass,
    GLOpcodeBindGraphicsPipeline,
    GLOpcodeBindComputePipeline,
    GLOpcodeBeginQuery,
    GLOpcodeEndQuery,
    GLOpcodeBeginConditionalRender,
    GLOpcodeEndConditionalRender,
    GLOpcodeDrawArrays,
    GLOpcodeDrawArraysInstanced,
    GLOpcodeDrawArraysInstancedBaseInstance,
    GLOpcodeDrawArraysIndirect,
    GLOpcodeDrawElements,
    GLOpcodeDrawElementsBaseVertex,
    GLOpcodeDrawElementsInstanced,
    GLOpcodeDrawElementsInstancedBaseVertex,
    GLOpcodeDrawElementsInstancedBaseVertexBaseInstance,
    GLOpcodeDrawElementsIndirect,
    GLOpcodeMultiDrawArraysIndirect,
    GLOpcodeMultiDrawElementsIndirect,
    GLOpcodeDispatchCompute,
    GLOpcodeDispatchComputeIndirect,
    GLOpcodeBindTexture,
    GLOpcodeBindSampler,
    GLOpcodeUnbindResources,
};


} // /namespace LLGL


#endif



// ================================================================================
