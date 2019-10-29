/*
 * POSIXJITProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
