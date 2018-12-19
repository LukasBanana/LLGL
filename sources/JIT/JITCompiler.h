/*
 * JITCompiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_JIT_COMPILER_H
#define LLGL_JIT_COMPILER_H


#include "JITProgram.h"
#include <LLGL/NonCopyable.h>
#include <vector>
#include <memory>
#include <cstdint>


namespace LLGL
{


// Enumeration for calling conventions.
enum class JITCall
{
    StdFarCall,     // '__stdcall' to external function
    StdNearCall,    // '__stdcall' to internal function
};

// IA-32 (a.k.a. x86) assembly code generator.
class LLGL_EXPORT JITCompiler : public NonCopyable
{

    public:

        // Instantiates a new JIT compiler for the current hardware architecture (i.e. x86, x64, ARM), or null if the architecture is not supported.
        static std::unique_ptr<JITCompiler> Create();

        // Begins with a new program.
        void Begin();

        // Compiles and flushes the currently build program, or null if no program was build.
        std::unique_ptr<JITProgram> End();

        virtual void BeginArgList(std::size_t numArgs) = 0;
        virtual void PushThisPtr(const void* value) = 0;
        virtual void PushPtr(const void* value) = 0;
        virtual void PushWord(std::uint16_t value) = 0;
        virtual void PushDWord(std::uint32_t value) = 0;
        virtual void PushQWord(std::uint64_t value) = 0;
        virtual void EndArgList() = 0;
        virtual void FuncCall(const void* addr, const JITCall call) = 0;

    protected:

        JITCompiler() = default;

        virtual bool IsLittleEndian() const = 0;

        void Write(const void* data, std::size_t size);
        void WriteByte(std::uint8_t data);
        void WriteWord(std::uint16_t data);
        void WriteDWord(std::uint32_t data);
        void WriteQWord(std::uint64_t data);

        // Returns the assembly code.
        inline const std::vector<std::uint8_t>& GetAssembly() const
        {
            return assembly_;
        }

    private:

        std::vector<std::uint8_t>   assembly_;
        bool                        littleEndian_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
