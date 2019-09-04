/*
 * Strings.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Strings.h>


namespace LLGL
{


LLGL_EXPORT const char* ToString(const ShaderType t)
{
    using T = ShaderType;

    switch (t)
    {
        case T::Undefined:      return "<undefined>";
        case T::Vertex:         return "vertex";
        case T::TessControl:    return "tessellation control";
        case T::TessEvaluation: return "tessellation evaluation";
        case T::Geometry:       return "geometry";
        case T::Fragment:       return "fragment";
        case T::Compute:        return "compute";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const ErrorType t)
{
    using T = ErrorType;

    switch (t)
    {
        case T::InvalidArgument:    return "invalid argument";
        case T::InvalidState:       return "invalid state";
        case T::UnsupportedFeature: return "unsupported feature";
        case T::UndefinedBehavior:  return "undefined behavior";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const WarningType t)
{
    using T = WarningType;

    switch (t)
    {
        case T::ImproperArgument:   return "improper argument";
        case T::ImproperState:      return "improper state";
        case T::PointlessOperation: return "pointless operation";
        case T::VaryingBehavior:    return "varying behavior";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const ShadingLanguage t)
{
    using T = ShadingLanguage;

    switch (t)
    {
        case T::GLSL:       return "GLSL";
        case T::GLSL_110:   return "GLSL 110";
        case T::GLSL_120:   return "GLSL 120";
        case T::GLSL_130:   return "GLSL 130";
        case T::GLSL_140:   return "GLSL 140";
        case T::GLSL_150:   return "GLSL 150";
        case T::GLSL_330:   return "GLSL 330";
        case T::GLSL_400:   return "GLSL 400";
        case T::GLSL_410:   return "GLSL 410";
        case T::GLSL_420:   return "GLSL 420";
        case T::GLSL_430:   return "GLSL 430";
        case T::GLSL_440:   return "GLSL 440";
        case T::GLSL_450:   return "GLSL 450";
        case T::GLSL_460:   return "GLSL 460";

        case T::ESSL:       return "ESSL";
        case T::ESSL_100:   return "ESSL 100";
        case T::ESSL_300:   return "ESSL 300";
        case T::ESSL_310:   return "ESSL 310";
        case T::ESSL_320:   return "ESSL 320";

        case T::HLSL:       return "HLSL";
        case T::HLSL_2_0:   return "HLSL 2.0";
        case T::HLSL_2_0a:  return "HLSL 2.0a";
        case T::HLSL_2_0b:  return "HLSL 2.0b";
        case T::HLSL_3_0:   return "HLSL 3.0";
        case T::HLSL_4_0:   return "HLSL 4.0";
        case T::HLSL_4_1:   return "HLSL 4.1";
        case T::HLSL_5_0:   return "HLSL 5.0";
        case T::HLSL_5_1:   return "HLSL 5.1";

        case T::Metal:      return "Metal";
        case T::Metal_1_0:  return "Metal 1.0";
        case T::Metal_1_1:  return "Metal 1.1";
        case T::Metal_1_2:  return "Metal 1.2";

        case T::SPIRV:      return "SPIR-V";
        case T::SPIRV_100:  return "SPIR-V 1.00";

        default:            break;
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const Format t)
{
    using T = Format;

    switch (t)
    {
        case T::Undefined:          return "<undefined>";

        /* --- Alpha channel color formats --- */
        case T::A8UNorm:            return "A8UNorm";

        /* --- Red channel color formats --- */
        case T::R8UNorm:            return "R8UNorm";
        case T::R8SNorm:            return "R8SNorm";
        case T::R8UInt:             return "R8UInt";
        case T::R8SInt:             return "R8SInt";

        case T::R16UNorm:           return "R16UNorm";
        case T::R16SNorm:           return "R16SNorm";
        case T::R16UInt:            return "R16UInt";
        case T::R16SInt:            return "R16SInt";
        case T::R16Float:           return "R16Float";

        case T::R32UInt:            return "R32UInt";
        case T::R32SInt:            return "R32SInt";
        case T::R32Float:           return "R32Float";

        case T::R64Float:           return "R64Float";

        /* --- RG color formats --- */
        case T::RG8UNorm:           return "RG8UNorm";
        case T::RG8SNorm:           return "RG8SNorm";
        case T::RG8UInt:            return "RG8UInt";
        case T::RG8SInt:            return "RG8SInt";

        case T::RG16UNorm:          return "RG16UNorm";
        case T::RG16SNorm:          return "RG16SNorm";
        case T::RG16UInt:           return "RG16UInt";
        case T::RG16SInt:           return "RG16SInt";
        case T::RG16Float:          return "RG16Float";

        case T::RG32UInt:           return "RG32UInt";
        case T::RG32SInt:           return "RG32SInt";
        case T::RG32Float:          return "RG32Float";

        case T::RG64Float:          return "RG64Float";

        /* --- RGB color formats --- */
        case T::RGB8UNorm:          return "RGB8UNorm";
        case T::RGB8UNorm_sRGB:     return "RGB8UNorm_sRGB";
        case T::RGB8SNorm:          return "RGB8SNorm";
        case T::RGB8UInt:           return "RGB8UInt";
        case T::RGB8SInt:           return "RGB8SInt";

        case T::RGB16UNorm:         return "RGB16UNorm";
        case T::RGB16SNorm:         return "RGB16SNorm";
        case T::RGB16UInt:          return "RGB16UInt";
        case T::RGB16SInt:          return "RGB16SInt";
        case T::RGB16Float:         return "RGB16Float";

        case T::RGB32UInt:          return "RGB32UInt";
        case T::RGB32SInt:          return "RGB32SInt";
        case T::RGB32Float:         return "RGB32Float";

        case T::RGB64Float:         return "RGB64Float";

        /* --- RGBA color formats --- */
        case T::RGBA8UNorm:         return "RGBA8UNorm";
        case T::RGBA8UNorm_sRGB:    return "RGBA8UNorm_sRGB";
        case T::RGBA8SNorm:         return "RGBA8SNorm";
        case T::RGBA8UInt:          return "RGBA8UInt";
        case T::RGBA8SInt:          return "RGBA8SInt";

        case T::RGBA16UNorm:        return "RGBA16UNorm";
        case T::RGBA16SNorm:        return "RGBA16SNorm";
        case T::RGBA16UInt:         return "RGBA16UInt";
        case T::RGBA16SInt:         return "RGBA16SInt";
        case T::RGBA16Float:        return "RGBA16Float";

        case T::RGBA32UInt:         return "RGBA32UInt";
        case T::RGBA32SInt:         return "RGBA32SInt";
        case T::RGBA32Float:        return "RGBA32Float";

        case T::RGBA64Float:        return "RGBA64Float";

        /* --- BGRA color formats --- */
        case T::BGRA8UNorm:         return "BGRA8UNorm";
        case T::BGRA8UNorm_sRGB:    return "BGRA8UNorm_sRGB";
        case T::BGRA8SNorm:         return "BGRA8SNorm";
        case T::BGRA8UInt:          return "BGRA8UInt";
        case T::BGRA8SInt:          return "BGRA8SInt";

        /* --- Packed formats --- */
        case T::RGB10A2UNorm:       return "RGB10A2UNorm";
        case T::RGB10A2UInt:        return "RGB10A2UInt";
        case T::RG11B10Float:       return "RG11B10Float";
        case T::RGB9E5Float:        return "RGB9E5Float";

        /* --- Depth-stencil formats --- */
        case T::D16UNorm:           return "D16UNorm";
        case T::D32Float:           return "D32Float";
        case T::D24UNormS8UInt:     return "D24UNormS8UInt";
        case T::D32FloatS8X24UInt:  return "D32FloatS8X24UInt";

        /* --- Block compression (BC) formats --- */
        case T::BC1UNorm:           return "BC1UNorm";
        case T::BC1UNorm_sRGB:      return "BC1UNorm_sRGB";
        case T::BC2UNorm:           return "BC2UNorm";
        case T::BC2UNorm_sRGB:      return "BC2UNorm_sRGB";
        case T::BC3UNorm:           return "BC3UNorm";
        case T::BC3UNorm_sRGB:      return "BC3UNorm_sRGB";
        case T::BC4UNorm:           return "BC4UNorm";
        case T::BC4SNorm:           return "BC4SNorm";
        case T::BC5UNorm:           return "BC5UNorm";
        case T::BC5SNorm:           return "BC5SNorm";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const TextureType t)
{
    using T = TextureType;
    switch (t)
    {
        case T::Texture1D:          return "Texture1D";
        case T::Texture2D:          return "Texture2D";
        case T::Texture3D:          return "Texture3D";
        case T::TextureCube:        return "TextureCube";
        case T::Texture1DArray:     return "Texture1DArray";
        case T::Texture2DArray:     return "Texture2DArray";
        case T::TextureCubeArray:   return "TextureCubeArray";
        case T::Texture2DMS:        return "Texture2DMS";
        case T::Texture2DMSArray:   return "Texture2DMSArray";
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
