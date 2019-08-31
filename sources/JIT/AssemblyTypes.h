/*
 * AssemblyTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
    StackPtr,
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
    ArgType             type    : 4;
    std::uint8_t        param   : 4; // Parameter index (0xF if unused)
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

// Returns true if the specified argument type denotes a floating-point type.
inline bool IsFloat(const ArgType t)
{
    return (t >= ArgType::Float);
}

// Converts the specified 16-bit value from little-endian to big-endian and vice-versa.
inline std::uint16_t SwapEndian16(std::uint16_t value)
{
    return (value >> 8) | (value << 8);
}

// Converts the specified 32-bit value from little-endian to big-endian and vice-versa.
inline std::uint32_t SwapEndian32(std::uint32_t value)
{
    return
    (
        ((value >> 0x18) & 0x000000ff) | // Move byte 3 to byte 0
        ((value >> 0x08) & 0x0000ff00) | // Move byte 2 to byte 1
        ((value << 0x08) & 0x00ff0000) | // Move byte 1 to byte 2
        ((value << 0x18) & 0xff000000)   // Move byte 0 to byte 3
    );
}


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
