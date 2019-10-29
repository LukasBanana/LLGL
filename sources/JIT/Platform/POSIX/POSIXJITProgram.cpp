/*
 * POSIXJITProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "POSIXJITProgram.h"
#include "../../../Core/Helper.h"
#include <cstdlib>
#include <stdexcept>
#include <unistd.h> // sysconf
#include <sys/mman.h> // mmap


namespace LLGL
{


std::unique_ptr<JITProgram> JITProgram::Create(const void* code, std::size_t size)
{
    return MakeUnique<POSIXJITProgram>(code, size);
}

POSIXJITProgram::POSIXJITProgram(const void* code, std::size_t size) :
    size_ { GetAlignedSize(size, std::size_t(sysconf(_SC_PAGE_SIZE))) }
{
    /* Map executable memory space */
    addr_ = ::mmap(
        nullptr,
        size_,
        (PROT_READ | PROT_WRITE | PROT_EXEC),
        (MAP_PRIVATE | MAP_ANONYMOUS),
        -1, // must be -1 if MAP_ANONYMOUS is used
        0
    );
    
    if (addr_ == MAP_FAILED)
        throw std::runtime_error("failed to map executable virtual memory with read/write protection mode");
    
    /* Copy code into executable memory space */
    ::memcpy(addr_, code, size);

    /* Set function pointer to executable memory address */
    SetEntryPoint(addr_);
}

POSIXJITProgram::POSIXJITProgram()
{
    munmap(addr_, size_);
}


} // /namespace LLGL



// ================================================================================
