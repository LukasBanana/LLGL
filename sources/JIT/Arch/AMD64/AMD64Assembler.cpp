/*
 * AMD64Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AMD64Assembler.h"
#include "AMD64Opcode.h"


namespace LLGL
{

namespace JIT
{


// List of registers that are used for the first couple of arguments
static const Reg            g_amd64ArgRegs[]    = { Reg::RCX, Reg::RDX, Reg::R8, Reg::R9 };
static const std::size_t    g_amd64ArgRegsCount = sizeof(g_amd64ArgRegs)/sizeof(g_amd64ArgRegs[0]);

void AMD64Assembler::Begin()
{
    PushReg(Reg::RBP);
    MovReg(Reg::RBP, Reg::RSP);
}

void AMD64Assembler::End()
{
    PopReg(Reg::RBP);
    RetNear();
}

void AMD64Assembler::WriteFuncCall(const void* addr, const JITCallConv conv, bool farCall)
{
    /* Write arguments */
    const auto& args = GetArgs();
    
    for (std::size_t i = 0, n = args.size(); i < n; ++i)
    {
        /* Determine if argument is passed via register or stack */
        bool passViaReg = (i < g_amd64ArgRegsCount);
        const auto& arg = args[passViaReg ? i : (n - i - g_amd64ArgRegsCount - 1u)];
        
        /* Determine destination register for argument */
        Reg dstReg = Reg::R10;
        if (passViaReg)
            dstReg = g_amd64ArgRegs[i];
        
        /* Move value into destination register */
        switch (arg.type)
        {
            case ArgType::Byte:
                MovRegImm32(dstReg, arg.value.i8);
                break;
            case ArgType::Word:
                MovRegImm32(dstReg, arg.value.i16);
                break;
            case ArgType::DWord:
                MovRegImm32(dstReg, arg.value.i32);
                break;
            case ArgType::QWord:
                MovRegImm64(dstReg, arg.value.i64);
                break;
            case ArgType::Ptr:
                //TODO...
                break;
        }
        
        if (!passViaReg)
            PushReg(dstReg);
    }
    
    #if 0
    /* Write 'this' pointer */
    if (conv == JITCallConv::ThisCall)
    {
        if (auto ptr = GetThisPtr())
            MovRegImm64(Reg::RCX, reinterpret_cast<std::uint64_t>(ptr));
        else
            throw std::runtime_error("missing 'this' pointer for '__thiscall' AMD64/x86_64 instruction");
    }
    #endif
    
    /* Write 'call' instruction */
    MovRegImm32(Reg::RAX, 0);
    MovRegImm64(Reg::RBX, reinterpret_cast<std::uint64_t>(addr));
    
    if (farCall)
        CallFar(Reg::RBX);
    else
        CallNear(Reg::RBX);
}


/*
 * ======= Private: =======
 */

bool AMD64Assembler::IsLittleEndian() const
{
    return true;
}

void AMD64Assembler::PushReg(const Reg reg)
{
    WriteByte(Opcode_PushReg | RegByte(reg));
}

void AMD64Assembler::PopReg(const Reg reg)
{
    WriteByte(Opcode_PopReg | RegByte(reg));
}

void AMD64Assembler::MovReg(const Reg dst, const Reg src)
{
    //TODO
}

void AMD64Assembler::MovRegImm32(const Reg reg, std::uint32_t dword)
{
    WriteByte(Opcode_MovRegImm | RegByte(reg));
    WriteDWord(dword);
}

void AMD64Assembler::MovRegImm64(const Reg reg, std::uint64_t qword)
{
    if (reg >= Reg::R8)
        WriteByte(REX_Prefix | REX_W | REX_R);
    else
        WriteByte(REX_Prefix | REX_W);
    WriteByte(Opcode_MovRegImm | RegByte(reg));
    WriteQWord(qword);
}

void AMD64Assembler::CallNear(const Reg reg)
{
    if (reg >= Reg::R8)
        WriteByte(REX_Prefix | REX_W | REX_R);
    else
        WriteByte(REX_Prefix | REX_W);
    WriteByte(0xFF);
    WriteByte(Opcode_CallNear | RegByte(reg));
}

void AMD64Assembler::CallFar(const Reg reg)
{
    //TODO
}

void AMD64Assembler::RetNear(std::uint16_t word)
{
    if (word > 0)
    {
        WriteByte(Opcode_RetNearImm16);
        WriteWord(word);
    }
    else
        WriteByte(Opcode_RetNear);
}

void AMD64Assembler::RetFar(std::uint16_t word)
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
