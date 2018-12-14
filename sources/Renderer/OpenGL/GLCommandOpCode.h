/*
 * GLCommandOpCode.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_OPCODE_H
#define LLGL_GL_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum GLOpCode : std::uint8_t
{
    GLOpCodeUpdateBuffer = 1,
    GLOpCodeCopyBuffer,
    GLOpCodeSetAPIDepState,
    GLOpCodeViewport,
    GLOpCodeViewportArray,
    GLOpCodeScissor,
    GLOpCodeScissorArray,
    GLOpCodeClearColor,
    GLOpCodeClearDepth,
    GLOpCodeClearStencil,
    GLOpCodeClear,
    GLOpCodeClearBuffers,
    GLOpCodeBindVertexArray,
    GLOpCodeBindElementArrayBufferToVAO,
    GLOpCodeBindBufferBase,
    GLOpCodeBindBuffersBase,
    GLOpCodeBeginTransformFeedback,
    GLOpCodeBeginTransformFeedbackNV,
    GLOpCodeEndTransformFeedback,
    GLOpCodeEndTransformFeedbackNV,
    GLOpCodeBindResourceHeap,
    GLOpCodeBindRenderPass,
    GLOpCodeBindGraphicsPipeline,
    GLOpCodeBindComputePipeline,
    GLOpCodeBeginQuery,
    GLOpCodeEndQuery,
    GLOpCodeBeginConditionalRender,
    GLOpCodeEndConditionalRender,
    GLOpCodeDrawArrays,
    GLOpCodeDrawArraysInstanced,
    GLOpCodeDrawArraysInstancedBaseInstance,
    GLOpCodeDrawArraysIndirect,
    GLOpCodeDrawElements,
    GLOpCodeDrawElementsBaseVertex,
    GLOpCodeDrawElementsInstanced,
    GLOpCodeDrawElementsInstancedBaseVertex,
    GLOpCodeDrawElementsInstancedBaseVertexBaseInstance,
    GLOpCodeDrawElementsIndirect,
    GLOpCodeMultiDrawArraysIndirect,
    GLOpCodeMultiDrawElementsIndirect,
    GLOpCodeDispatchCompute,
    GLOpCodeDispatchComputeIndirect,
    GLOpCodeBindTexture,
    GLOpCodeBindSampler,
    GLOpCodeUnbindResources,
};


} // /namespace LLGL


#endif



// ================================================================================
