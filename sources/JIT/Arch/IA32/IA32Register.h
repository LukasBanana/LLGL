/*
 * IA32Register.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IA32_REGISTER_H
#define LLGL_IA32_REGISTER_H


#include <cstdint>


namespace LLGL
{

namespace JIT
{


// IA-32 register enumeration.
enum class Reg
{
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,
};

// Returns the specified register part of an IA-32 opcode.
std::uint8_t RegByte(const Reg reg);


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
