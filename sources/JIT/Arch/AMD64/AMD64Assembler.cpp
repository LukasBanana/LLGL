/*
 * AMD64Assembler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AMD64Assembler.h"
#include "AMD64Opcode.h"
#include <limits.h>

#include <fstream>//!!!
#include <iomanip>


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

/*
Microsoft x64 calling convention (Windows)
Preserved for caller: ???
*/
static const Reg g_amd64IntParams[] = { Reg::RCX, Reg::RDX, Reg::R8, Reg::R9 };
static const Reg g_amd64FltParams[] = { Reg::XMM0, Reg::XMM1, Reg::XMM2, Reg::XMM3 };
static const Reg g_amd64TempReg     = Reg::RAX;

#else

/*
System V AMD64 ABI (Solaris, Linux, BSD, macOS)
Preserved for caller: RBP, RBX, R12-R15
*/
static const Reg g_amd64IntParams[] = { Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9 };
static const Reg g_amd64FltParams[] = { Reg::XMM0, Reg::XMM1, Reg::XMM2, Reg::XMM3, Reg::XMM4, Reg::XMM5, Reg::XMM6, Reg::XMM7 };
static const Reg g_amd64TempReg     = Reg::RAX;

#endif

static const std::size_t g_amd64IntParamsCount = sizeof(g_amd64IntParams)/sizeof(g_amd64IntParams[0]);
static const std::size_t g_amd64FltParamsCount = sizeof(g_amd64FltParams)/sizeof(g_amd64FltParams[0]);


/*
 * Internal functions
 */

// Size of byte (1), word (2), dword (4), qword (8), ptr (8), stack-ptr (8), float (4), double (8)
static std::uint8_t GetArgSize(const ArgType t)
{
    static const std::uint8_t sizes[] = { 1, 2, 4, 8, 8, 8, 4, 8 };
    return sizes[static_cast<std::uint8_t>(t)];
}


/*
 * AMD64Assembler class
 */

void AMD64Assembler::Begin()
{
    #if 0//TEST
    _ForceExcep();
    XOrReg(Reg::EAX, Reg::EAX);
    XOrReg(Reg::EAX, Reg::EBX);
    XOrReg(Reg::RAX, Reg::RBX);
    XOrReg(Reg::RDI, Reg::RSI);
    #endif

    /* Reset data about local stack */
    localStackSize_ = 128;//0;
    paramStackSize_ = 0;

    /* Write entry point prologue */
    WritePrologue();
    WriteStackFrame(GetEntryVarArgs(), GetStackAllocs());
}

void AMD64Assembler::End()
{
    /* Pop local stack */
    if (localStackSize_ > 0)
        AddImm32(Reg::RSP, localStackSize_);

    /* Write entry point epilogue and append supplement at the end of program */
    WriteEpilogue();
    ApplySupplements();

    // TEST: write program to file
    #if 0
    {
        std::ofstream f("JITProgram.txt");
        DumpAssembly(f, true);
    }
    exit(0);
    #endif // /TEST
}

void AMD64Assembler::WriteFuncCall(const void* addr, JITCallConv conv, bool farCall)
{
    const auto& args = GetArgs();

    /* Move first couple of arguments into registers */
    std::size_t numIntRegs = 0, numFltRegs = 0;
    std::size_t lastInt, lastFlt;
    std::size_t num = args.size();

    lastInt = lastFlt = std::numeric_limits<std::size_t>::max();

    for (std::size_t i = 0; i < num; ++i)
    {
        const auto& arg = args[i];

        /* Determine destination register for argument */
        Reg dstReg = g_amd64TempReg;
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

        if (arg.param < 0xF)
        {
            if (arg.param < varArgDisp_.size())
            {
                /* Move parameter from local stack into destination register */
                if (IsFltReg(dstReg))
                    MovDQURegMem(dstReg, Reg::RBP, varArgDisp_[arg.param]);
                else
                    MovRegMem(dstReg, Reg::RBP, varArgDisp_[arg.param]);
            }
        }
        else if (dstReg >= Reg::R8 && dstReg <= Reg::R15)
        {
            /* R8-R15 registers are only supported for 64-bit operand size */
            MovRegImm64(dstReg, arg.value.i64);
        }
        else
        {
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
                    MovRegImm64(dstReg, arg.value.i64);
                    break;
                case ArgType::StackPtr:
                    MovReg(dstReg, Reg::RBP);
                    SubImm32(dstReg, stackChunkOffsets_[arg.value.i8]);
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
    Displacement stackDisp;

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
            case ArgType::Word:
            case ArgType::DWord:
            case ArgType::Float:
                MovMemImm32(Reg::RSP, arg.value.i32, stackDisp);
                stackDisp.disp8 += 8;
                break;
            case ArgType::QWord:
            case ArgType::Ptr:
            case ArgType::Double:
                MovRegImm64(g_amd64TempReg, arg.value.i64);
                MovMemReg(Reg::RSP, g_amd64TempReg, stackDisp);
                stackDisp.disp8 += 8;
                break;
            case ArgType::StackPtr:
                MovReg(g_amd64TempReg, Reg::RBP);
                SubImm32(g_amd64TempReg, stackChunkOffsets_[arg.value.i8]);
                MovMemReg(Reg::RSP, g_amd64TempReg, stackDisp);
                stackDisp.disp8 += 8;
                break;
        }
    }

    /* Write 'call' instruction */
    MovRegImm64(g_amd64TempReg, reinterpret_cast<std::uint64_t>(addr));
    CallNear(g_amd64TempReg);
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

std::uint8_t AMD64Assembler::ModRM(std::uint8_t mode, Reg r0, Reg r1) const
{
    std::uint8_t modRM = (mode | (RegByte(r0) << 3));

    if (r1 == Reg::RSP)
        modRM |= Operand_SIB;
    else
        modRM |= RegByte(r1);

    return modRM;
}

void AMD64Assembler::WritePrologue()
{
    /* Store base stack pointer (RBP) */
    PushReg(Reg::RBP);
    MovReg(Reg::RBP, Reg::RSP);

    #if 1
    /* Store general purpose registers */
    //TODO: determine which registers must be stored
    PushReg(Reg::RBX);
    #endif
}

void AMD64Assembler::WriteEpilogue()
{
    #if 1
    /* Restore general purpose registers */
    //TODO: determine which registers must be restored
    PopReg(Reg::RBX);
    #endif

    /* Restore base stack pointer (RBP) */
    PopReg(Reg::RBP);
    RetNear(paramStackSize_);
}

void AMD64Assembler::WriteStackFrame(
    const std::vector<JIT::ArgType>&    varArgTypes,
    const std::vector<std::uint32_t>&   stackChunks)
{
    /* Determine required stack size for variadic arguments */
    std::uint32_t varArgSize = 0;
    for (auto type : varArgTypes)
        varArgSize += (IsFloat(type) ? 16 : 8);

    /* Determine required stack size for allocations */
    std::uint32_t stackChunksSize = 0;
    for (auto chunk : stackChunks)
        stackChunksSize += chunk;

    /* Allocate local stack */
    localStackSize_ += varArgSize;

    std::uint32_t chunkStackOffset = localStackSize_ + 8;

    localStackSize_ += stackChunksSize;

    if (localStackSize_ > 0)
        SubImm32(Reg::RSP, localStackSize_);

    /* Store parameters in local stack */
    std::size_t numIntRegs = 0, numFltRegs = 0;
    std::int8_t paramStackOffset = 16; // first parameter at [EBP+16]
    std::int8_t localStackOffset = -16; // local variables after preserved EBX

    for (auto type : varArgTypes)
    {
        bool isFloat = IsFloat(type);
        Reg srcReg = g_amd64TempReg;

        if (isFloat && numFltRegs < g_amd64FltParamsCount)
        {
            /* Get parameter from floating-point register */
            srcReg = g_amd64FltParams[numFltRegs++];
        }
        else if (!isFloat && numIntRegs < g_amd64IntParamsCount)
        {
            /* Get parameter from integer register */
            srcReg = g_amd64IntParams[numIntRegs++];
        }
        else
        {
            /* Load parameter from stack */
            MovRegMem(srcReg, Reg::RBP, Disp8{ paramStackOffset });
            paramStackOffset += 8;
            paramStackSize_ += 8;
        }

        /* Store parameter in local stack */
        if (IsFltReg(srcReg))
        {
            localStackOffset -= 16; // SSE2 register size of 128 bits
            MovDQUMemReg(Reg::RBP, srcReg, Disp8{ localStackOffset });
        }
        else
        {
            localStackOffset -= 8; // x64 register size of 64 bits
            MovMemReg(Reg::RBP, srcReg, Disp8{ localStackOffset });
        }

        /* Store parameter offset within stack frame */
        varArgDisp_.push_back(Disp8{ localStackOffset });
    }

    /* Determine stack base for arguments of subsequent calls */
    argStackBase_.disp8 = localStackOffset;

    /* Determine stack base for allocated stack chunks */
    stackChunkOffsets_.reserve(stackChunks.size());
    for (auto chunk : stackChunks)
    {
        chunkStackOffset += static_cast<std::int32_t>(chunk);
        stackChunkOffsets_.push_back(chunkStackOffset);
    }
}

void AMD64Assembler::WriteOptREX(Reg reg, bool defaultsTo64Bit)
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

void AMD64Assembler::WriteOptSIB(Reg reg)
{
    if (reg == Reg::RSP)
        WriteByte((RegByte(reg) << 3) | RegByte(reg));
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

    for (const auto& supp : supplements_)
    {
        /* Override displacement dummy */
        std::uint32_t disp32 = static_cast<std::uint32_t>(code.size() - supp.rip);
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

void AMD64Assembler::PushReg(Reg srcReg)
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

void AMD64Assembler::Push(Reg srcReg)
{
    if (IsFltReg(srcReg))
    {
        SubImm32(Reg::RSP, 16);
        MovDQUMemReg(Reg::RSP, srcReg, {});
    }
    else
        PushReg(srcReg);
}

/* ----- POP ----- */

void AMD64Assembler::PopReg(Reg dstReg)
{
    WriteOptREX(dstReg, true);
    WriteByte(Opcode_PopReg | RegByte(dstReg));
}

void AMD64Assembler::Pop(Reg dstReg)
{
    if (IsFltReg(dstReg))
    {
        MovDQURegMem(dstReg, Reg::RSP, {});
        AddImm32(Reg::RSP, 16);
    }
    else
        PopReg(dstReg);
}

/* ----- MOV ----- */

// Opcode: 89 /r
void AMD64Assembler::MovReg(Reg dstReg, Reg srcReg)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_MovMemReg);
    WriteByte(Operand_Mod11 | RegByte(srcReg) << 3 | RegByte(dstReg));
}

void AMD64Assembler::MovRegImm32(Reg dstReg, std::uint32_t dword)
{
    if (dword != 0)
    {
        WriteByte(Opcode_MovRegImm | RegByte(dstReg));
        WriteDWord(dword);
    }
    else
        XOrReg(dstReg, dstReg);
}

void AMD64Assembler::MovRegImm64(Reg dstReg, std::uint64_t qword)
{
    if (qword != 0)
    {
        WriteOptREX(dstReg);
        WriteByte(Opcode_MovRegImm | RegByte(dstReg));
        WriteQWord(qword);
    }
    else
        XOrReg(dstReg, dstReg);
}

void AMD64Assembler::MovMemImm32(Reg dstMemReg, std::uint32_t dword, const Displacement& disp)
{
    WriteOptREX(dstMemReg); // prefix
    WriteByte(Opcode_MovMemImm);
    WriteByte(DispMod(disp) | RegByte(dstMemReg));
    WriteOptSIB(dstMemReg);
    WriteOptDisp(disp); // displacement
    WriteDWord(dword); // immediate
}

void AMD64Assembler::MovMemReg(Reg dstMemReg, Reg srcReg, const Displacement& disp)
{
    WriteOptREX(srcReg); // prefix
    WriteByte(Opcode_MovMemReg);
    WriteByte(ModRM(DispMod(disp), srcReg, dstMemReg));
    WriteOptSIB(dstMemReg);
    WriteOptDisp(disp); // displacement
}

void AMD64Assembler::MovRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_MovRegMem);
    WriteByte(ModRM(DispMod(disp), dstReg, srcMemReg));
    WriteOptSIB(srcMemReg);
    WriteOptDisp(disp);
}

#if 0 // UNUSED
// dstReg: XMM0-XMM7, srcMemReg: RAX-RDI
void AMD64Assembler::MovSSRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovSSRegMem, 3);
    WriteByte(DispMod(disp) | (RegByte(dstReg) << 3) | RegByte(srcMemReg));
    WriteOptDisp(disp); // displacement
}

// dstReg: XMM0-XMM7, srcMemReg: RAX-RDI
void AMD64Assembler::MovSDRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovSDRegMem, 3);
    WriteByte(DispMod(disp) | (RegByte(dstReg) << 3) | RegByte(srcMemReg));
    WriteOptDisp(disp); // displacement
}
#endif // /UNUSED

void AMD64Assembler::MovSSRegImm32(Reg dstReg, float f32)
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

void AMD64Assembler::MovSDRegImm64(Reg dstReg, double f64)
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
void AMD64Assembler::MovDQURegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovDQURegMem, 3);
    WriteByte(ModRM(DispMod(disp), dstReg, srcMemReg));
    WriteOptSIB(srcMemReg);
    WriteOptDisp(disp);
}

// dstMemReg: RAX-RDI, srcReg: XMM0-XMM7
void AMD64Assembler::MovDQUMemReg(Reg dstMemReg, Reg srcReg, const Displacement& disp)
{
    Write(OpcodeSSE2_MovDQUMemReg, 3);
    WriteByte(ModRM(DispMod(disp), srcReg, dstMemReg));
    WriteOptSIB(dstMemReg);
    WriteOptDisp(disp);
}

/* ----- ADD ----- */

void AMD64Assembler::AddImm32(Reg dst, std::uint32_t dword)
{
    WriteOptREX(dst);
    WriteByte(Opcode_AddImm);
    WriteByte(Operand_Mod11 | RegByte(dst));
    WriteDWord(dword);
}

/* ----- SUB ----- */

// Opcode: 81 /5 id
void AMD64Assembler::SubImm32(Reg dstReg, std::uint32_t dword)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_SubImm);
    WriteByte(Operand_Mod11 | (5u << 3) | RegByte(dstReg));
    WriteDWord(dword);
}

/* ----- DIV ----- */

// Opcode: F7 /6
// Divide RDX:RAX -> Quotient: RAX, Remainder: RDX
void AMD64Assembler::DivReg(Reg srcReg)
{
    WriteOptREX(srcReg);
    WriteByte(Opcode_DivReg);
    WriteByte(Operand_Mod11 | (6u << 3) | RegByte(srcReg));
}

/* ----- XOR ----- */

// Opcode: 31 /r
void AMD64Assembler::XOrReg(Reg dstReg, Reg srcReg)
{
    WriteOptREX(dstReg);
    WriteByte(Opcode_XOrMemReg);
    WriteByte(Operand_Mod11 | RegByte(srcReg) << 3 | RegByte(dstReg));
}

/* ----- CALL ----- */

void AMD64Assembler::CallNear(Reg reg)
{
    WriteOptREX(reg, true);
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

AMD64Assembler::Displacement::Displacement() :
    has32Bits { false },
    disp32    { 0     }
{
}

AMD64Assembler::Disp8::Disp8(std::int8_t disp)
{
    has32Bits   = false;
    disp8       = disp;
}

AMD64Assembler::Disp32::Disp32(std::int32_t disp)
{
    has32Bits   = true;
    disp32      = disp;
}


} // /namespace JIT

} // /namespace LLGL



// ================================================================================
