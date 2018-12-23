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


// Argument type enumeration.
enum class ArgType
{
    Byte,
    Word,
    DWord,
    QWord,
    Ptr,
    Float,
    Double,
};

// Word (16 bit).
union Word
{
    std::uint16_t i16;
    std::uint8_t  i8[2];
};

// Double word (32 bit).
union DWord
{
    std::uint32_t i32;
    std::uint8_t  i8[4];
};

// Quad word (64 bit).
union QWord
{
    std::uint64_t i64;
    std::uint8_t  i8[8];
};

// Function argument with type and value.
struct Arg
{
    ArgType             type;
    union
    {
        std::uint8_t    i8;
        std::uint16_t   i16;
        std::uint32_t   i32;
        std::uint64_t   i64;
        const void*     ptr;
        float           f32;
        double          f64;
    }
    value;
};

inline bool IsFloat(const ArgType t)
{
    return (t >= ArgType::Float);
}


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
