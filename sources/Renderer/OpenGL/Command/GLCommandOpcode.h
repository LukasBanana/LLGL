/*
 * GLCommandOpcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMMAND_OPCODE_H
#define LLGL_GL_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum GLOpcode : std::uint8_t
{
    GLOpcodeBufferSubData = 1,
    GLOpcodeCopyBufferSubData,
    GLOpcodeClearBufferData,
    GLOpcodeClearBufferSubData,
    GLOpcodeCopyImageSubData,
    GLOpcodeCopyImageToBuffer,
    GLOpcodeCopyImageFromBuffer,
    GLOpcodeCopyFramebufferSubData,
    GLOpcodeGenerateMipmap,
    GLOpcodeGenerateMipmapSubresource,
    GLOpcodeExecute,
    GLOpcodeViewport,
    GLOpcodeViewportArray,
    GLOpcodeScissor,
    GLOpcodeScissorArray,
    GLOpcodeClearColor,
    GLOpcodeClearDepth,
    GLOpcodeClearStencil,
    GLOpcodeClear,
    GLOpcodeClearAttachmentsWithRenderPass,
    GLOpcodeClearBuffers,
    GLOpcodeResolveRenderTarget,
    GLOpcodeBindVertexArray,
    GLOpcodeBindElementArrayBufferToVAO,
    GLOpcodeBindBufferBase,
    GLOpcodeBindBuffersBase,
    GLOpcodeBeginBufferXfb,
    GLOpcodeEndBufferXfb,
    GLOpcodeBeginTransformFeedback,
    GLOpcodeBeginTransformFeedbackNV,
    GLOpcodeEndTransformFeedback,
    GLOpcodeEndTransformFeedbackNV,
    GLOpcodeBindResourceHeap,
    GLOpcodeBindRenderTarget,
    GLOpcodeBindPipelineState,
    GLOpcodeSetBlendColor,
    GLOpcodeSetStencilRef,
    GLOpcodeSetUniform,
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
    GLOpcodeDrawTransformFeedback,
    GLOpcodeDrawEmulatedTransformFeedback,
    GLOpcodeMultiDrawArraysIndirect,
    GLOpcodeMultiDrawElementsIndirect,
    GLOpcodeDispatchCompute,
    GLOpcodeDispatchComputeIndirect,
    GLOpcodeBindTexture,
    GLOpcodeBindImageTexture,
    GLOpcodeBindSampler,
    GLOpcodeBindEmulatedSampler,
    GLOpcodeMemoryBarrier,
    GLOpcodePushDebugGroup,
    GLOpcodePopDebugGroup,
};


} // /namespace LLGL


#endif



// ================================================================================
