/*
 * TextureFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <details/TextureFormat.h>


namespace LLGL
{


LLGL_EXPORT LLGL_API std::string ToString(TextureFormat value)
{
    switch (value)
    {
        case TextureFormat::R8ub:
            return "R 8-UByte";
        case TextureFormat::RG8ub:
            return "RG 8-UByte";
        case TextureFormat::RGB8ub:
            return "RGB 8-UByte";
        case TextureFormat::RGBA8ub:
            return "RGBA 8-UByte";
        case TextureFormat::R32f:
            return "R 32-Float";
        case TextureFormat::RG32f:
            return "RG 32-Float";
        case TextureFormat::RGB32f:
            return "RGB 32-Float";
        case TextureFormat::RGBA32f:
            return "RGBA 32-Float";
    }
    return "";
}


} // /namespace LLGL



// ================================================================================
