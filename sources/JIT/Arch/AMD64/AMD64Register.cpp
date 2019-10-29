/*
 * AMD64Register.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AMD64Register.h"
#include "AMD64Opcode.h"


namespace LLGL
{

namespace JIT
{


std::uint8_t RegByte(const Reg reg)
{
    switch (reg)
    {
        case Reg::EAX:   return 0x00; // 000
        case Reg::ECX:   return 0x01; // 001
        case Reg::EDX:   return 0x02; // 010
        case Reg::EBX:   return 0x03; // 011
        case Reg::ESP:   return 0x04; // 100
        case Reg::EBP:   return 0x05; // 101
        case Reg::ESI:   return 0x06; // 110
        case Reg::EDI:   return 0x07; // 111

        case Reg::RAX:   return 0x00; // 000
        case Reg::RCX:   return 0x01; // 001
        case Reg::RDX:   return 0x02; // 010
        case Reg::RBX:   return 0x03; // 011
        case Reg::RSP:   return 0x04; // 100
        case Reg::RBP:   return 0x05; // 101
        case Reg::RSI:   return 0x06; // 110
        case Reg::RDI:   return 0x07; // 111

        case Reg::R8:    return 0x00; // 000
        case Reg::R9:    return 0x01; // 001
        case Reg::R10:   return 0x02; // 010
        case Reg::R11:   return 0x03; // 011
        case Reg::R12:   return 0x04; // 100
        case Reg::R13:   return 0x05; // 101
        case Reg::R14:   return 0x06; // 110
        case Reg::R15:   return 0x07; // 111

        case Reg::XMM0:  return 0x00; // 000
        case Reg::XMM1:  return 0x01; // 001
        case Reg::XMM2:  return 0x02; // 010
        case Reg::XMM3:  return 0x03; // 011
        case Reg::XMM4:  return 0x04; // 100
        case Reg::XMM5:  return 0x05; // 101
        case Reg::XMM6:  return 0x06; // 110
        case Reg::XMM7:  return 0x07; // 111

        case Reg::XMM8:  return 0x00; // 000
        case Reg::XMM9:  return 0x01; // 001
        case Reg::XMM10: return 0x02; // 010
        case Reg::XMM11: return 0x03; // 011
        case Reg::XMM12: return 0x04; // 100
        case Reg::XMM13: return 0x05; // 101
        case Reg::XMM14: return 0x06; // 110
        case Reg::XMM15: return 0x07; // 111
    }
    return 0xFF;
}

bool Is64Reg(const Reg reg)
{
    return (reg >= Reg::RAX && reg <= Reg::R15);
}

bool IsFltReg(const Reg reg)
{
    return (reg >= Reg::XMM0 && reg <= Reg::XMM15);
}


} // /namespace JIT

} // /namespace LLGL



// ================================================================================
