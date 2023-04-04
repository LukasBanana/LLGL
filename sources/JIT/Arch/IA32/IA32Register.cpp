/*
 * IA32Register.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
