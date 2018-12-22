/*
 * AMD64Register.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_AMD64_REGISTER_H
#define LLGL_AMD64_REGISTER_H


#include <cstdint>


namespace LLGL
{

namespace JIT
{


// AMD64 register enumeration.
enum class Reg
{
    RAX,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
};

// Returns the specified register part of an AMD64 opcode.
std::uint8_t RegByte(const Reg reg);


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
