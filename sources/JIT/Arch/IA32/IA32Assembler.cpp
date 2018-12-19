/*
 * IA32Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IA32Assembler.h"
#include "IA32Opcode.h"


namespace LLGL
{

namespace JIT
{

namespace IA32
{


void IA32Assembler::BeginArgList(std::size_t numArgs)
{
    //TODO
}

void IA32Assembler::PushThisPtr(const void* value)
{
    //TODO
}

void IA32Assembler::PushPtr(const void* value)
{
    //TODO
}

void IA32Assembler::PushWord(std::uint16_t value)
{
    //TODO
}

void IA32Assembler::PushDWord(std::uint32_t value)
{
    //TODO
}

void IA32Assembler::PushQWord(std::uint64_t value)
{
    //TODO
}

void IA32Assembler::EndArgList()
{
    //TODO
}

void IA32Assembler::FuncCall(const void* addr, const JITCall call)
{
    //TODO
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


} // /namespace IA32

} // /namespace JIT

} // /namespace LLGL



// ================================================================================
