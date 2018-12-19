/*
 * JITProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_JIT_PROGRAM_H
#define LLGL_JIT_PROGRAM_H


#include <LLGL/NonCopyable.h>
#include <cstddef>
#include <memory>


namespace LLGL
{


// Wrapper class for platform dependent native code.
class LLGL_EXPORT JITProgram : public NonCopyable
{

    public:

        // Creates a new JIT program with the specified code.
        static std::unique_ptr<JITProgram> Create(const void* code, std::size_t size);

        // Runs the JIT program natively.
        inline void Execute()
        {
            func_();
        }

    protected:

        JITProgram() = default;

        // Sets the address for the function pointer that can be executed.
        inline void SetFuncPtr(void* addr)
        {
            func_ = reinterpret_cast<FuncPtr>(addr);
        }

    private:

        typedef void (*FuncPtr)();
        FuncPtr func_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
