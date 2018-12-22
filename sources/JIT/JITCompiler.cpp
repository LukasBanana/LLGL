/*
 * JITCompiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "JITCompiler.h"
#include "AssemblyTypes.h"
#include "../Core/Helper.h"

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

#ifdef LLGL_DEBUG
#   include <iostream>
#endif // /LLGL_DEBUG


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
    compiler->littleEndian_ = compiler->IsLittleEndian();
    
    return compiler;
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

void JITCompiler::PushThisPtr(const void* value)
{
    thisPtr_ = value;
}

void JITCompiler::PushPtr(const void* value)
{
    Arg arg;
    arg.type = ArgType::Ptr;
    arg.value.ptr = value;
    args_.push_back(arg);
}

void JITCompiler::PushByte(std::uint8_t value)
{
    Arg arg;
    arg.type        = ArgType::Byte;
    arg.value.i8    = value;
    args_.push_back(arg);
}

void JITCompiler::PushWord(std::uint16_t value)
{
    Arg arg;
    arg.type        = ArgType::Word;
    arg.value.i16   = value;
    args_.push_back(arg);
}

void JITCompiler::PushDWord(std::uint32_t value)
{
    Arg arg;
    arg.type        = ArgType::DWord;
    arg.value.i32   = value;
    args_.push_back(arg);
}

void JITCompiler::PushQWord(std::uint64_t value)
{
    Arg arg;
    arg.type        = ArgType::QWord;
    arg.value.i64   = value;
    args_.push_back(arg);
}

void JITCompiler::FuncCall(const void* addr, const JITCallConv conv, bool farCall)
{
    WriteFuncCall(addr, conv, farCall);
    args_.clear();
    thisPtr_ = nullptr;
}


/*
 * ======= Protected: =======
 */

void JITCompiler::Write(const void* data, std::size_t size)
{
    assembly_.reserve(assembly_.size() + size);
    auto byteAlignedData = reinterpret_cast<const std::int8_t*>(data);
    if (littleEndian_)
    {
        /* Encode for little endian */
        for (std::size_t i = 0; i < size; ++i)
            assembly_.push_back(byteAlignedData[size - i - 1u]);
    }
    else
    {
        /* Encode for big endian */
        for (std::size_t i = 0; i < size; ++i)
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

void LLGL_API_STDCALL Test_Foo()
{
    std::cout << __FUNCTION__ << /*": x = " << x <<*/ std::endl;
}

LLGL_EXPORT void TestJIT1()
{
    auto comp = JITCompiler::Create();
    
    comp->Begin();
    comp->PushDWord(42);
    comp->FuncCall(reinterpret_cast<const void*>(Test_Foo), JITCallConv::StdCall, false);
    comp->End();
    
    auto prog = comp->FlushProgram();
    
    prog->Execute();
}

#endif // /LLGL_DEBUG


} // /namespace LLGL



// ================================================================================
