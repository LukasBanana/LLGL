/*
 * D3D11CommandOpcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMAND_OPCODE_H
#define LLGL_D3D11_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum D3D11Opcode : std::uint8_t
{
    D3D11OpcodeSetVertexBuffer = 1,
    D3D11OpcodeSetVertexBufferArray,
    D3D11OpcodeSetIndexBuffer,
    D3D11OpcodeSetPipelineState,
    D3D11OpcodeSetResourceHeap,
    D3D11OpcodeSetResource,
    D3D11OpcodeSetBlendFactor,
    D3D11OpcodeSetStencilRef,
    D3D11OpcodeSetUniforms,
    D3D11OpcodeDraw,
    D3D11OpcodeDrawIndexed,
    D3D11OpcodeDrawInstanced,
    D3D11OpcodeDrawIndexedInstanced,
    D3D11OpcodeDrawInstancedIndirect,
    D3D11OpcodeDrawInstancedIndirectN,
    D3D11OpcodeDrawIndexedInstancedIndirect,
    D3D11OpcodeDrawIndexedInstancedIndirectN,
    D3D11OpcodeDrawAuto,
    D3D11OpcodeDispatch,
    D3D11OpcodeDispatchIndirect,
};


} // /namespace LLGL


#endif



// ================================================================================
