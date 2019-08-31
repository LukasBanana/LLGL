/*
 * JITCompiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_JIT_COMPILER_H
#define LLGL_JIT_COMPILER_H


#include "JITProgram.h"
#include "AssemblyTypes.h"
#include "../Core/Helper.h"
#include <LLGL/NonCopyable.h>
#include <iostream>
#include <vector>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <initializer_list>


namespace LLGL
{


// Enumeration for calling conventions.
enum class JITCallConv
{
    CDecl,      // '__cdecl' to internal function
    StdCall,    // '__stdcall' to internal function
    ThisCall,   // '__thiscall' to internal function
};

// Structure to pass a variadic argument via 'JITCompiler::Call' template function.
struct JITVarArg
{
    std::uint8_t index;
};

// Structure to pass a stack pointer via to the 'JITCompiler::Call' template function.
struct JITStackPtr
{
    std::uint8_t index;
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

        // Dumps the current assembly code to the output stream.
        void DumpAssembly(std::ostream& stream, bool textForm = false, std::size_t bytesPerLine = 8) const;

        // Flushes the currently build program, or null if no program was build.
        std::unique_ptr<JITProgram> FlushProgram();

    public:

        // Stores the parameter list of the secified types for the program entry points (must be called before 'Begin').
        void EntryPointVarArgs(const std::initializer_list<JIT::ArgType>& varArgTypes);

        // Stores teh stack allocation for the specified amount of bytes, and returns the ID of this allocation (must be called before 'Begin').
        std::uint8_t StackAlloc(std::uint32_t size);

        // Begins with generating assembly code
        virtual void Begin() = 0;
        virtual void End() = 0;

        // Pushes the entry point parameter, specified by the zero-based index 'idx', to the argument list.
        void PushVarArg(std::uint8_t idx);

        // Pushes the ID of the specified stack allocation, specified by the zero-based index 'idx', to the argument list.
        void PushStackPtr(std::uint8_t idx);

        // Pushes the specified value to the argument list for the next function call.
        void PushPtr(const void* value);
        void PushByte(std::uint8_t value);
        void PushWord(std::uint16_t value);
        void PushDWord(std::uint32_t value);
        void PushQWord(std::uint64_t value);
        void PushFloat(float value);
        void PushDouble(double value);
        void PushSizeT(std::uint64_t value);
        void PushSSizeT(std::int64_t value);

        /*
        Encodes a function call.
        \param[in] addr Specifies the pointer to the native function that is to be called.
        \param[in] conv Specifies the calling convention. This is only used for x86 assembly and ignored otherwise.
        \param[in] farCall Specifies whether an intersegment function (far call) is to be used.
        */
        void FuncCall(
            const void* addr,
            JITCallConv conv    = JITCallConv::CDecl,
            bool        farCall = false
        );

        /*
        Encodes a function call with the specified variadic arguments.
        \param[in] func Specifies the function object. This must be a global non-overloaded function.
        \param[in] args Specifies the argument list. Only pointers, integrals and floating-point types are allowed (no references).
        */
        template <typename Func, typename... Args>
        void Call(Func&& func, Args&&... args)
        {
            PushArgs(std::forward<Args>(args)...);
            FuncCall(reinterpret_cast<const void*>(func));
        }

        /*
        Encodes a member function call with the specified variadic arguments.
        \param[in] func Specifies the function object. This must be a global non-overloaded function.
        \param[in] inst Specifies the class instance on which the member function is to be called.
        \param[in] args Specifies the argument list. Only pointers, integrals and floating-point types are allowed (no references).
        */
        template <typename Func, typename Inst, typename... Args>
        void CallMember(Func&& func, Inst&& inst, Args&&... args)
        {
            PushVariant(std::forward<Inst>(inst));//TODO: PushThisPtr(inst);
            PushArgs(std::forward<Args>(args)...);
            FuncCall(GetMemberFuncPtr(func));
        }

    protected:

        JITCompiler() = default;

        virtual bool IsLittleEndian() const = 0;
        virtual void WriteFuncCall(const void* addr, JITCallConv conv, bool farCall) = 0;

    protected:

        void Write(const void* data, std::size_t size);
        void WriteByte(std::uint8_t data);
        void WriteWord(std::uint16_t data);
        void WriteDWord(std::uint32_t data);
        void WriteQWord(std::uint64_t data);
        void WritePtr(const void* data);

    protected:

        // Returns the assembly code (constant).
        inline const std::vector<std::uint8_t>& GetAssembly() const
        {
            return assembly_;
        }

        // Returns the assembly code.
        inline std::vector<std::uint8_t>& GetAssembly()
        {
            return assembly_;
        }

        // Returns the list of function arguments.
        inline const std::vector<JIT::Arg>& GetArgs() const
        {
            return args_;
        }

        // Returns the list of entry point variadic arguments.
        inline const std::vector<JIT::ArgType>& GetEntryVarArgs() const
        {
            return entryVarArgs_;
        }

        // Returns the list of stack allocations.
        inline const std::vector<std::uint32_t>& GetStackAllocs() const
        {
            return stackAllocs_;
        }

    private:

        template <typename T>
        inline void PushVariant(T arg);

        template <typename T>
        inline void PushVariant(T* arg);

        template <typename T>
        inline void PushVariant(const T* arg);

        template <typename Arg0>
        inline void PushArgsPrimary(Arg0&& arg0);

        template <typename Arg0, typename... ArgsN>
        inline void PushArgsPrimary(Arg0&& arg0, ArgsN&&... argsN);

        template <typename... Args>
        inline void PushArgs(Args&&... args);

    private:

        bool                        littleEndian_   = false;
        std::vector<std::uint8_t>   assembly_;

        std::vector<JIT::Arg>       args_;
        std::vector<JIT::ArgType>   entryVarArgs_;
        std::vector<std::uint32_t>  stackAllocs_;

};


/* ----- Template implementations ----- */

template <typename T>
inline void JITCompiler::PushVariant(T arg)
{
    switch (sizeof(T))
    {
        case 1:
            PushByte(static_cast<std::uint8_t>(arg));
            break;
        case 2:
            PushWord(static_cast<std::uint16_t>(arg));
            break;
        case 4:
            PushDWord(static_cast<std::uint32_t>(arg));
            break;
        case 8:
            PushQWord(static_cast<std::uint64_t>(arg));
            break;
    }
}

template <typename T>
inline void JITCompiler::PushVariant(T* arg)
{
    PushPtr(reinterpret_cast<void*>(arg));
}

template <typename T>
inline void JITCompiler::PushVariant(const T* arg)
{
    PushPtr(reinterpret_cast<const void*>(arg));
}

// Template specialization
template <>
inline void JITCompiler::PushVariant<float>(float arg)
{
    PushFloat(arg);
}

// Template specialization
template <>
inline void JITCompiler::PushVariant<double>(double arg)
{
    PushDouble(arg);
}

// Template specialization
template <>
inline void JITCompiler::PushVariant<JITVarArg>(JITVarArg arg)
{
    PushVarArg(arg.index);
}

// Template specialization
template <>
inline void JITCompiler::PushVariant<JITStackPtr>(JITStackPtr arg)
{
    PushStackPtr(arg.index);
}

template <typename Arg0>
inline void JITCompiler::PushArgsPrimary(Arg0&& arg0)
{
    PushVariant(arg0);
}

template <typename Arg0, typename... ArgsN>
inline void JITCompiler::PushArgsPrimary(Arg0&& arg0, ArgsN&&... argsN)
{
    PushVariant(arg0);
    PushArgsPrimary(std::forward<ArgsN>(argsN)...);
}

template <typename... Args>
inline void JITCompiler::PushArgs(Args&&... args)
{
    PushArgsPrimary(std::forward<Args>(args)...);
}

template <>
inline void JITCompiler::PushArgs<>()
{
    // do nothing
}

#ifdef LLGL_DEBUG
LLGL_EXPORT void TestJIT1();
#endif // /LLGL_DEBUG


} // /namespace LLGL


#endif



// ================================================================================
