/*
 * JITCompiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_JIT_COMPILER_H
#define LLGL_JIT_COMPILER_H


#include "JITProgram.h"
#include "AssemblyTypes.h"
#include <LLGL/NonCopyable.h>
#include <vector>
#include <memory>
#include <cstdint>


namespace LLGL
{


// Enumeration for calling conventions.
enum class JITCallConv
{
    CDecl,      // '__cdecl' to internal function
    StdCall,    // '__stdcall' to internal function
    ThisCall,   // '__thiscall' to internal function
};

// IA-32 (a.k.a. x86) assembly code generator.
class LLGL_EXPORT JITCompiler : public NonCopyable
{

    public:

        /*
        Instantiates a new JIT compiler for the current hardware architecture (i.e. x86, x64, ARM),
        or null if the architecture is not supported.
        */
        static std::unique_ptr<JITCompiler> Create();
    
        // Flushes the currently build program, or null if no program was build.
        std::unique_ptr<JITProgram> FlushProgram();

    public:
    
        virtual void Begin() = 0;
        virtual void End() = 0;
    
        void PushThisPtr(const void* value);
        void PushPtr(const void* value);
        void PushByte(std::uint8_t value);
        void PushWord(std::uint16_t value);
        void PushDWord(std::uint32_t value);
        void PushQWord(std::uint64_t value);

        void FuncCall(const void* addr, const JITCallConv conv, bool farCall);

    protected:

        JITCompiler() = default;

        virtual bool IsLittleEndian() const = 0;
        virtual void WriteFuncCall(const void* addr, const JITCallConv conv, bool farCall) = 0;

        void Write(const void* data, std::size_t size);
        void WriteByte(std::uint8_t data);
        void WriteWord(std::uint16_t data);
        void WriteDWord(std::uint32_t data);
        void WriteQWord(std::uint64_t data);
        void WritePtr(const void* data);

        // Returns the assembly code.
        inline const std::vector<std::uint8_t>& GetAssembly() const
        {
            return assembly_;
        }

        // Returns the list of function arguments.
        inline const std::vector<JIT::Arg>& GetArgs() const
        {
            return args_;
        }

        // Returns the 'this'-pointer argument, or null if there is no such argument.
        inline const void* GetThisPtr() const
        {
            return thisPtr_;
        }

    private:

        bool                        littleEndian_   = false;
        std::vector<std::uint8_t>   assembly_;
    
        std::vector<JIT::Arg>       args_;
        const void*                 thisPtr_        = nullptr;

};

#ifdef LLGL_DEBUG

LLGL_EXPORT void TestJIT1();

#endif // /LLGL_DEBUG


} // /namespace LLGL


#endif



// ================================================================================
