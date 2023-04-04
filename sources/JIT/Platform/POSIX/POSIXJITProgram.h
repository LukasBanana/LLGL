/*
 * POSIXJITProgram.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_POSIX_JIT_PROGRAM_H
#define LLGL_POSIX_JIT_PROGRAM_H


#include "../../JITProgram.h"
#include <cstddef>


namespace LLGL
{


class LLGL_EXPORT POSIXJITProgram : public JITProgram
{

    public:

        POSIXJITProgram(const void* code, std::size_t size);
        POSIXJITProgram();

    private:

        void*       addr_ = nullptr;
        std::size_t size_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
