/*
 * JITCompiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "JITCompiler.h"
#include "AssemblyTypes.h"
#include <LLGL/Platform/Platform.h>
#include "../Core/Helper.h"

#if defined LLGL_OS_WIN32
#   include "Platform/Win32/Win32JITProgram.h"
#else
//#   include "Platform/POSIX/POSIXJITProgram.h"
#endif

#if defined LLGL_ARCH_ARM
//#   include "Arch/ARM/ARMAssembler.h"
#elif defined LLGL_ARCH_AMD64
//#   include "Arch/AMD64/AMD64Assembler.h"
#elif defined LLGL_ARCH_IA32
#   include "Arch/IA32/IA32Assembler.h"
#endif


namespace LLGL
{


std::unique_ptr<JITCompiler> JITCompiler::Create()
{
    #if defined LLGL_ARCH_ARM
    return nullptr; //TODO
    #elif defined LLGL_ARCH_AMD64
    return nullptr; //TODO
    #elif defined LLGL_ARCH_IA32
    return MakeUnique<JIT::IA32::IA32Assembler>();
    #else
    return nullptr;
    #endif
}

void JITCompiler::Begin()
{
    littleEndian_ = IsLittleEndian();
}

std::unique_ptr<JITProgram> JITCompiler::End()
{
    auto program = JITProgram::Create(assembly_.data(), assembly_.size());
    assembly_.clear();
    return program;
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


} // /namespace LLGL



// ================================================================================
