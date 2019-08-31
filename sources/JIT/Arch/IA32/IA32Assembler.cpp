/*
 * IA32Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IA32Assembler.h"
#include "IA32Opcode.h"


namespace LLGL
{

namespace JIT
{


void IA32Assembler::Begin()
{
    //TODO
}

void IA32Assembler::End()
{
    //TODO
}

void IA32Assembler::WriteFuncCall(const void* addr, JITCallConv conv, bool farCall)
{
    #if 0
    /* Write arguments */
    for ()
    {

    }

    /* Write 'this' pointer */
    if (conv == JITCallConv::ThisCall)
    {
        if (auto ptr = GetThisPtr())
            MovRegImm32(Reg::ECX, reinterpret_cast<std::uint32_t>(ptr));
        else
            throw std::runtime_error("missing 'this' pointer for '__thiscall' IA-32/x86 instruction");
    }

    /* Write 'call' instruction */
    if (farCall)
        CallFar(Reg::EAX);
    else
        CallNear(Reg::EAX);
    #endif
}


/*
 * ======= Private: =======
 */

bool IA32Assembler::IsLittleEndian() const
{
    return true;
}

void IA32Assembler::PushReg(const Reg reg)
{
    WriteByte(Opcode_PushReg | RegByte(reg));
}

void IA32Assembler::PushImm32(std::uint32_t dword)
{
    WriteByte(Opcode_PushImm32);
    WriteDWord(dword);
}

void IA32Assembler::PopReg(const Reg reg)
{
    WriteByte(Opcode_PopReg | RegByte(reg));
}

void IA32Assembler::MovRegImm32(const Reg reg, std::uint32_t dword)
{
    WriteByte(0x48); //???
    WriteByte(Opcode_MovRegImm32 | RegByte(reg));
    WriteDWord(dword);
}

void IA32Assembler::CallNear(const Reg reg)
{
    WriteByte(0xFF);
    WriteByte(Opcode_CallNear | RegByte(reg));
}

void IA32Assembler::CallFar(const Reg reg)
{
    //TODO
}

void IA32Assembler::RetNear(std::uint16_t word)
{
    if (word > 0)
    {
        WriteByte(Opcode_RetNearImm16);
        WriteWord(word);
    }
    else
        WriteByte(Opcode_RetNear);
}

void IA32Assembler::RetFar(std::uint16_t word)
{
    if (word > 0)
    {
        WriteByte(Opcode_RetFarImm16);
        WriteWord(word);
    }
    else
        WriteByte(Opcode_RetFar);
}


} // /namespace JIT

} // /namespace LLGL



// ================================================================================
