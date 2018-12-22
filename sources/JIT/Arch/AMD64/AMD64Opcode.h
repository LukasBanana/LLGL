/*
 * AMD64Opcode.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_AMD64_OPCODE_H
#define LLGL_AMD64_OPCODE_H


#include <LLGL/Export.h>
#include <vector>
#include <cstdint>


namespace LLGL
{

namespace JIT
{

/*
------------------------------------------------------------------------
| Field: |   REX    | Opcode | ModR/M | SIB | Displacement | Immediate |
|--------|----------|--------|--------|-----|--------------|-----------|
| Size:  |   0-1    |  1-3   |  0-1   | 0-1 |   0,1,2,4    |  0,1,2,4  |
|--------|----------|--------|--------|-----|--------------|-----------|
| Bits:  | 0100WR0B |        |        |     |              |           |
------------------------------------------------------------------------
*/

enum REXBits : std::uint8_t
{
    REX_Prefix  = 0x40,
    REX_W       = 0x08,
    REX_R       = 0x04,
    REX_B       = 0x01,
};

enum Opcode : std::uint8_t
{
    Opcode_PushReg      = 0x50,
    Opcode_PopReg       = 0x58,
    Opcode_MovRegImm    = 0xB8, // [REX.W+] B8+ rd io
    Opcode_RetNear      = 0xC3, // C3
    Opcode_RetFar       = 0xCB, // CB
    Opcode_RetNearImm16 = 0xC2, // C2 iw
    Opcode_RetFarImm16  = 0xCA, // CA iw
    Opcode_CallNear     = 0xD0,
    //OpCode_CallFar      = 0x9A,
};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
