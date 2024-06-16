/*
 * JITCompiler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "JITCompiler.h"
#include "AssemblyTypes.h"
#include "../Core/CoreUtils.h"
#include "../Core/PrintfUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Report.h>
#include <LLGL/Log.h>

#include <LLGL/Platform/Platform.h>
#if defined LLGL_OS_WIN32
#   include "Platform/Win32/Win32JITProgram.h"
#else
#   include "Platform/POSIX/POSIXJITProgram.h"
#endif

#if defined LLGL_ARCH_ARM
//#   include "Arch/ARM/ARMAssembler.h"
#elif defined LLGL_ARCH_AMD64
#   include "Arch/AMD64/AMD64Assembler.h"
#elif defined LLGL_ARCH_IA32
#   include "Arch/IA32/IA32Assembler.h"
#endif


namespace LLGL
{


using namespace JIT;

std::unique_ptr<JITCompiler> JITCompiler::Create()
{
    std::unique_ptr<JITCompiler> compiler;

    /* Create JIT compiler for current CPU architecture */
    #if defined LLGL_ARCH_ARM
    //TODO
    #elif defined LLGL_ARCH_AMD64
    compiler = MakeUnique<AMD64Assembler>();
    #elif defined LLGL_ARCH_IA32
    compiler = MakeUnique<IA32Assembler>();
    #endif

    /* Store meta data */
    if (compiler)
        compiler->littleEndian_ = compiler->IsLittleEndian();

    return compiler;
}

UTF8String JITCompiler::DumpAssembly(std::size_t bytesPerLine) const
{
    Report output;

    const auto& bytes = GetAssembly();
    for_range(i, bytes.size())
    {
        /* Write current hex value and separator */
        output.Printf("0x%02X", static_cast<int>(bytes[i]));
        if (i + 1 < for_range_end(i))
            output.Printf(bytesPerLine > 0 && (i + 1) % bytesPerLine == 0 ? "\n" : " ");
    }

    return UTF8String{ output.GetText() };
}

std::unique_ptr<JITProgram> JITCompiler::FlushProgram()
{
    if (!assembly_.empty())
    {
        auto program = JITProgram::Create(assembly_.data(), assembly_.size());
        assembly_.clear();
        return program;
    }
    return nullptr;
}

void JITCompiler::EntryPointVarArgs(const std::initializer_list<JIT::ArgType>& varArgTypes)
{
    entryVarArgs_.reserve(varArgTypes.size());
    for (JIT::ArgType type : varArgTypes)
        entryVarArgs_.push_back(type);
}

std::uint8_t JITCompiler::StackAlloc(std::uint32_t size)
{
    auto idx = static_cast<std::uint8_t>(stackAllocs_.size());
    stackAllocs_.push_back(size);
    return idx;
}

void JITCompiler::PushVarArg(std::uint8_t idx)
{
    if (idx < entryVarArgs_.size() && idx < 0xF)
    {
        Arg arg;
        {
            arg.type        = entryVarArgs_[idx];
            arg.param       = idx;
            arg.value.i64   = 0;
        }
        args_.push_back(arg);
    }
}

void JITCompiler::PushStackPtr(std::uint8_t idx)
{
    if (idx < stackAllocs_.size())
    {
        Arg arg;
        {
            arg.type        = ArgType::StackPtr;
            arg.param       = 0xF;
            arg.value.i8    = idx;
        }
        args_.push_back(arg);
    }
}

void JITCompiler::PushPtr(const void* value)
{
    Arg arg;
    {
        arg.type        = ArgType::Ptr;
        arg.param       = 0xF;
        arg.value.ptr   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushByte(std::uint8_t value)
{
    Arg arg;
    {
        arg.type        = ArgType::Byte;
        arg.param       = 0xF;
        arg.value.i64   = 0;
        arg.value.i8    = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushWord(std::uint16_t value)
{
    Arg arg;
    {
        arg.type        = ArgType::Word;
        arg.param       = 0xF;
        arg.value.i64   = 0;
        arg.value.i16   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushDWord(std::uint32_t value)
{
    Arg arg;
    {
        arg.type        = ArgType::DWord;
        arg.param       = 0xF;
        arg.value.i64   = 0;
        arg.value.i32   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushQWord(std::uint64_t value)
{
    Arg arg;
    {
        arg.type        = ArgType::QWord;
        arg.param       = 0xF;
        arg.value.i64   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushFloat(float value)
{
    Arg arg;
    {
        arg.type        = ArgType::Float;
        arg.param       = 0xF;
        arg.value.i64   = 0;
        arg.value.f32   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::PushDouble(double value)
{
    Arg arg;
    {
        arg.type        = ArgType::Double;
        arg.param       = 0xF;
        arg.value.f64   = value;
    }
    args_.push_back(arg);
}

void JITCompiler::FuncCall(const void* addr, JITCallConv conv, bool farCall)
{
    WriteFuncCall(addr, conv, farCall);
    args_.clear();
}

void JITCompiler::PushSizeT(std::uint64_t value)
{
    #ifdef LLGL_ARCH_IA32
    PushDWord(static_cast<std::uint32_t>(value));
    #else
    PushQWord(value);
    #endif
}

void JITCompiler::PushSSizeT(std::int64_t value)
{
    PushSizeT(static_cast<std::uint64_t>(value));
}


/*
 * ======= Protected: =======
 */

void JITCompiler::Write(const void* data, std::size_t size)
{
    assembly_.reserve(assembly_.size() + size);
    auto* byteAlignedData = reinterpret_cast<const std::uint8_t*>(data);
    #if 0
    if (littleEndian_)
    {
        /* Encode for little endian */
        for_range(i, size)
            assembly_.push_back(byteAlignedData[size - i - 1u]);
    }
    else
    #endif
    {
        /* Encode for big endian */
        for_range(i, size)
            assembly_.push_back(byteAlignedData[i]);
    }
}

void JITCompiler::WriteByte(std::uint8_t data)
{
    Write(&data, sizeof(data));
}

void JITCompiler::WriteWord(std::uint16_t data)
{
    Write(&data, sizeof(data));
}

void JITCompiler::WriteDWord(std::uint32_t data)
{
    Write(&data, sizeof(data));
}

void JITCompiler::WriteQWord(std::uint64_t data)
{
    Write(&data, sizeof(data));
}

void JITCompiler::WritePtr(const void* data)
{
    Write(&data, sizeof(data));
}


#ifdef LLGL_DEBUG

void Test1(int x, int8_t b, uint16_t h, uint64_t q, int i5, int i6, int i7, int8_t i8, uint64_t i9)
{
    Log::Printf(
        "%s: x = %d, b = %d, h = 0x%04X, q = %" PRIu64 ", i = { %d, %d, %d, %d, %" PRIu64 " }\n",
        __FUNCTION__, x, (int)b, (int)h, q, i5, i6, i7, (int)i8, i9
    );
}

void Test2(float f, double d)
{
    Log::Printf(
        "%s: f = %f, d = %f\n",
        __FUNCTION__, (double)f, d
    );
}

void Call_Test1()
{
    Test1(1, 2, 3, 4, 5, 6, 7, 8, 9);
}

LLGL_EXPORT void TestJIT1()
{
    #if 0//TEST
    //Test2(1.23f, 4.56);
    Call_Test1();
    #endif

    auto comp = JITCompiler::Create();

    comp->EntryPointVarArgs({ JIT::ArgType::DWord, JIT::ArgType::Float, JIT::ArgType::Double });

    comp->Begin();

    #if 1
    #   if 0
    comp->PushDWord(42);
    #   else
    comp->PushVarArg(0);
    #   endif
    comp->PushByte(-3);
    comp->PushWord(0x40);
    comp->PushQWord(999999ull);
    comp->PushDWord(1);
    comp->PushDWord(2);
    comp->PushDWord(3);
    comp->PushByte(4);
    comp->PushQWord(888888ull);
    comp->FuncCall(reinterpret_cast<const void*>(Test1));

    int a[] = { 1, 2, 3 };
    int b[] = { 4, 5, 6 };

    comp->PushPtr(b);
    comp->PushPtr(a);
    comp->PushQWord(sizeof(a));
    comp->FuncCall(reinterpret_cast<const void*>(::memcpy));
    #endif

    #if 1
    #   if 0
    comp->PushFloat(-1.2345f);
    comp->PushDouble(3.14);
    #   else
    comp->PushVarArg(1);
    comp->PushVarArg(2);
    #   endif
    comp->FuncCall(reinterpret_cast<const void*>(Test2));
    #endif

    comp->End();

    auto prog = comp->FlushProgram();

    prog->GetEntryPoint()(28, 2.3f, 4.5);
}

#endif // /LLGL_DEBUG


} // /namespace LLGL



// ================================================================================
