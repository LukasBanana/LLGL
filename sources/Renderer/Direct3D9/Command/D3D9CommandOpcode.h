/*
 * D3D9CommandOpcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_COMMAND_OPCODE_H
#define LLGL_D3D9_COMMAND_OPCODE_H


#include "../../VirtualCommandBuffer.h"
#include <cstdint>


namespace LLGL
{


enum D3D9Opcode : std::uint8_t
{
    D3D9OpcodeBeginScene = 1,
    D3D9OpcodeEndScene,
    D3D9OpcodeExecute,
    D3D9OpcodeSetRenderTargets,
    D3D9OpcodeSetViewport,
    D3D9OpcodeSetScissorRect,
    D3D9OpcodeClear,
    D3D9OpcodeSetIndices,
    D3D9OpcodeSetStreamSource,
    D3D9OpcodeSetPipelineState,
    D3D9OpcodeSetRenderStates,
    D3D9OpcodeBindTexture,
    D3D9OpcodeBindSampler,
    D3D9OpcodeGenerateMips,
    D3D9OpcodeBufferWrite,
    //TODO
    D3D9OpcodeSetVertexShaderConstantF,
    D3D9OpcodeSetVertexShaderConstantI,
    D3D9OpcodeSetVertexShaderConstantB,
    D3D9OpcodeSetPixelShaderConstantF,
    D3D9OpcodeSetPixelShaderConstantI,
    D3D9OpcodeSetPixelShaderConstantB,
    D3D9OpcodeDraw,
    D3D9OpcodeDrawIndexed,
    D3D9OpcodeSetStreamSourceFreqIndexData,
    D3D9OpcodeSetStreamSourceFreqInstanceData,
};

using D3D9VirtualCommandBuffer = VirtualCommandBuffer<D3D9Opcode>;


} // /namespace LLGL


#endif



// ================================================================================
