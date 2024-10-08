/*
 * TypeNames.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/TypeNames.h>
#include "MacroUtils.h"


namespace LLGL
{


LLGL_EXPORT const char* ToString(const ShaderType val)
{
    using T = ShaderType;

    switch (val)
    {
        case T::Undefined:      return "undefined";
        case T::Vertex:         return "vertex";
        case T::TessControl:    return "tessellation control";
        case T::TessEvaluation: return "tessellation evaluation";
        case T::Geometry:       return "geometry";
        case T::Fragment:       return "fragment";
        case T::Compute:        return "compute";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const ErrorType val)
{
    using T = ErrorType;

    switch (val)
    {
        case T::InvalidArgument:    return "invalid argument";
        case T::InvalidState:       return "invalid state";
        case T::UnsupportedFeature: return "unsupported feature";
        case T::UndefinedBehavior:  return "undefined behavior";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const WarningType val)
{
    using T = WarningType;

    switch (val)
    {
        case T::ImproperArgument:   return "improper argument";
        case T::ImproperState:      return "improper state";
        case T::PointlessOperation: return "pointless operation";
        case T::VaryingBehavior:    return "varying behavior";
    }

    return nullptr;
}

//TODO: rename ToString() functions that return a more human readable friendly name to ToName() or ToLabel.
LLGL_EXPORT const char* ToString(const ShadingLanguage val)
{
    using T = ShadingLanguage;

    switch (val)
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
        case T::HLSL_6_0:   return "HLSL 6.0";
        case T::HLSL_6_1:   return "HLSL 6.1";
        case T::HLSL_6_2:   return "HLSL 6.2";
        case T::HLSL_6_3:   return "HLSL 6.3";
        case T::HLSL_6_4:   return "HLSL 6.4";
        case T::HLSL_6_5:   return "HLSL 6.5";
        case T::HLSL_6_6:   return "HLSL 6.6";
        case T::HLSL_6_7:   return "HLSL 6.7";
        case T::HLSL_6_8:   return "HLSL 6.8";

        case T::Metal:      return "Metal";
        case T::Metal_1_0:  return "Metal 1.0";
        case T::Metal_1_1:  return "Metal 1.1";
        case T::Metal_1_2:  return "Metal 1.2";
        case T::Metal_2_0:  return "Metal 2.0";
        case T::Metal_2_1:  return "Metal 2.1";
        case T::Metal_2_2:  return "Metal 2.2";
        case T::Metal_2_3:  return "Metal 2.3";
        case T::Metal_2_4:  return "Metal 2.4";
        case T::Metal_3_0:  return "Metal 3.0";

        case T::SPIRV:      return "SPIR-V";
        case T::SPIRV_100:  return "SPIR-V 1.00";

        default:            break;
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const Format val)
{
    switch (val)
    {
        LLGL_CASE_TO_STR_TYPED( Format, Undefined         );

        /* --- Alpha channel color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, A8UNorm           );

        /* --- Red channel color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, R8UNorm           );
        LLGL_CASE_TO_STR_TYPED( Format, R8SNorm           );
        LLGL_CASE_TO_STR_TYPED( Format, R8UInt            );
        LLGL_CASE_TO_STR_TYPED( Format, R8SInt            );

        LLGL_CASE_TO_STR_TYPED( Format, R16UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, R16SNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, R16UInt           );
        LLGL_CASE_TO_STR_TYPED( Format, R16SInt           );
        LLGL_CASE_TO_STR_TYPED( Format, R16Float          );

        LLGL_CASE_TO_STR_TYPED( Format, R32UInt           );
        LLGL_CASE_TO_STR_TYPED( Format, R32SInt           );
        LLGL_CASE_TO_STR_TYPED( Format, R32Float          );

        LLGL_CASE_TO_STR_TYPED( Format, R64Float          );

        /* --- RG color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, RG8UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, RG8SNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, RG8UInt           );
        LLGL_CASE_TO_STR_TYPED( Format, RG8SInt           );

        LLGL_CASE_TO_STR_TYPED( Format, RG16UNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, RG16SNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, RG16UInt          );
        LLGL_CASE_TO_STR_TYPED( Format, RG16SInt          );
        LLGL_CASE_TO_STR_TYPED( Format, RG16Float         );

        LLGL_CASE_TO_STR_TYPED( Format, RG32UInt          );
        LLGL_CASE_TO_STR_TYPED( Format, RG32SInt          );
        LLGL_CASE_TO_STR_TYPED( Format, RG32Float         );

        LLGL_CASE_TO_STR_TYPED( Format, RG64Float         );

        /* --- RGB color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, RGB8UNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB8UNorm_sRGB    );
        LLGL_CASE_TO_STR_TYPED( Format, RGB8SNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB8UInt          );
        LLGL_CASE_TO_STR_TYPED( Format, RGB8SInt          );

        LLGL_CASE_TO_STR_TYPED( Format, RGB16UNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, RGB16SNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, RGB16UInt         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB16SInt         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB16Float        );

        LLGL_CASE_TO_STR_TYPED( Format, RGB32UInt         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB32SInt         );
        LLGL_CASE_TO_STR_TYPED( Format, RGB32Float        );

        LLGL_CASE_TO_STR_TYPED( Format, RGB64Float        );

        /* --- RGBA color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, RGBA8UNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA8UNorm_sRGB   );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA8SNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA8UInt         );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA8SInt         );

        LLGL_CASE_TO_STR_TYPED( Format, RGBA16UNorm       );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA16SNorm       );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA16UInt        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA16SInt        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA16Float       );

        LLGL_CASE_TO_STR_TYPED( Format, RGBA32UInt        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA32SInt        );
        LLGL_CASE_TO_STR_TYPED( Format, RGBA32Float       );

        LLGL_CASE_TO_STR_TYPED( Format, RGBA64Float       );

        /* --- BGRA color formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, BGRA8UNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, BGRA8UNorm_sRGB   );
        LLGL_CASE_TO_STR_TYPED( Format, BGRA8SNorm        );
        LLGL_CASE_TO_STR_TYPED( Format, BGRA8UInt         );
        LLGL_CASE_TO_STR_TYPED( Format, BGRA8SInt         );

        /* --- Packed formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, RGB10A2UNorm      );
        LLGL_CASE_TO_STR_TYPED( Format, RGB10A2UInt       );
        LLGL_CASE_TO_STR_TYPED( Format, RG11B10Float      );
        LLGL_CASE_TO_STR_TYPED( Format, RGB9E5Float       );

        /* --- Depth-stencil formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, D16UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, D32Float          );
        LLGL_CASE_TO_STR_TYPED( Format, D24UNormS8UInt    );
        LLGL_CASE_TO_STR_TYPED( Format, D32FloatS8X24UInt );

        /* --- Block compression (BC) formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, BC1UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC1UNorm_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, BC2UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC2UNorm_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, BC3UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC3UNorm_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, BC4UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC4SNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC5UNorm          );
        LLGL_CASE_TO_STR_TYPED( Format, BC5SNorm          );

        /* --- Advanced scalable texture compression (ASTC) formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, ASTC4x4           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC4x4_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC5x4           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC5x4_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC5x5           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC5x5_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC6x5           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC6x5_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC6x6           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC6x6_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x5           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x5_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x6           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x6_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x8           );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC8x8_sRGB      );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x5          );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x5_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x6          );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x6_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x8          );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x8_sRGB     );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x10         );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC10x10_sRGB    );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC12x10         );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC12x10_sRGB    );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC12x12         );
        LLGL_CASE_TO_STR_TYPED( Format, ASTC12x12_sRGB    );

        /* --- Ericsson texture compression (ETC) formats --- */
        LLGL_CASE_TO_STR_TYPED( Format, ETC1UNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, ETC2UNorm         );
        LLGL_CASE_TO_STR_TYPED( Format, ETC2UNorm_sRGB    );
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const ImageFormat val)
{
    using T = ImageFormat;

    switch (val)
    {
        /* Color formats */
        case T::Alpha:          return "Alpha";
        case T::R:              return "R";
        case T::RG:             return "RG";
        case T::RGB:            return "RGB";
        case T::BGR:            return "BGR";
        case T::RGBA:           return "RGBA";
        case T::BGRA:           return "BGRA";
        case T::ARGB:           return "ARGB";
        case T::ABGR:           return "ABGR";

        /* Depth-stencil formats */
        case T::Depth:          return "Depth";
        case T::DepthStencil:   return "DepthStencil";
        case T::Stencil:        return "Stencil";

        /* Compressed formats */
        case T::Compressed:     return "Compressed";

        case T::BC1:            return "BC1 (DEPRECATED)";
        case T::BC2:            return "BC2 (DEPRECATED)";
        case T::BC3:            return "BC3 (DEPRECATED)";
        case T::BC4:            return "BC4 (DEPRECATED)";
        case T::BC5:            return "BC5 (DEPRECATED)";
    }

    return nullptr;
}

LLGL_EXPORT const char* ToString(const TextureType val)
{
    switch (val)
    {
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture1D        );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture2D        );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture3D        );
        LLGL_CASE_TO_STR_TYPED( TextureType, TextureCube      );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture1DArray   );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture2DArray   );
        LLGL_CASE_TO_STR_TYPED( TextureType, TextureCubeArray );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture2DMS      );
        LLGL_CASE_TO_STR_TYPED( TextureType, Texture2DMSArray );
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const BlendOp val)
{
    switch (val)
    {
        LLGL_CASE_TO_STR_TYPED( BlendOp, Zero             );
        LLGL_CASE_TO_STR_TYPED( BlendOp, One              );
        LLGL_CASE_TO_STR_TYPED( BlendOp, SrcColor         );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvSrcColor      );
        LLGL_CASE_TO_STR_TYPED( BlendOp, SrcAlpha         );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvSrcAlpha      );
        LLGL_CASE_TO_STR_TYPED( BlendOp, DstColor         );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvDstColor      );
        LLGL_CASE_TO_STR_TYPED( BlendOp, DstAlpha         );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvDstAlpha      );
        LLGL_CASE_TO_STR_TYPED( BlendOp, SrcAlphaSaturate );
        LLGL_CASE_TO_STR_TYPED( BlendOp, BlendFactor      );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvBlendFactor   );
        LLGL_CASE_TO_STR_TYPED( BlendOp, Src1Color        );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvSrc1Color     );
        LLGL_CASE_TO_STR_TYPED( BlendOp, Src1Alpha        );
        LLGL_CASE_TO_STR_TYPED( BlendOp, InvSrc1Alpha     );
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const ResourceType val)
{
    using T = ResourceType;
    switch (val)
    {
        case T::Undefined:  return "undefined";
        case T::Buffer:     return "buffer";
        case T::Texture:    return "texture";
        case T::Sampler:    return "sampler";
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const SystemValue val)
{
    switch (val)
    {
        LLGL_CASE_TO_STR_TYPED( SystemValue, Undefined         );
        LLGL_CASE_TO_STR_TYPED( SystemValue, ClipDistance      );
        LLGL_CASE_TO_STR_TYPED( SystemValue, Color             );
        LLGL_CASE_TO_STR_TYPED( SystemValue, CullDistance      );
        LLGL_CASE_TO_STR_TYPED( SystemValue, Depth             );
        LLGL_CASE_TO_STR_TYPED( SystemValue, DepthGreater      );
        LLGL_CASE_TO_STR_TYPED( SystemValue, DepthLess         );
        LLGL_CASE_TO_STR_TYPED( SystemValue, FrontFacing       );
        LLGL_CASE_TO_STR_TYPED( SystemValue, InstanceID        );
        LLGL_CASE_TO_STR_TYPED( SystemValue, Position          );
        LLGL_CASE_TO_STR_TYPED( SystemValue, PrimitiveID       );
        LLGL_CASE_TO_STR_TYPED( SystemValue, RenderTargetIndex );
        LLGL_CASE_TO_STR_TYPED( SystemValue, SampleMask        );
        LLGL_CASE_TO_STR_TYPED( SystemValue, SampleID          );
        LLGL_CASE_TO_STR_TYPED( SystemValue, Stencil           );
        LLGL_CASE_TO_STR_TYPED( SystemValue, VertexID          );
        LLGL_CASE_TO_STR_TYPED( SystemValue, ViewportIndex     );
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const QueryType val)
{
    switch (val)
    {
        LLGL_CASE_TO_STR_TYPED( QueryType, SamplesPassed                );
        LLGL_CASE_TO_STR_TYPED( QueryType, AnySamplesPassed             );
        LLGL_CASE_TO_STR_TYPED( QueryType, AnySamplesPassedConservative );
        LLGL_CASE_TO_STR_TYPED( QueryType, TimeElapsed                  );
        LLGL_CASE_TO_STR_TYPED( QueryType, StreamOutPrimitivesWritten   );
        LLGL_CASE_TO_STR_TYPED( QueryType, StreamOutOverflow            );
        LLGL_CASE_TO_STR_TYPED( QueryType, PipelineStatistics           );
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
