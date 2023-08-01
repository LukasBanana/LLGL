/*
 * IA32Opcode.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IA32_OPCODE_H
#define LLGL_IA32_OPCODE_H


#include <LLGL/Export.h>
#include <vector>
#include <cstdint>


namespace LLGL
{

namespace JIT
{


enum Opcode : std::uint8_t
{
    Opcode_PushReg      = 0x50,
    Opcode_PopReg       = 0x58,
    Opcode_PushImm32    = 0x68,
    Opcode_MovRegImm32  = 0xB8, // B8+ rd id
    Opcode_RetNear      = 0xC3, // C3
    Opcode_RetFar       = 0xCB, // CB
    Opcode_RetNearImm16 = 0xC2, // C2 iw
    Opcode_RetFarImm16  = 0xCA, // CA iw
    Opcode_CallNear     = 0xD0,
    //OpCode_CallFar      = 0xE0,
};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
