/*
 * Win32JITProgram.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
