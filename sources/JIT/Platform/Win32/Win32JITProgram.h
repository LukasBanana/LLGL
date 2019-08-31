/*
 * Win32JITProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_JIT_PROGRAM_H
#define LLGL_WIN32_JIT_PROGRAM_H


#include "../../JITProgram.h"
#include <cstddef>
#include <memory>


namespace LLGL
{


// Wrapper class for platform dependent native code.
class LLGL_EXPORT Win32JITProgram : public JITProgram
{

    public:

        Win32JITProgram(const void* code, std::size_t size);
        ~Win32JITProgram();

    private:

        void*       addr_ = nullptr;
        std::size_t size_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
