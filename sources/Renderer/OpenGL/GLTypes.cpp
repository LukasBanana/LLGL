/*
 * GLTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTypes.h"
#include <LLGL/StaticLimits.h>
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace GLTypes
{


/* ----- Internal functions ----- */

[[noreturn]]
static void MapFailed(const char* typeName)
{
    throw std::invalid_argument("failed to map <LLGL::" + std::string(typeName) + "> to OpenGL parameter");
}

[[noreturn]]
static void UnmapFailed(const char* typeName)
{
    throw std::invalid_argument("failed to unmap <LLGL::" + std::string(typeName) + "> from OpenGL parameter");
}


/* ----- MapOrZero functions ----- */

GLenum MapOrZero(const Format format)
{
    switch (format)
    {
        case Format::Undefined:         return 0;

        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:           return GL_R8; // texture swizzle

        /* --- Red channel color formats --- */
        case Format::R8UNorm:           return GL_R8;
        case Format::R8SNorm:           return GL_R8_SNORM;
        case Format::R8UInt:            return GL_R8UI;
        case Format::R8SInt:            return GL_R8I;

        #ifdef LLGL_OPENGL
        case Format::R16UNorm:          return GL_R16;
        case Format::R16SNorm:          return GL_R16_SNORM;
        #endif
        case Format::R16UInt:           return GL_R16UI;
        case Format::R16SInt:           return GL_R16I;
        case Format::R16Float:          return GL_R16F;

        case Format::R32UInt:           return GL_R32I;
        case Format::R32SInt:           return GL_R32UI;
        case Format::R32Float:          return GL_R32F;

        case Format::R64Float:          return 0;

        /* --- RG color formats --- */
        case Format::RG8UNorm:          return GL_RG8;
        case Format::RG8SNorm:          return GL_RG8_SNORM;
        case Format::RG8UInt:           return GL_RG8UI;
        case Format::RG8SInt:           return GL_RG8I;

        #ifdef LLGL_OPENGL
        case Format::RG16UNorm:         return GL_RG16;
        case Format::RG16SNorm:         return GL_RG16_SNORM;
        #endif
        case Format::RG16UInt:          return GL_RG16UI;
        case Format::RG16SInt:          return GL_RG16I;
        case Format::RG16Float:         return GL_RG16F;

        case Format::RG32UInt:          return GL_RG32UI;
        case Format::RG32SInt:          return GL_RG32I;
        case Format::RG32Float:         return GL_RG32F;

        case Format::RG64Float:         return 0;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:         return GL_RGB8;
        case Format::RGB8UNorm_sRGB:    return GL_SRGB8;
        case Format::RGB8SNorm:         return GL_RGB8_SNORM;
        case Format::RGB8UInt:          return GL_RGB8UI;
        case Format::RGB8SInt:          return GL_RGB8I;

        #ifdef LLGL_OPENGL
        case Format::RGB16UNorm:        return GL_RGB16;
        case Format::RGB16SNorm:        return GL_RGB16_SNORM;
        #endif
        case Format::RGB16UInt:         return GL_RGB16UI;
        case Format::RGB16SInt:         return GL_RGB16I;
        case Format::RGB16Float:        return GL_RGB16F;

        case Format::RGB32UInt:         return GL_RGB32UI;
        case Format::RGB32SInt:         return GL_RGB32I;
        case Format::RGB32Float:        return GL_RGB32F;

        case Format::RGB64Float:        return 0;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:        return GL_RGBA8;
        case Format::RGBA8UNorm_sRGB:   return GL_SRGB8_ALPHA8;
        case Format::RGBA8SNorm:        return GL_RGBA8_SNORM;
        case Format::RGBA8UInt:         return GL_RGBA8UI;
        case Format::RGBA8SInt:         return GL_RGBA8I;

        #ifdef LLGL_OPENGL
        case Format::RGBA16UNorm:       return GL_RGBA16;
        case Format::RGBA16SNorm:       return GL_RGBA16_SNORM;
        #endif
        case Format::RGBA16UInt:        return GL_RGBA16UI;
        case Format::RGBA16SInt:        return GL_RGBA16I;
        case Format::RGBA16Float:       return GL_RGBA16F;

        case Format::RGBA32UInt:        return GL_RGBA32UI;
        case Format::RGBA32SInt:        return GL_RGBA32I;
        case Format::RGBA32Float:       return GL_RGBA32F;

        case Format::RGBA64Float:       return 0;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:        return GL_RGBA8;        // texture swizzle
        case Format::BGRA8UNorm_sRGB:   return GL_SRGB8_ALPHA8; // texture swizzle
        case Format::BGRA8SNorm:        return GL_RGBA8_SNORM;  // texture swizzle
        case Format::BGRA8UInt:         return GL_RGBA8UI;      // texture swizzle
        case Format::BGRA8SInt:         return GL_RGBA8I;       // texture swizzle

        #ifdef LLGL_OPENGL
        /* --- Packed formats --- */
        case Format::RGB10A2UNorm:      return GL_RGB10_A2;
        case Format::RGB10A2UInt:       return GL_RGB10_A2UI;
        case Format::RG11B10Float:      return GL_R11F_G11F_B10F;
        case Format::RGB9E5Float:       return GL_RGB9_E5;
        #endif

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return GL_DEPTH_COMPONENT16;
        #ifdef LLGL_OPENGL
        case Format::D32Float:          return GL_DEPTH_COMPONENT32;//GL_DEPTH_COMPONENT;
        #else
        case Format::D32Float:          return GL_DEPTH_COMPONENT32F;
        #endif
        case Format::D24UNormS8UInt:    return GL_DEPTH24_STENCIL8;//GL_DEPTH_STENCIL;
        case Format::D32FloatS8X24UInt: return GL_DEPTH32F_STENCIL8;

        /* --- Block compression (BC) formats --- */
        #ifdef GL_EXT_texture_compression_s3tc
        case Format::BC1UNorm:          return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case Format::BC2UNorm:          return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case Format::BC3UNorm:          return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        #endif // /GL_EXT_texture_compression_s3tc

        #ifdef GL_EXT_texture_sRGB
        case Format::BC1UNorm_sRGB:     return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
        case Format::BC2UNorm_sRGB:     return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
        case Format::BC3UNorm_sRGB:     return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        #endif // /GL_EXT_texture_sRGB

        #ifdef GL_EXT_texture_compression_rgtc
        case Format::BC4UNorm:          return GL_COMPRESSED_RED_RGTC1_EXT;
        case Format::BC4SNorm:          return GL_COMPRESSED_SIGNED_RED_RGTC1_EXT;
        case Format::BC5UNorm:          return GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
        case Format::BC5SNorm:          return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT;
        #endif // /GL_EXT_texture_compression_rgtc

        default:                        return 0;
    }
}

/* ----- Map functions ----- */

GLenum Map(const CPUAccess cpuAccess)
{
    #ifdef LLGL_OPENGL
    switch (cpuAccess)
    {
        case CPUAccess::ReadOnly:       return GL_READ_ONLY;
        case CPUAccess::WriteOnly:      return GL_WRITE_ONLY;
        case CPUAccess::WriteDiscard:   return GL_WRITE_ONLY; // discard is optional
        case CPUAccess::ReadWrite:      return GL_READ_WRITE;
    }
    #endif
    MapFailed("CPUAccess");
}

GLenum Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Undefined:   break;
        case DataType::Int8:        return GL_BYTE;
        case DataType::UInt8:       return GL_UNSIGNED_BYTE;
        case DataType::Int16:       return GL_SHORT;
        case DataType::UInt16:      return GL_UNSIGNED_SHORT;
        case DataType::Int32:       return GL_INT;
        case DataType::UInt32:      return GL_UNSIGNED_INT;
        case DataType::Float16:     return GL_HALF_FLOAT;
        case DataType::Float32:     return GL_FLOAT;
        #ifdef LLGL_OPENGL
        case DataType::Float64:     return GL_DOUBLE;
        #else
        case DataType::Float64:     break;
        #endif
    }
    MapFailed("DataType");
}

GLenum Map(const TextureType textureType)
{
    #ifdef LLGL_OPENGL
    switch (textureType)
    {
        case TextureType::Texture1D:        return GL_TEXTURE_1D;
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP;
        case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
        case TextureType::Texture2DMS:      return GL_TEXTURE_2D_MULTISAMPLE;
        case TextureType::Texture2DMSArray: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
    }
    #else
    switch (textureType)
    {
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        default:                            break;
    }
    #endif
    MapFailed("TextureType");
}

GLenum Map(const TextureSwizzle textureSwizzle)
{
    switch (textureSwizzle)
    {
        case TextureSwizzle::Zero:  return GL_ZERO;
        case TextureSwizzle::One:   return GL_ONE;
        case TextureSwizzle::Red:   return GL_RED;
        case TextureSwizzle::Green: return GL_GREEN;
        case TextureSwizzle::Blue:  return GL_BLUE;
        case TextureSwizzle::Alpha: return GL_ALPHA;
    }
    MapFailed("TextureSwizzle");
}

GLenum Map(const Format textureFormat)
{
    if (auto result = MapOrZero(textureFormat))
        return result;
    MapFailed("Format");
}

static GLenum MapImageFormat(const ImageFormat imageFormat)
{
    switch (imageFormat)
    {
        case ImageFormat::R:                return GL_RED;
        case ImageFormat::RG:               return GL_RG;
        case ImageFormat::RGB:              return GL_RGB;
        #ifdef LLGL_OPENGL
        case ImageFormat::BGR:              return GL_BGR;
        #endif
        case ImageFormat::RGBA:             return GL_RGBA;
        #ifdef LLGL_OPENGL
        case ImageFormat::BGRA:             return GL_BGRA;
        #endif
        case ImageFormat::Depth:            return GL_DEPTH_COMPONENT;
        case ImageFormat::DepthStencil:     return GL_DEPTH_STENCIL;
        #ifdef LLGL_OPENGL
        case ImageFormat::BC1:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC2:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC3:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC4:              return GL_COMPRESSED_RED;
        case ImageFormat::BC5:              return GL_COMPRESSED_RG;
        #endif
        default:                            break;
    }
    MapFailed("ImageFormat");
}

static GLenum MapIntegerImageFormat(const ImageFormat imageFormat)
{
    switch (imageFormat)
    {
        case ImageFormat::R:                return GL_RED_INTEGER;
        case ImageFormat::RG:               return GL_RG_INTEGER;
        case ImageFormat::RGB:              return GL_RGB_INTEGER;
        #ifdef LLGL_OPENGL
        case ImageFormat::BGR:              return GL_BGR_INTEGER;
        #endif
        case ImageFormat::RGBA:             return GL_RGBA_INTEGER;
        case ImageFormat::BGRA:             return GL_BGRA_INTEGER;
        case ImageFormat::Depth:            return GL_DEPTH_COMPONENT;
        case ImageFormat::DepthStencil:     return GL_DEPTH_STENCIL;
        #ifdef LLGL_OPENGL
        case ImageFormat::BC1:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC2:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC3:              return GL_COMPRESSED_RGBA;
        case ImageFormat::BC4:              return GL_COMPRESSED_RED;
        case ImageFormat::BC5:              return GL_COMPRESSED_RG;
        #endif
        default:                            break;
    }
    MapFailed("ImageFormat");
}

GLenum Map(const ImageFormat imageFormat)
{
    return MapImageFormat(imageFormat);
}

GLenum Map(const ImageFormat imageFormat, bool isIntegerType)
{
    if (isIntegerType)
        return MapIntegerImageFormat(imageFormat);
    else
        return MapImageFormat(imageFormat);
}

GLenum Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return GL_NEVER;
        case CompareOp::Less:           return GL_LESS;
        case CompareOp::Equal:          return GL_EQUAL;
        case CompareOp::LessEqual:      return GL_LEQUAL;
        case CompareOp::Greater:        return GL_GREATER;
        case CompareOp::NotEqual:       return GL_NOTEQUAL;
        case CompareOp::GreaterEqual:   return GL_GEQUAL;
        case CompareOp::AlwaysPass:     return GL_ALWAYS;
    }
    MapFailed("CompareOp");
}

GLenum Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return GL_KEEP;
        case StencilOp::Zero:       return GL_ZERO;
        case StencilOp::Replace:    return GL_REPLACE;
        case StencilOp::IncClamp:   return GL_INCR;
        case StencilOp::DecClamp:   return GL_DECR;
        case StencilOp::Invert:     return GL_INVERT;
        case StencilOp::IncWrap:    return GL_INCR_WRAP;
        case StencilOp::DecWrap:    return GL_DECR_WRAP;
    }
    MapFailed("StencilOp");
}

GLenum Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return GL_ZERO;
        case BlendOp::One:              return GL_ONE;
        case BlendOp::SrcColor:         return GL_SRC_COLOR;
        case BlendOp::InvSrcColor:      return GL_ONE_MINUS_SRC_COLOR;
        case BlendOp::SrcAlpha:         return GL_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
        case BlendOp::DstColor:         return GL_DST_COLOR;
        case BlendOp::InvDstColor:      return GL_ONE_MINUS_DST_COLOR;
        case BlendOp::DstAlpha:         return GL_DST_ALPHA;
        case BlendOp::InvDstAlpha:      return GL_ONE_MINUS_DST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
        case BlendOp::BlendFactor:      return GL_CONSTANT_COLOR;
        case BlendOp::InvBlendFactor:   return GL_ONE_MINUS_CONSTANT_COLOR;
        #ifdef LLGL_OPENGL
        case BlendOp::Src1Color:        return GL_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return GL_ONE_MINUS_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return GL_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return GL_ONE_MINUS_SRC1_ALPHA;
        #else
        default:                        break;
        #endif
    }
    MapFailed("BlendOp");
}

GLenum Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return GL_FUNC_ADD;
        case BlendArithmetic::Subtract:     return GL_FUNC_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return GL_FUNC_REVERSE_SUBTRACT;
        case BlendArithmetic::Min:          return GL_MIN;
        case BlendArithmetic::Max:          return GL_MAX;
    }
    MapFailed("BlendArithmetic");
}

GLenum Map(const PolygonMode polygonMode)
{
    #ifdef LLGL_OPENGL
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return GL_FILL;
        case PolygonMode::Wireframe:    return GL_LINE;
        case PolygonMode::Points:       return GL_POINT;
    }
    #endif
    MapFailed("PolygonMode");
}

GLenum Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return 0;
        case CullMode::Front:       return GL_FRONT;
        case CullMode::Back:        return GL_BACK;
    }
    MapFailed("CullMode");
}

GLenum Map(const SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode::Repeat:       return GL_REPEAT;
        case SamplerAddressMode::Mirror:       return GL_MIRRORED_REPEAT;
        case SamplerAddressMode::Clamp:        return GL_CLAMP_TO_EDGE;
        #ifdef LLGL_OPENGL
        case SamplerAddressMode::Border:       return GL_CLAMP_TO_BORDER;
        case SamplerAddressMode::MirrorOnce:   return GL_MIRROR_CLAMP_TO_EDGE;
        #else
        default:                        break;
        #endif
    }
    MapFailed("SamplerAddressMode");
}

GLenum Map(const SamplerFilter textureFilter)
{
    switch (textureFilter)
    {
        case SamplerFilter::Nearest:    return GL_NEAREST;
        case SamplerFilter::Linear:     return GL_LINEAR;
    }
    MapFailed("SamplerFilter");
}

GLenum Map(const SamplerFilter textureMinFilter, const SamplerFilter textureMipMapFilter)
{
    switch (textureMinFilter)
    {
        case SamplerFilter::Nearest:
            switch (textureMipMapFilter)
            {
                case SamplerFilter::Nearest:    return GL_NEAREST_MIPMAP_NEAREST;
                case SamplerFilter::Linear:     return GL_NEAREST_MIPMAP_LINEAR;
            }
            break;

        case SamplerFilter::Linear:
            switch (textureMipMapFilter)
            {
                case SamplerFilter::Nearest:    return GL_LINEAR_MIPMAP_NEAREST;
                case SamplerFilter::Linear:     return GL_LINEAR_MIPMAP_LINEAR;
            }
            break;
    }
    MapFailed("Min/MipMap SamplerFilter");
}

GLenum Map(const ShaderType shaderType)
{
    switch (shaderType)
    {
        case ShaderType::Vertex:            return GL_VERTEX_SHADER;
        #if defined(GL_VERSION_3_2) || defined(GL_ES_VERSION_3_2)
        case ShaderType::Geometry:          return GL_GEOMETRY_SHADER;
        #endif
        #if defined(GL_VERSION_4_0) || defined(GL_ES_VERSION_3_2)
        case ShaderType::TessControl:       return GL_TESS_CONTROL_SHADER;
        case ShaderType::TessEvaluation:    return GL_TESS_EVALUATION_SHADER;
        #endif
        case ShaderType::Fragment:          return GL_FRAGMENT_SHADER;
        #if defined(GL_VERSION_4_3) || defined(GL_ES_VERSION_3_1)
        case ShaderType::Compute:           return GL_COMPUTE_SHADER;
        #endif
        default:                            break;
    }
    MapFailed("ShaderType");
}

GLenum Map(const RenderConditionMode renderConditionMode)
{
    #ifdef LLGL_OPENGL
    switch (renderConditionMode)
    {
        case RenderConditionMode::Wait:                     return GL_QUERY_WAIT;
        case RenderConditionMode::NoWait:                   return GL_QUERY_NO_WAIT;
        case RenderConditionMode::ByRegionWait:             return GL_QUERY_BY_REGION_WAIT;
        case RenderConditionMode::ByRegionNoWait:           return GL_QUERY_BY_REGION_NO_WAIT;
        #ifndef __APPLE__
        case RenderConditionMode::WaitInverted:             return GL_QUERY_WAIT_INVERTED;
        case RenderConditionMode::NoWaitInverted:           return GL_QUERY_NO_WAIT_INVERTED;
        case RenderConditionMode::ByRegionWaitInverted:     return GL_QUERY_BY_REGION_WAIT_INVERTED;
        case RenderConditionMode::ByRegionNoWaitInverted:   return GL_QUERY_BY_REGION_NO_WAIT_INVERTED;
        #else
        default:                                            break;
        #endif
    }
    #endif
    MapFailed("RenderConditionMode");
}

GLenum Map(const LogicOp logicOp)
{
    #ifdef LLGL_OPENGL
    switch (logicOp)
    {
        case LogicOp::Disabled:     break;
        case LogicOp::Clear:        return GL_CLEAR;
        case LogicOp::Set:          return GL_SET;
        case LogicOp::Copy:         return GL_COPY;
        case LogicOp::CopyInverted: return GL_COPY_INVERTED;
        case LogicOp::NoOp:         return GL_NOOP;
        case LogicOp::Invert:       return GL_INVERT;
        case LogicOp::AND:          return GL_AND;
        case LogicOp::ANDReverse:   return GL_AND_REVERSE;
        case LogicOp::ANDInverted:  return GL_AND_INVERTED;
        case LogicOp::NAND:         return GL_NAND;
        case LogicOp::OR:           return GL_OR;
        case LogicOp::ORReverse:    return GL_OR_REVERSE;
        case LogicOp::ORInverted:   return GL_OR_INVERTED;
        case LogicOp::NOR:          return GL_NOR;
        case LogicOp::XOR:          return GL_XOR;
        case LogicOp::Equiv:        return GL_EQUIV;
    }
    #endif
    MapFailed("LogicOp");
}

GLenum Map(const StencilFace stencilFace)
{
    switch (stencilFace)
    {
        case StencilFace::FrontAndBack: return GL_FRONT_AND_BACK;
        case StencilFace::Front:        return GL_FRONT;
        case StencilFace::Back:         return GL_BACK;
    }
    MapFailed("StencilFace");
}

GLenum ToDrawMode(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:              return GL_POINTS;
        case PrimitiveTopology::LineList:               return GL_LINES;
        case PrimitiveTopology::LineStrip:              return GL_LINE_STRIP;
        case PrimitiveTopology::LineLoop:               return GL_LINE_LOOP;
        #ifdef LLGL_OPENGL
        case PrimitiveTopology::LineListAdjacency:      return GL_LINES_ADJACENCY;
        case PrimitiveTopology::LineStripAdjacency:     return GL_LINE_STRIP_ADJACENCY;
        #endif
        case PrimitiveTopology::TriangleList:           return GL_TRIANGLES;
        case PrimitiveTopology::TriangleStrip:          return GL_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:            return GL_TRIANGLE_FAN;
        #ifdef LLGL_OPENGL
        case PrimitiveTopology::TriangleListAdjacency:  return GL_TRIANGLES_ADJACENCY;
        case PrimitiveTopology::TriangleStripAdjacency: return GL_TRIANGLE_STRIP_ADJACENCY;
        #endif
        default:
            #ifdef LLGL_OPENGL
            if (primitiveTopology >= PrimitiveTopology::Patches1 && primitiveTopology <= PrimitiveTopology::Patches32)
                return GL_PATCHES;
            #endif
            break;
    }
    MapFailed("PrimitiveTopology");
}

GLenum ToPrimitiveMode(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:
            return GL_POINTS;
        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineLoop:
        case PrimitiveTopology::LineListAdjacency:
        case PrimitiveTopology::LineStripAdjacency:
            return GL_LINES;
        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::TriangleListAdjacency:
        case PrimitiveTopology::TriangleStripAdjacency:
            return GL_TRIANGLES;
        default:
            if (primitiveTopology >= PrimitiveTopology::Patches1 && primitiveTopology <= PrimitiveTopology::Patches32)
                return 0;
            break;
    }
    MapFailed("PrimitiveTopology");
}


/* ----- Unmap functions ----- */

UniformType UnmapUniformType(const GLenum uniformType)
{
    #ifdef LLGL_OPENGLES3

    switch (uniformType)
    {
        /* ----- Scalars/Vectors ----- */
        case GL_FLOAT:              return UniformType::Float1;
        case GL_FLOAT_VEC2:         return UniformType::Float2;
        case GL_FLOAT_VEC3:         return UniformType::Float3;
        case GL_FLOAT_VEC4:         return UniformType::Float4;
        case GL_INT:                return UniformType::Int1;
        case GL_INT_VEC2:           return UniformType::Int2;
        case GL_INT_VEC3:           return UniformType::Int3;
        case GL_INT_VEC4:           return UniformType::Int4;
        case GL_UNSIGNED_INT:       return UniformType::UInt1;
        case GL_UNSIGNED_INT_VEC2:  return UniformType::UInt2;
        case GL_UNSIGNED_INT_VEC3:  return UniformType::UInt3;
        case GL_UNSIGNED_INT_VEC4:  return UniformType::UInt4;
        case GL_BOOL:               return UniformType::Bool1;
        case GL_BOOL_VEC2:          return UniformType::Bool2;
        case GL_BOOL_VEC3:          return UniformType::Bool3;
        case GL_BOOL_VEC4:          return UniformType::Bool4;

        /* ----- Matrices ----- */
        case GL_FLOAT_MAT2:         return UniformType::Float2x2;
        case GL_FLOAT_MAT3:         return UniformType::Float3x3;
        case GL_FLOAT_MAT4:         return UniformType::Float4x4;
        case GL_FLOAT_MAT2x3:       return UniformType::Float2x3;
        case GL_FLOAT_MAT2x4:       return UniformType::Float2x4;
        case GL_FLOAT_MAT3x2:       return UniformType::Float3x2;
        case GL_FLOAT_MAT3x4:       return UniformType::Float3x4;
        case GL_FLOAT_MAT4x2:       return UniformType::Float4x2;
        case GL_FLOAT_MAT4x3:       return UniformType::Float4x3;

        /* ----- Samplers ----- */
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            return UniformType::Sampler;
    }

    #else

    switch (uniformType)
    {
        /* ----- Scalars/Vectors ----- */
        case GL_FLOAT:              return UniformType::Float1;
        case GL_FLOAT_VEC2:         return UniformType::Float2;
        case GL_FLOAT_VEC3:         return UniformType::Float3;
        case GL_FLOAT_VEC4:         return UniformType::Float4;
        case GL_DOUBLE:             return UniformType::Double1;
        case GL_DOUBLE_VEC2:        return UniformType::Double2;
        case GL_DOUBLE_VEC3:        return UniformType::Double3;
        case GL_DOUBLE_VEC4:        return UniformType::Double4;
        case GL_INT:                return UniformType::Int1;
        case GL_INT_VEC2:           return UniformType::Int2;
        case GL_INT_VEC3:           return UniformType::Int3;
        case GL_INT_VEC4:           return UniformType::Int4;
        case GL_UNSIGNED_INT:       return UniformType::UInt1;
        case GL_UNSIGNED_INT_VEC2:  return UniformType::UInt2;
        case GL_UNSIGNED_INT_VEC3:  return UniformType::UInt3;
        case GL_UNSIGNED_INT_VEC4:  return UniformType::UInt4;
        case GL_BOOL:               return UniformType::Bool1;
        case GL_BOOL_VEC2:          return UniformType::Bool2;
        case GL_BOOL_VEC3:          return UniformType::Bool3;
        case GL_BOOL_VEC4:          return UniformType::Bool4;

        /* ----- Matrices ----- */
        case GL_FLOAT_MAT2:         return UniformType::Float2x2;
        case GL_FLOAT_MAT2x3:       return UniformType::Float2x3;
        case GL_FLOAT_MAT2x4:       return UniformType::Float2x4;
        case GL_FLOAT_MAT3x2:       return UniformType::Float3x2;
        case GL_FLOAT_MAT3:         return UniformType::Float3x3;
        case GL_FLOAT_MAT3x4:       return UniformType::Float3x4;
        case GL_FLOAT_MAT4x2:       return UniformType::Float4x2;
        case GL_FLOAT_MAT4x3:       return UniformType::Float4x3;
        case GL_FLOAT_MAT4:         return UniformType::Float4x4;
        case GL_DOUBLE_MAT2:        return UniformType::Double2x2;
        case GL_DOUBLE_MAT2x3:      return UniformType::Double2x3;
        case GL_DOUBLE_MAT2x4:      return UniformType::Double2x4;
        case GL_DOUBLE_MAT3x2:      return UniformType::Double3x2;
        case GL_DOUBLE_MAT3:        return UniformType::Double3x3;
        case GL_DOUBLE_MAT3x4:      return UniformType::Double3x4;
        case GL_DOUBLE_MAT4x2:      return UniformType::Double4x2;
        case GL_DOUBLE_MAT4x3:      return UniformType::Double4x3;
        case GL_DOUBLE_MAT4:        return UniformType::Double4x4;

        /* ----- Samplers ----- */
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return UniformType::Sampler;

        #ifndef __APPLE__
        /* ----- Images ----- */
        case GL_UNSIGNED_INT_IMAGE_1D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        case GL_IMAGE_1D:
        case GL_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_IMAGE_2D_RECT:
        case GL_IMAGE_CUBE:
        case GL_IMAGE_BUFFER:
        case GL_IMAGE_1D_ARRAY:
        case GL_IMAGE_2D_ARRAY:
        case GL_IMAGE_2D_MULTISAMPLE:
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
        case GL_INT_IMAGE_1D:
        case GL_INT_IMAGE_2D:
        case GL_INT_IMAGE_3D:
        case GL_INT_IMAGE_2D_RECT:
        case GL_INT_IMAGE_CUBE:
        case GL_INT_IMAGE_BUFFER:
        case GL_INT_IMAGE_1D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_MULTISAMPLE:
        case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
            return UniformType::Image;

        /* ----- Misc ----- */
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
            return UniformType::AtomicCounter;
        #endif // /__APPLE__
    }

    #endif // /LLGL_OPENGLES3

    return UniformType::Undefined;
}

GLenum ToTextureCubeMap(std::uint32_t arrayLayer)
{
    static const GLenum g_textureCubeMaps[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    return g_textureCubeMaps[arrayLayer % 6];
}

GLenum ToColorAttachment(std::uint32_t attachmentIndex)
{
    if (attachmentIndex < LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        static const GLenum g_drawBuffers[] =
        {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7,
            #if LLGL_MAX_NUM_COLOR_ATTACHMENTS > 8
            GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
            GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15,
            #endif
        };
        return g_drawBuffers[attachmentIndex];
    }
    return 0;
}

Format UnmapFormat(const GLenum internalFormat)
{
    switch (internalFormat)
    {
        /* --- Red channel color formats --- */
        case GL_RED:                                    return Format::R8UNorm;

        case GL_R8:                                     return Format::R8UNorm;
        case GL_R8_SNORM:                               return Format::R8SNorm;
        case GL_R8UI:                                   return Format::R8UInt;
        case GL_R8I:                                    return Format::R8SInt;

        #ifdef LLGL_OPENGL
        case GL_R16:                                    return Format::R16UNorm;
        case GL_R16_SNORM:                              return Format::R16SNorm;
        #endif
        case GL_R16UI:                                  return Format::R16UInt;
        case GL_R16I:                                   return Format::R16SInt;
        case GL_R16F:                                   return Format::R16Float;

        case GL_R32I:                                   return Format::R32UInt;
        case GL_R32UI:                                  return Format::R32SInt;
        case GL_R32F:                                   return Format::R32Float;

        /* --- RG color formats --- */
        case GL_RG:                                     return Format::RG8UNorm;

        case GL_RG8:                                    return Format::RG8UNorm;
        case GL_RG8_SNORM:                              return Format::RG8SNorm;
        case GL_RG8UI:                                  return Format::RG8UInt;
        case GL_RG8I:                                   return Format::RG8SInt;

        #ifdef LLGL_OPENGL
        case GL_RG16:                                   return Format::RG16UNorm;
        case GL_RG16_SNORM:                             return Format::RG16SNorm;
        #endif
        case GL_RG16UI:                                 return Format::RG16UInt;
        case GL_RG16I:                                  return Format::RG16SInt;
        case GL_RG16F:                                  return Format::RG16Float;

        case GL_RG32UI:                                 return Format::RG32UInt;
        case GL_RG32I:                                  return Format::RG32SInt;
        case GL_RG32F:                                  return Format::RG32Float;

        /* --- RGB color formats --- */
        case GL_RGB:                                    return Format::RGB8UNorm;

        case GL_RGB8:                                   return Format::RGB8UNorm;
        case GL_RGB8_SNORM:                             return Format::RGB8SNorm;
        case GL_RGB8UI:                                 return Format::RGB8UInt;
        case GL_RGB8I:                                  return Format::RGB8SInt;

        #ifdef LLGL_OPENGL
        case GL_RGB16:                                  return Format::RGB16UNorm;
        case GL_RGB16_SNORM:                            return Format::RGB16SNorm;
        #endif
        case GL_RGB16UI:                                return Format::RGB16UInt;
        case GL_RGB16I:                                 return Format::RGB16SInt;
        case GL_RGB16F:                                 return Format::RGB16Float;

        case GL_RGB32UI:                                return Format::RGB32UInt;
        case GL_RGB32I:                                 return Format::RGB32SInt;
        case GL_RGB32F:                                 return Format::RGB32Float;

        /* --- RGBA color formats --- */
        case GL_RGBA:                                   return Format::RGBA8UNorm;

        case GL_RGBA8:                                  return Format::RGBA8UNorm;
        case GL_RGBA8_SNORM:                            return Format::RGBA8SNorm;
        case GL_RGBA8UI:                                return Format::RGBA8UInt;
        case GL_RGBA8I:                                 return Format::RGBA8SInt;

        #ifdef LLGL_OPENGL
        case GL_RGBA16:                                 return Format::RGBA16UNorm;
        case GL_RGBA16_SNORM:                           return Format::RGBA16SNorm;
        #endif
        case GL_RGBA16UI:                               return Format::RGBA16UInt;
        case GL_RGBA16I:                                return Format::RGBA16SInt;
        case GL_RGBA16F:                                return Format::RGBA16Float;

        case GL_RGBA32UI:                               return Format::RGBA32UInt;
        case GL_RGBA32I:                                return Format::RGBA32SInt;
        case GL_RGBA32F:                                return Format::RGBA32Float;

        #ifdef LLGL_OPENGL
        /* --- Packed formats --- */
        case GL_RGB10_A2:                               return Format::RGB10A2UNorm;
        case GL_RGB10_A2UI:                             return Format::RGB10A2UInt;
        case GL_R11F_G11F_B10F:                         return Format::RG11B10Float;
        case GL_RGB9_E5:                                return Format::RGB9E5Float;
        #endif

        /* --- Depth-stencil formats --- */
        case GL_DEPTH_COMPONENT16:                      return Format::D16UNorm;
        #ifdef LLGL_OPENGL
        case GL_DEPTH_COMPONENT32:                      /* pass */
        #else
        case GL_DEPTH_COMPONENT32F:
        #endif
        case GL_DEPTH_COMPONENT:                        return Format::D32Float;
        case GL_DEPTH24_STENCIL8:                       /* pass */
        case GL_DEPTH_STENCIL:                          return Format::D24UNormS8UInt;
        case GL_DEPTH32F_STENCIL8:                      return Format::D32FloatS8X24UInt;

        /* --- Block compression (BC) formats --- */
        #ifdef GL_EXT_texture_compression_s3tc
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:          return Format::BC1UNorm;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:          return Format::BC2UNorm;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:          return Format::BC3UNorm;
        #endif // /GL_EXT_texture_compression_s3tc

        #ifdef GL_EXT_texture_sRGB
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:    return Format::BC1UNorm_sRGB;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:    return Format::BC2UNorm_sRGB;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:    return Format::BC3UNorm_sRGB;
        #endif // /GL_EXT_texture_sRGB

        #ifdef GL_EXT_texture_compression_rgtc
        case GL_COMPRESSED_RED_RGTC1_EXT:               return Format::BC4UNorm;
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:        return Format::BC4SNorm;
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:         return Format::BC5UNorm;
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:  return Format::BC5SNorm;
        #endif // /GL_EXT_texture_compression_rgtc

        default:                                        break;
    }
    return Format::Undefined;
}

DataType UnmapDataType(const GLenum type)
{
    switch (type)
    {
        case GL_BYTE:           return DataType::Int8;
        case GL_UNSIGNED_BYTE:  return DataType::UInt8;
        case GL_SHORT:          return DataType::Int16;
        case GL_UNSIGNED_SHORT: return DataType::UInt16;
        case GL_INT:            return DataType::Int32;
        case GL_UNSIGNED_INT:   return DataType::UInt32;
        case GL_HALF_FLOAT:     return DataType::Float16;
        case GL_FLOAT:          return DataType::Float32;
        #ifdef LLGL_OPENGL
        case GL_DOUBLE:         return DataType::Float64;
        #endif
    }
    UnmapFailed("DataType");
}

bool IsIntegerTypedFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
        case GL_R8UI:
        case GL_R8I:
        case GL_R16UI:
        case GL_R16I:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8UI:
        case GL_RG8I:
        case GL_RG16UI:
        case GL_RG16I:
        case GL_RG32UI:
        case GL_RG32I:
        case GL_RGB8UI:
        case GL_RGB8I:
        case GL_RGB16UI:
        case GL_RGB16I:
        case GL_RGB32UI:
        case GL_RGB32I:
        case GL_RGBA8UI:
        case GL_RGBA8I:
        case GL_RGBA16UI:
        case GL_RGBA16I:
        case GL_RGBA32UI:
        case GL_RGBA32I:
            return true;
        default:
            return false;
    }
}


} // /namespace GLTypes

} // /namespace LLGL



// ================================================================================
