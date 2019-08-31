/*
 * JITProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

        // Function pointer type of the main entry point.
        typedef void (*EntryPointPtr)(...);

    public:

        // Creates a new JIT program with the specified code.
        static std::unique_ptr<JITProgram> Create(const void* code, std::size_t size);

        // Returns the main entry point of the native JIT program.
        inline EntryPointPtr GetEntryPoint() const
        {
            return entryPoint_;
        }

    protected:

        JITProgram() = default;

        // Sets the address for the function pointer that can be executed.
        inline void SetEntryPoint(void* addr)
        {
            entryPoint_ = reinterpret_cast<EntryPointPtr>(addr);
        }

    private:

        EntryPointPtr entryPoint_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
