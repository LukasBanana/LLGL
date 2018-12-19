/*
 * IA32Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IA32Assembler.h"


namespace LLGL
{


/*
 * Internal unions and enums
 */

union Word
{
    std::uint16_t i16;
    std::uint8_t  i8[2];
};

union DWord
{
    std::uint32_t i32;
    std::uint8_t  i8[4];
};

enum IA32OpCode : std::uint8_t
{
    OpCode_PushReg      = 0x50,
    OpCode_PopReg       = 0x58,
    OpCode_PushImm32    = 0x68,
    OpCode_MovRegImm32  = 0xB8, // B8+ rd id
    OpCode_RetNear      = 0xC3, // C3
    OpCode_RetFar       = 0xCB, // CB
    OpCode_RetNearImm16 = 0xC2, // C2 iw
    OpCode_RetFarImm16  = 0xCA, // CA iw
    OpCode_CallNear     = 0xD0,
    //OpCode_CallFar      = 0xE0,
};


/*
 * Internal functions
 */

static std::uint8_t RegByte(IA32Reg reg)
{
    switch (reg)
    {
        case IA32Reg::EAX: return 0; // 000
        case IA32Reg::ECX: return 1; // 001
        case IA32Reg::EDX: return 2; // 010
        case IA32Reg::EBX: return 3; // 011
        case IA32Reg::ESP: return 4; // 100
        case IA32Reg::EBP: return 5; // 101
        case IA32Reg::ESI: return 6; // 110
        case IA32Reg::EDI: return 7; // 111
    }
    return 0;
}

void IA32Assembler::PushReg(IA32Reg reg)
{
    WriteByte(OpCode_PushReg | RegByte(reg));
}

void IA32Assembler::PushImm32(std::uint32_t dword)
{
    WriteByte(OpCode_PushImm32);
    WriteDWord(dword);
}

void IA32Assembler::PopReg(IA32Reg reg)
{
    WriteByte(OpCode_PopReg | RegByte(reg));
}

void IA32Assembler::MovRegImm32(IA32Reg reg, std::uint32_t dword)
{
    WriteByte(0x48); //???
    WriteByte(OpCode_MovRegImm32 | RegByte(reg));
    WriteDWord(dword);
}

void IA32Assembler::CallNear(IA32Reg reg)
{
    WriteByte(0xFF);
    WriteByte(OpCode_CallNear | RegByte(reg));
}

void IA32Assembler::CallFar(IA32Reg reg)
{
    //TODO
}

void IA32Assembler::RetNear(std::uint16_t word)
{
    if (word > 0)
    {
        WriteByte(OpCode_RetNearImm16);
        WriteWord(word);
    }
    else
        WriteByte(OpCode_RetNear);
}

void IA32Assembler::RetFar(std::uint16_t word)
{
    if (word > 0)
    {
        WriteByte(OpCode_RetFarImm16);
        WriteWord(word);
    }
    else
        WriteByte(OpCode_RetFar);
}


/*
 * ======= Private: =======
 */

void IA32Assembler::WriteByte(std::uint8_t byte)
{
    assembly_.push_back(byte);
}

void IA32Assembler::WriteWord(std::uint16_t word)
{
    Word val;
    val.i16 = word;
    assembly_.push_back(val.i8[1]);
    assembly_.push_back(val.i8[0]);
}

void IA32Assembler::WriteDWord(std::uint32_t dword)
{
    DWord val;
    val.i32 = dword;
    assembly_.push_back(val.i8[3]);
    assembly_.push_back(val.i8[2]);
    assembly_.push_back(val.i8[1]);
    assembly_.push_back(val.i8[0]);
}


} // /namespace LLGL



// ================================================================================
