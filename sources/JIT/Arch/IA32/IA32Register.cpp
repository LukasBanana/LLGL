/*
 * IA32Register.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IA32Register.h"
#include "IA32Opcode.h"


namespace LLGL
{

namespace JIT
{


std::uint8_t RegByte(const Reg reg)
{
    switch (reg)
    {
        case Reg::EAX: return 0x00; // 000
        case Reg::ECX: return 0x01; // 001
        case Reg::EDX: return 0x02; // 010
        case Reg::EBX: return 0x03; // 011
        case Reg::ESP: return 0x04; // 100
        case Reg::EBP: return 0x05; // 101
        case Reg::ESI: return 0x06; // 110
        case Reg::EDI: return 0x07; // 111
    }
    return 0xFF;
}


} // /namespace JIT

} // /namespace LLGL



// ================================================================================
