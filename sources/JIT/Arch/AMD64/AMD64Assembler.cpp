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


/*
 * Internal members
 */

/*
List of registers that are used for the first couple of arguments.
Note the difference between Microsoft and Unix x64 calling conventions.
see https://en.wikipedia.org/wiki/X86_calling_conventions#List_of_x86_calling_conventions
*/
#ifdef _WIN32

// Microsoft x64 calling convention (Windows)
static const Reg g_amd64IntParams[] = { Reg::RCX, Reg::RDX, Reg::R8, Reg::R9 };
static const Reg g_amd64FltParams[] = { Reg::XMM0, Reg::XMM1, Reg::XMM2, Reg::XMM3 };

#else

// System V AMD64 ABI (Solaris, Linux, BSD, macOS)
static const Reg g_amd64IntParams[] = { Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9 };
static const Reg g_amd64FltParams[] = { Reg::XMM0, Reg::XMM1, Reg::XMM2, Reg::XMM3, Reg::XMM4, Reg::XMM5, Reg::XMM6, Reg::XMM7 };

#endif

static const std::size_t g_amd64IntParamsCount = sizeof(g_amd64IntParams)/sizeof(g_amd64IntParams[0]);
static const std::size_t g_amd64FltParamsCount = sizeof(g_amd64FltParams)/sizeof(g_amd64FltParams[0]);


/*
 * AMD64Assembler class
 */

void AMD64Assembler::Begin()
{
    PushReg(Reg::RBP);
    MovReg(Reg::RBP, Reg::RSP);
}

void AMD64Assembler::End()
{
    MovReg(Reg::RBP, Reg::RSP);//!!!
    PopReg(Reg::RBP);
    RetNear();
}

void AMD64Assembler::WriteFuncCall(const void* addr, const JITCallConv conv, bool farCall)
{
    const auto& args = GetArgs();
    
    /* Move first couple of arguments into registers */
    std::size_t numIntRegs = 0, numFltRegs = 0;
    std::size_t lastInt = ~0, lastFlt = ~0;
    std::size_t num = args.size();
    
    for (std::size_t i = 0; i < num; ++i)
    {
        const auto& arg = args[i];
        
        /* Determine destination register for argument */
        Reg dstReg = Reg::R10;
        bool isFloat = IsFloat(arg.type);
        
        if (isFloat && numFltRegs < g_amd64FltParamsCount)
        {
            dstReg  = g_amd64FltParams[numFltRegs++];
            lastFlt = i;
        }
        else if (!isFloat && numIntRegs < g_amd64IntParamsCount)
        {
            dstReg  = g_amd64IntParams[numIntRegs++];
            lastInt = i;
        }
        else
            break;
        
        /* Move value into destination register */
        if (dstReg >= Reg::R8 && dstReg <= Reg::R15)
        {
            /* R8-R15 registers are only supported for 64-bit operand size */
            MovRegImm64(dstReg, arg.value.i64);
        }
        else
        {
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
                    MovRegImm64(dstReg, arg.value.i64);
                    break;
                case ArgType::Float:
                    //TODO...
                    break;
                case ArgType::Double:
                    //TODO...
                    break;
            }
        }
    }
    
    /* Push remaining arguments onto stack */
    for (std::size_t i = 0; i < num; ++i)
    {
        auto iRev = num - i - 1u;
        const auto& arg = args[iRev];
        
        /* Check if argument has already been moved into a register */
        bool isFloat = IsFloat(arg.type);
        
        if ( (  isFloat && iRev == lastFlt ) ||
             ( !isFloat && iRev == lastInt ) )
        {
            break;
        }

        /* Push argument onto stack */
        switch (arg.type)
        {
            case ArgType::Byte:
                PushImm8(arg.value.i8);
                break;
            case ArgType::Word:
                PushImm16(arg.value.i16);
                break;
            case ArgType::DWord:
                PushImm32(arg.value.i32);
                break;
            case ArgType::QWord:
                //PushImm64(arg.value.i64);
                break;
            case ArgType::Ptr:
                //PushImm64(arg.value.i64);
                break;
            case ArgType::Float:
                PushImm32(arg.value.i32);
                break;
            case ArgType::Double:
                //PushImm64(arg.value.i64);
                break;
        }
    }
    
    /* Write 'call' instruction */
    MovRegImm64(Reg::RAX, reinterpret_cast<std::uint64_t>(addr));
    CallNear(Reg::RAX);
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

void AMD64Assembler::PushImm8(std::uint8_t byte)
{
    WriteByte(Opcode_PushImm8);
    WriteByte(byte);
}

void AMD64Assembler::PushImm16(std::uint16_t word)
{
    PushImm32(word);
}

void AMD64Assembler::PushImm32(std::uint32_t dword)
{
    WriteByte(Opcode_PushImm);
    WriteDWord(dword);
}

void AMD64Assembler::PopReg(const Reg reg)
{
    WriteByte(Opcode_PopReg | RegByte(reg));
}

void AMD64Assembler::MovReg(const Reg dst, const Reg src)
{
    if (Is64Reg(dst))
        WriteByte(REX_Prefix | REX_W);
    WriteByte(Opcode_MovRegMem);
    WriteByte(Operand_Mod11 | RegByte(dst) << 3 | RegByte(src));
}

void AMD64Assembler::MovRegImm32(const Reg reg, std::uint32_t dword)
{
    WriteByte(Opcode_MovRegImm | RegByte(reg));
    WriteDWord(dword);
}

void AMD64Assembler::MovRegImm64(const Reg reg, std::uint64_t qword)
{
    if (reg >= Reg::R8)
        WriteByte(REX_Prefix | REX_W | REX_B);
    else
        WriteByte(REX_Prefix | REX_W);
    WriteByte(Opcode_MovRegImm | RegByte(reg));
    WriteQWord(qword);
}

void AMD64Assembler::CallNear(const Reg reg)
{
    WriteByte(0xFF);
    WriteByte(Opcode_CallNear | Operand_Mod11 | RegByte(reg));
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
