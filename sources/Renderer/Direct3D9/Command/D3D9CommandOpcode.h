/*
 * D3D9CommandOpcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_COMMAND_OPCODE_H
#define LLGL_D3D9_COMMAND_OPCODE_H


#include <cstdint>


namespace LLGL
{


enum D3D9Opcode : std::uint8_t
{
    D3D9OpcodeBeginScene = 1,
    D3D9OpcodeEndScene,
    D3D9OpcodeSetRenderTargets,
    D3D9OpcodeSetViewport,
    D3D9OpcodeSetScissorRect,
    D3D9OpcodeClear,
    D3D9OpcodeSetIndices,
    D3D9OpcodeSetStreamSource,
    D3D9OpcodeBindProgrammablePSO,
    D3D9OpcodeSetRenderStates,
    //TODO
    D3D9OpcodeDraw,
    D3D9OpcodeDrawIndexed,
};


} // /namespace LLGL


#endif



// ================================================================================
