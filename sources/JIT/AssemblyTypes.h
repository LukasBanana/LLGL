/*
 * AssemblyTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ASSEMBLY_TYPES_H
#define LLGL_ASSEMBLY_TYPES_H


#include <cstdint>


namespace LLGL
{

namespace JIT
{


// Word, 16 bit
union Word
{
    std::uint16_t i16;
    std::uint8_t  i8[2];
};

// Double word, 32 bit
union DWord
{
    std::uint32_t i32;
    std::uint8_t  i8[4];
};

// Quad word, 64 bit
union QWord
{
    std::uint64_t i64;
    std::uint8_t  i8[8];
};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
