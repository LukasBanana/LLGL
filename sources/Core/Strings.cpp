/*
 * Strings.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

LLGL_EXPORT const char* ToString(const TextureFormat t)
{
    using T = TextureFormat;

    switch (t)
    {
        case T::Unknown:        return "unknown";

        /* --- Color formats --- */
        case T::R8:             return "R8";
        case T::R8Sgn:          return "R8Sgn";

        case T::R16:            return "R16";
        case T::R16Sgn:         return "R16Sgn";
        case T::R16Float:       return "R16Float";

        case T::R32UInt:        return "R32UInt";
        case T::R32SInt:        return "R32SInt";
        case T::R32Float:       return "R32Float";

        case T::RG8:            return "RG8";
        case T::RG8Sgn:         return "RG8Sgn";

        case T::RG16:           return "RG16";
        case T::RG16Sgn:        return "RG16Sgn";
        case T::RG16Float:      return "RG16Float";

        case T::RG32UInt:       return "RG32UInt";
        case T::RG32SInt:       return "RG32SInt";
        case T::RG32Float:      return "RG32Float";

        case T::RGB8:           return "RGB8";
        case T::RGB8Sgn:        return "RGB8Sgn";

        case T::RGB16:          return "RGB16";
        case T::RGB16Sgn:       return "RGB16Sgn";
        case T::RGB16Float:     return "RGB16Float";

        case T::RGB32UInt:      return "RGB32UInt";
        case T::RGB32SInt:      return "RGB32SInt";
        case T::RGB32Float:     return "RGB32Float";

        case T::RGBA8:          return "RGBA8";
        case T::RGBA8Sgn:       return "RGBA8Sgn";

        case T::RGBA16:         return "RGBA16";
        case T::RGBA16Sgn:      return "RGBA16Sgn";
        case T::RGBA16Float:    return "RGBA16Float";

        case T::RGBA32UInt:     return "RGBA32UInt";
        case T::RGBA32SInt:     return "RGBA32SInt";
        case T::RGBA32Float:    return "RGBA32Float";

        /* --- Depth-stencil formats --- */
        case T::D32:            return "D32";
        case T::D24S8:          return "D24S8";

        /* --- Compressed color formats --- */
        case T::RGB_DXT1:       return "RGB DXT1";
        case T::RGBA_DXT1:      return "RGBA DXT1";
        case T::RGBA_DXT3:      return "RGBA DXT3";
        case T::RGBA_DXT5:      return "RGBA DXT5";
    }

    return nullptr;
}


} // /namespace LLGL



// ================================================================================
