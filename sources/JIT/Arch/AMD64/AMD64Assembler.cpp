/*
 * AMD64Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AMD64Assembler.h"
#include "AMD64Opcode.h"
#include <limits.h>


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
 * Internal functions
 */

// Size of byte (1), word (2), dword (4), qword (8), ptr (8), float (4), double (8)
static std::uint8_t GetArgSize(const ArgType t)
{
    static const std::uint8_t sizes[] = { 1, 2, 4, 8, 8, 4, 8 };
    return sizes[static_cast<std::uint8_t>(t)];
}


/*
 * AMD64Assembler class
 */

void AMD64Assembler::Begin()
{
    #if 0
    _ForceExcep();
    MovDQURegMem(Reg::XMM1, Reg::RSP, std::int32_t(0xF1));
    MovDQUMemReg(Reg::RSP, Reg::XMM2, std::int8_t(0xF1));
    #endif
    
    /* Reset data about local stack */
    localStack_ = 0;
    
    /* Store base stack pointer (RBP) */
    PushReg(Reg::RBP);
    MovReg(Reg::RBP, Reg::RSP);
    
    #if 0
    /* Store general purpose registers */
    //TODO: determine which registers must be stored
    for (std::size_t i = 0; i < g_amd64IntParamsCount; ++i)
        Push(g_amd64IntParams[i]);
    /*for (std::size_t i = 0; i < g_amd64FltParamsCount; ++i)
        Push(g_amd64FltParams[i]);*/
    PushReg(Reg::RAX);
    #endif
}

void AMD64Assembler::End()
{
    #if 0
    /* Pop local stack */
    if (localStack_ > 0)
        AddImm32(Reg::RSP, localStack_);
    
    /* Restore general purpose registers */
    //TODO: determine which registers must be restored
    PopReg(Reg::RAX);
    /*for (std::size_t i = 0; i < g_amd64FltParamsCount; ++i)
        Pop(g_amd64FltParams[g_amd64FltParamsCount - i - 1u]);*/
    for (std::size_t i = 0; i < g_amd64IntParamsCount; ++i)
        Pop(g_amd64IntParams[g_amd64IntParamsCount - i - 1u]);
    #endif

    /* Restore base stack pointer (RBP) */
    //MovReg(Reg::RSP, Reg::RBP);
    PopReg(Reg::RBP);
    RetNear();
    
    /* Write supplement at the end of program */
    ApplySupplements();
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
        Reg dstReg = Reg::RAX;
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
                    MovSSRegImm32(dstReg, arg.value.f32);
                    break;
                case ArgType::Double:
                    MovSDRegImm64(dstReg, arg.value.f64);
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
            case ArgType::Float:
                PushImm32(arg.value.i32);
                break;
            case ArgType::QWord:
            case ArgType::Ptr:
            case ArgType::Double:
                MovRegImm64(Reg::RAX, arg.value.i64);
                PushReg(Reg::RAX);
                break;
        }
        
        /* Increase local stack size by register width (8 bytes) */
        localStack_ += 8;
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

std::uint8_t AMD64Assembler::DispMod(const Displacement& disp) const
{
    if (disp.disp32 != 0)
    {
        if (disp.has32Bits)
            return Operand_Mod10; // disp32
        else
            return Operand_Mod01; // disp8
    }
    return 0;
}

std::uint8_t AMD64Assembler::ModRM(std::uint8_t mode, const Reg r0, const Reg r1) const
{
    std::uint8_t modRM = (mode | (RegByte(r0) << 3));
    
    if (r1 == Reg::RSP)
        modRM |= Operand_SIB;
    else
        modRM |= RegByte(r1);
    
    return modRM;
}

void AMD64Assembler::WriteOptREX(const Reg reg, bool defaultsTo64Bit)
{
    std::uint8_t prefix = 0;
    
    if (Is64Reg(reg))
    {
        if (!defaultsTo64Bit || reg >= Reg::R8)
        {
            prefix |= REX_W;
            if (reg >= Reg::R8)
                prefix |= REX_B;
        }
    }
    
    if (prefix != 0)
        WriteByte(REX_Prefix | prefix);
}

void AMD64Assembler::WriteOptDisp(const Displacement& disp)
{
    if (disp.disp32 != 0)
    {
        if (disp.has32Bits)
            WriteDWord(static_cast<std::uint32_t>(disp.disp32));
        else
            WriteByte(static_cast<std::uint8_t>(disp.disp8));
    }
}

void AMD64Assembler::BeginSupplement(const Arg& arg)
{
    Supplement supp;
    {
        supp.data.i64   = arg.value.i64;
        supp.dataSize   = GetArgSize(arg.type);
        supp.rip        = 0;
        supp.dstOffset  = GetAssembly().size();
    }
    supplements_.push_back(supp);
}

void AMD64Assembler::EndSupplement()
{
    /* Write RIP value for offset of next instruction */
    if (!supplements_.empty())
        supplements_.back().rip = GetAssembly().size();
}

void AMD64Assembler::ApplySupplements()
{
    auto& code = GetAssembly();
    
    std::uint32_t disp32 = 0;
    for (const auto& supp : supplements_)
    {
        /* Override displacement dummy */
        disp32 = static_cast<std::uint32_t>(code.size() - supp.rip);
        ::memcpy(&(code[supp.dstOffset]), &disp32, sizeof(disp32));
        
        /* Write supplement data */
        Write(supp.data.i8, supp.dataSize);
    }
}

void AMD64Assembler::ErrInvalidUseOfRSP()
{
    #ifdef LLGL_DEBUG
    //Error("invalid use of %RSP register in MOV instruction");
    #endif
}

/* ----- PUSH ----- */

void AMD64Assembler::PushReg(const Reg srcReg)
{
    WriteOptREX(srcReg, true);
    WriteByte(Opcode_PushReg | RegByte(srcReg));
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

void AMD64Assembler::Push(const Reg srcReg)
{
    if (IsFltReg(srcReg))
    {
        SubImm32(Reg::RSP, 16);
        MovDQUMemReg(Reg::RSP, srcReg, 0);
    }
    else
        PushReg(srcReg);
}

/* ----- POP ----- */

void AMD64Assembler::PopReg(const Reg dstReg)
{
    WriteByte(Opcode_PopReg | RegByte(dstReg));
}

void AMD64Assembler::Pop(const Reg dstReg)
{
    if (IsFltReg(dstReg))
    {
        MovDQURegMem(dstReg, Reg::RSP, 0);
        AddImm32(Reg::RSP, 16);
    }
    else
        PopReg(dstReg);
}

/* ----- MOV ----- */

void AMD64Assembler::MovReg(const Reg dstReg, const Reg srcReg)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_MovMemReg);
    WriteByte(Operand_Mod11 | RegByte(srcReg) << 3 | RegByte(dstReg));
}

void AMD64Assembler::MovRegImm32(const Reg dstReg, std::uint32_t dword)
{
    WriteByte(Opcode_MovRegImm | RegByte(dstReg));
    WriteDWord(dword);
}

void AMD64Assembler::MovRegImm64(const Reg dstReg, std::uint64_t qword)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_MovRegImm | RegByte(dstReg));
    WriteQWord(qword);
}

void AMD64Assembler::MovMemImm32(const Reg dstMemReg, std::uint32_t dword, const Displacement& disp)
{
    #ifdef LLGL_DEBUG
    if (dstMemReg == Reg::RSP)
        ErrInvalidUseOfRSP();
    #endif
    
    /* Write opcode */
    WriteOptREX(dstMemReg); // prefix
    WriteByte(Opcode_MovMemImm);
    WriteByte(DispMod(disp) | RegByte(dstMemReg));
    WriteOptDisp(disp); // displacement
    WriteDWord(dword); // immediate
}

void AMD64Assembler::MovMemReg(const Reg dstMemReg, const Reg srcReg, const Displacement& disp)
{
    WriteOptREX(srcReg); // prefix
    WriteByte(Opcode_MovMemReg);
    WriteByte(DispMod(disp) | RegByte(srcReg) << 3 | RegByte(dstMemReg));
    WriteOptDisp(disp); // displacement
}

#if 0 // UNUSED
// dstReg: XMM0-XMM7, srcMemReg: RAX-RDI
void AMD64Assembler::MovSSRegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovSSRegMem, 3);
    WriteByte(DispMod(disp) | (RegByte(dstReg) << 3) | RegByte(srcMemReg));
    WriteOptDisp(disp); // displacement
}

// dstReg: XMM0-XMM7, srcMemReg: RAX-RDI
void AMD64Assembler::MovSDRegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovSDRegMem, 3);
    WriteByte(DispMod(disp) | (RegByte(dstReg) << 3) | RegByte(srcMemReg));
    WriteOptDisp(disp); // displacement
}
#endif // /UNUSED

void AMD64Assembler::MovSSRegImm32(const Reg dstReg, float f32)
{
    Write(OpcodeSSE2_MovSSRegMem, 3);
    WriteByte((RegByte(dstReg) << 3) | Operand_RIP);
    
    Arg arg;
    arg.type        = ArgType::Float;
    arg.value.f32   = f32;
    BeginSupplement(arg);
    
    WriteDWord(0); // displacement (dummy)
    
    EndSupplement();
}

void AMD64Assembler::MovSDRegImm64(const Reg dstReg, double f64)
{
    Write(OpcodeSSE2_MovSDRegMem, 3);
    WriteByte((RegByte(dstReg) << 3) | Operand_RIP);
    
    Arg arg;
    arg.type        = ArgType::Double;
    arg.value.f64   = f64;
    BeginSupplement(arg);
    
    WriteDWord(0); // displacement (dummy)
    
    EndSupplement();
}

// dstReg: XMM0-XMM7, srcMemReg: RAX-RDI
void AMD64Assembler::MovDQURegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovDQURegMem, 3);
    WriteByte(ModRM(DispMod(disp), dstReg, srcMemReg));
    if (srcMemReg == Reg::RSP)
        WriteByte((RegByte(Reg::RSP) << 3) | RegByte(Reg::RSP));
    WriteOptDisp(disp);
}

// dstMemReg: RAX-RDI, srcReg: XMM0-XMM7
void AMD64Assembler::MovDQUMemReg(const Reg dstMemReg, const Reg srcReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovDQUMemReg, 3);
    WriteByte(ModRM(DispMod(disp), srcReg, dstMemReg));
    if (dstMemReg == Reg::RSP)
        WriteByte((RegByte(Reg::RSP) << 3) | RegByte(Reg::RSP));
    WriteOptDisp(disp);
}

/* ----- ADD ----- */

void AMD64Assembler::AddImm32(const Reg dst, std::uint32_t dword)
{
    WriteOptREX(dst);
    WriteByte(Opcode_AddImm);
    WriteByte(Operand_Mod11 | RegByte(dst));
    WriteDWord(dword);
}

/* ----- SUB ----- */

// Opcode: 81 /5 id
void AMD64Assembler::SubImm32(const Reg dst, std::uint32_t dword)
{
    WriteOptREX(dst);
    WriteByte(Opcode_SubImm);
    WriteByte(Operand_Mod11 | (5u << 3) | RegByte(dst));
    WriteDWord(dword);
}

/* ----- DIV ----- */

// Opcode: F7 /6
// Divide RDX:RAX -> Quotient: RAX, Remainder: RDX
void AMD64Assembler::DivReg(const Reg src)
{
    WriteOptREX(src);
    WriteByte(Opcode_DivReg);
    WriteByte(Operand_Mod11 | (6u << 3) | RegByte(src));
}

/* ----- CALL ----- */

void AMD64Assembler::CallNear(const Reg reg)
{
    WriteByte(0xFF);
    WriteByte(Opcode_CallNear | Operand_Mod11 | RegByte(reg));
}

/* ----- RET ----- */

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

/* ----- INT ----- */

void AMD64Assembler::Int(std::uint8_t byte)
{
    WriteByte(Opcode_Int);
    WriteByte(byte);
}

#if 1//TESTING
void AMD64Assembler::_ForceExcep()
{
    //TEST: generate division by zero instruction to force exception
    MovRegImm32(Reg::RAX, 0);
    MovRegImm32(Reg::RDX, 0);
    DivReg(Reg::RAX);
}
#endif


/*
 * Displacement structure
 */

AMD64Assembler::Displacement::Displacement(std::int8_t disp8) :
    has32Bits { false },
    disp8     { disp8 }
{
}

AMD64Assembler::Displacement::Displacement(std::int32_t disp32) :
    has32Bits { true },
    disp32    { disp32 }
{
}


} // /namespace JIT

} // /namespace LLGL



// ================================================================================
