/*
 * MTCommandOpcode.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_OPCODE_H
#define LLGL_MT_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum MTOpcode : std::uint8_t
{
    MTOpcodeNop = 0,
    MTOpcodeExecute,
    MTOpcodeCopyBuffer,
    MTOpcodeCopyBufferFromTexture,
    MTOpcodeCopyTexture,
    MTOpcodeCopyTextureFromBuffer,
    MTOpcodeCopyTextureFromFramebuffer,
    MTOpcodeGenerateMipmaps,
    MTOpcodeSetGraphicsPSO,
    MTOpcodeSetComputePSO,
    MTOpcodeSetViewports,
    MTOpcodeSetScissorRects,
    MTOpcodeSetBlendColor,
    MTOpcodeSetStencilRef,
    MTOpcodeSetUniforms,
    MTOpcodeSetVertexBuffers,
    MTOpcodeSetIndexBuffer,
    MTOpcodeSetResourceHeap,
    MTOpcodeSetResource,
    MTOpcodeBeginRenderPass,
    MTOpcodeEndRenderPass,
    MTOpcodeClearRenderPass,
    MTOpcodeDraw,
    MTOpcodeDrawIndexed,
    MTOpcodeDispatchThreadgroups,
    MTOpcodeDispatchThreadgroupsIndirect,
    MTOpcodePushDebugGroup,
    MTOpcodePopDebugGroup,
    MTOpcodePresentDrawables,
    MTOpcodeFlush,
};


} // /namespace LLGL


#endif



// ================================================================================
