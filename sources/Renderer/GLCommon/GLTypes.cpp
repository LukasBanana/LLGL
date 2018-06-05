/*
 * GLTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTypes.h"
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

/*[[noreturn]]
static void UnmapFailed(const char* typeName)
{
    throw std::invalid_argument("failed to unmap <LLGL::" + std::string(typeName) + "> from OpenGL parameter");
}*/


/* ----- MapOrZero functions ----- */

GLenum MapOrZero(const TextureFormat textureFormat)
{
    switch (textureFormat)
    {
        case TextureFormat::Unknown:        return 0;

        /* --- Color formats --- */
        case TextureFormat::R8:             return GL_R8;
        case TextureFormat::R8Sgn:          return GL_R8_SNORM;

        #ifdef LLGL_OPENGL
        case TextureFormat::R16:            return GL_R16;
        case TextureFormat::R16Sgn:         return GL_R16_SNORM;
        #endif
        case TextureFormat::R16Float:       return GL_R16F;

        case TextureFormat::R32UInt:        return GL_R32I;
        case TextureFormat::R32SInt:        return GL_R32UI;
        case TextureFormat::R32Float:       return GL_R32F;

        case TextureFormat::RG8:            return GL_RG8;
        case TextureFormat::RG8Sgn:         return GL_RG8_SNORM;

        #ifdef LLGL_OPENGL
        case TextureFormat::RG16:           return GL_RG16;
        case TextureFormat::RG16Sgn:        return GL_RG16_SNORM;
        #endif
        case TextureFormat::RG16Float:      return GL_RG16F;

        case TextureFormat::RG32UInt:       return GL_RG32UI;
        case TextureFormat::RG32SInt:       return GL_RG32I;
        case TextureFormat::RG32Float:      return GL_RG32F;

        case TextureFormat::RGB8:           return GL_RGB8;
        case TextureFormat::RGB8Sgn:        return GL_RGB8_SNORM;

        #ifdef LLGL_OPENGL
        case TextureFormat::RGB16:          return GL_RGB16;
        case TextureFormat::RGB16Sgn:       return GL_RGB16_SNORM;
        #endif
        case TextureFormat::RGB16Float:     return GL_RGB16F;

        case TextureFormat::RGB32UInt:      return GL_RGB32UI;
        case TextureFormat::RGB32SInt:      return GL_RGB32I;
        case TextureFormat::RGB32Float:     return GL_RGB32F;

        case TextureFormat::RGBA8:          return GL_RGBA8;
        case TextureFormat::RGBA8Sgn:       return GL_RGBA8_SNORM;

        #ifdef LLGL_OPENGL
        case TextureFormat::RGBA16:         return GL_RGBA16;
        case TextureFormat::RGBA16Sgn:      return GL_RGBA16_SNORM;
        #endif
        case TextureFormat::RGBA16Float:    return GL_RGBA16F;

        case TextureFormat::RGBA32UInt:     return GL_RGBA32UI;
        case TextureFormat::RGBA32SInt:     return GL_RGBA32I;
        case TextureFormat::RGBA32Float:    return GL_RGBA32F;

        /* --- Depth-stencil formats --- */
        case TextureFormat::D32:            return GL_DEPTH_COMPONENT;
        case TextureFormat::D24S8:          return GL_DEPTH_STENCIL;

        #ifdef GL_EXT_texture_compression_s3tc
        /* --- Compressed color formats --- */
        case TextureFormat::RGB_DXT1:       return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case TextureFormat::RGBA_DXT1:      return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case TextureFormat::RGBA_DXT3:      return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case TextureFormat::RGBA_DXT5:      return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        #endif

        default:                            return 0;
    }
}

/* ----- Map functions ----- */

GLenum Map(const CPUAccess cpuAccess)
{
    #ifdef LLGL_OPENGL
    switch (cpuAccess)
    {
        case CPUAccess::ReadOnly:   return GL_READ_ONLY;
        case CPUAccess::WriteOnly:  return GL_WRITE_ONLY;
        case CPUAccess::ReadWrite:  return GL_READ_WRITE;
    }
    #endif
    MapFailed("CPUAccess");
}

GLenum Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Int8:    return GL_BYTE;
        case DataType::UInt8:   return GL_UNSIGNED_BYTE;
        case DataType::Int16:   return GL_SHORT;
        case DataType::UInt16:  return GL_UNSIGNED_SHORT;
        case DataType::Int32:   return GL_INT;
        case DataType::UInt32:  return GL_UNSIGNED_INT;
        case DataType::Float:   return GL_FLOAT;
        #ifdef LLGL_OPENGL
        case DataType::Double:  return GL_DOUBLE;
        #else
        default:                break;
        #endif
    }
    MapFailed("DataType");
}

GLenum Map(const PrimitiveType primitiveType)
{
    switch (primitiveType)
    {
        case PrimitiveType::Points:     return GL_POINTS;
        case PrimitiveType::Lines:      return GL_LINES;
        case PrimitiveType::Triangles:  return GL_TRIANGLES;
    }
    MapFailed("PrimitiveType");
}

GLenum Map(const PrimitiveTopology primitiveTopology)
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

GLenum Map(const TextureFormat textureFormat)
{
    if (auto result = MapOrZero(textureFormat))
        return result;
    MapFailed("TextureFormat");
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
        case ImageFormat::BGRA:             return GL_BGRA;
        case ImageFormat::Depth:            return GL_DEPTH_COMPONENT;
        case ImageFormat::DepthStencil:     return GL_DEPTH_STENCIL;
        #ifdef LLGL_OPENGL
        case ImageFormat::CompressedRGB:    return GL_COMPRESSED_RGB;
        case ImageFormat::CompressedRGBA:   return GL_COMPRESSED_RGBA;
        #endif
        default:                            break;
    }
    MapFailed("ImageFormat");
}

/*static GLenum MapIntegerImageFormat(const ImageFormat imageFormat)
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
        case ImageFormat::CompressedRGB:    return GL_COMPRESSED_RGB;
        case ImageFormat::CompressedRGBA:   return GL_COMPRESSED_RGBA;
        #endif
        default:                            break;
    }
    MapFailed("ImageFormat");
}*/

GLenum Map(const ImageFormat imageFormat)
{
    return MapImageFormat(imageFormat);
}

/*GLenum Map(const ImageFormat imageFormat, bool integerFormat)
{
    if (integerFormat)
        return MapIntegerImageFormat(imageFormat);
    else
        return MapImageFormat(imageFormat);
}*/

GLenum Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::Never:          return GL_NEVER;
        case CompareOp::Less:           return GL_LESS;
        case CompareOp::Equal:          return GL_EQUAL;
        case CompareOp::LessEqual:      return GL_LEQUAL;
        case CompareOp::Greater:        return GL_GREATER;
        case CompareOp::NotEqual:       return GL_NOTEQUAL;
        case CompareOp::GreaterEqual:   return GL_GEQUAL;
        case CompareOp::Ever:           return GL_ALWAYS;
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

GLenum Map(const AxisDirection cubeFace)
{
    switch (cubeFace)
    {
        case AxisDirection::XPos: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case AxisDirection::XNeg: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case AxisDirection::YPos: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case AxisDirection::YNeg: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case AxisDirection::ZPos: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case AxisDirection::ZNeg: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    }
    MapFailed("AxisDirection");
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

GLenum Map(const BufferType bufferType)
{
    switch (bufferType)
    {
        case BufferType::Vertex:        return GL_ARRAY_BUFFER;
        case BufferType::Index:         return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::Constant:      return GL_UNIFORM_BUFFER;
        #ifdef LLGL_OPENGL
        case BufferType::Storage:       return GL_SHADER_STORAGE_BUFFER;
        #endif
        case BufferType::StreamOutput:  return GL_TRANSFORM_FEEDBACK_BUFFER;
        default:                        break;
    }
    MapFailed("BufferType");
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


/* ----- Unmap functions ----- */

static UniformType UnmapUniformType(const GLenum uniformType)
{
    #ifdef LLGL_OPENGLES3

    switch (uniformType)
    {
        /* ----- Scalars/Vectors ----- */
        case GL_FLOAT:              return UniformType::Float;
        case GL_FLOAT_VEC2:         return UniformType::Float2;
        case GL_FLOAT_VEC3:         return UniformType::Float3;
        case GL_FLOAT_VEC4:         return UniformType::Float4;
        case GL_INT:                return UniformType::Int;
        case GL_INT_VEC2:           return UniformType::Int2;
        case GL_INT_VEC3:           return UniformType::Int3;
        case GL_INT_VEC4:           return UniformType::Int4;
        case GL_UNSIGNED_INT:       return UniformType::UInt;
        case GL_UNSIGNED_INT_VEC2:  return UniformType::UInt2;
        case GL_UNSIGNED_INT_VEC3:  return UniformType::UInt3;
        case GL_UNSIGNED_INT_VEC4:  return UniformType::UInt4;
        case GL_BOOL:               return UniformType::Bool;
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
        case GL_FLOAT_MAT3:         return UniformType::Float3x3;
        case GL_FLOAT_MAT4:         return UniformType::Float4x4;
        case GL_FLOAT_MAT2x3:       return UniformType::Float2x3;
        case GL_FLOAT_MAT2x4:       return UniformType::Float2x4;
        case GL_FLOAT_MAT3x2:       return UniformType::Float3x2;
        case GL_FLOAT_MAT3x4:       return UniformType::Float3x4;
        case GL_FLOAT_MAT4x2:       return UniformType::Float4x2;
        case GL_FLOAT_MAT4x3:       return UniformType::Float4x3;
        case GL_DOUBLE_MAT2:        return UniformType::Double2x2;
        case GL_DOUBLE_MAT3:        return UniformType::Double3x3;
        case GL_DOUBLE_MAT4:        return UniformType::Double4x4;
        case GL_DOUBLE_MAT2x3:      return UniformType::Double2x3;
        case GL_DOUBLE_MAT2x4:      return UniformType::Double2x4;
        case GL_DOUBLE_MAT3x2:      return UniformType::Double3x2;
        case GL_DOUBLE_MAT3x4:      return UniformType::Double3x4;
        case GL_DOUBLE_MAT4x2:      return UniformType::Double4x2;
        case GL_DOUBLE_MAT4x3:      return UniformType::Double4x3;

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

void Unmap(UniformType& result, const GLenum uniformType)
{
    result = UnmapUniformType(uniformType);
}

static TextureFormat UnmapTextureFormat(const GLenum internalFormat)
{
    switch (internalFormat)
    {
        /* --- Base internal formats --- */
        case GL_RED:                            return TextureFormat::R8;
        case GL_RG:                             return TextureFormat::RG8;
        case GL_RGB:                            return TextureFormat::RGB8;
        case GL_RGBA:                           return TextureFormat::RGBA8;

        /* --- Color formats --- */
        case GL_R8:                             return TextureFormat::R8;
        case GL_R8_SNORM:                       return TextureFormat::R8Sgn;

        #ifdef LLGL_OPENGL
        case GL_R16:                            return TextureFormat::R16;
        case GL_R16_SNORM:                      return TextureFormat::R16Sgn;
        #endif
        case GL_R16F:                           return TextureFormat::R16Float;

        case GL_R32I:                           return TextureFormat::R32UInt;
        case GL_R32UI:                          return TextureFormat::R32SInt;
        case GL_R32F:                           return TextureFormat::R32Float;

        case GL_RG8:                            return TextureFormat::RG8;
        case GL_RG8_SNORM:                      return TextureFormat::RG8Sgn;

        #ifdef LLGL_OPENGL
        case GL_RG16:                           return TextureFormat::RG16;
        case GL_RG16_SNORM:                     return TextureFormat::RG16Sgn;
        #endif
        case GL_RG16F:                          return TextureFormat::RG16Float;

        case GL_RG32UI:                         return TextureFormat::RG32UInt;
        case GL_RG32I:                          return TextureFormat::RG32SInt;
        case GL_RG32F:                          return TextureFormat::RG32Float;

        case GL_RGB8:                           return TextureFormat::RGB8;
        case GL_RGB8_SNORM:                     return TextureFormat::RGB8Sgn;

        #ifdef LLGL_OPENGL
        case GL_RGB16:                          return TextureFormat::RGB16;
        case GL_RGB16_SNORM:                    return TextureFormat::RGB16Sgn;
        #endif
        case GL_RGB16F:                         return TextureFormat::RGB16Float;

        case GL_RGB32UI:                        return TextureFormat::RGB32UInt;
        case GL_RGB32I:                         return TextureFormat::RGB32SInt;
        case GL_RGB32F:                         return TextureFormat::RGB32Float;

        case GL_RGBA8:                          return TextureFormat::RGBA8;
        case GL_RGBA8_SNORM:                    return TextureFormat::RGBA8Sgn;

        #ifdef LLGL_OPENGL
        case GL_RGBA16:                         return TextureFormat::RGBA16;
        case GL_RGBA16_SNORM:                   return TextureFormat::RGBA16Sgn;
        #endif
        case GL_RGBA16F:                        return TextureFormat::RGBA16Float;

        case GL_RGBA32UI:                       return TextureFormat::RGBA32UInt;
        case GL_RGBA32I:                        return TextureFormat::RGBA32SInt;
        case GL_RGBA32F:                        return TextureFormat::RGBA32Float;

        /* --- Depth-stencil formats --- */
        case GL_DEPTH_COMPONENT:                return TextureFormat::D32;
        case GL_DEPTH_STENCIL:                  return TextureFormat::D24S8;

        #ifdef LLGL_OPENGL
        /* --- Compressed color formats --- */
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   return TextureFormat::RGB_DXT1;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  return TextureFormat::RGBA_DXT1;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  return TextureFormat::RGBA_DXT3;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  return TextureFormat::RGBA_DXT5;
        #endif
        
        default:                                break;
    }
    return TextureFormat::Unknown;
}

void Unmap(TextureFormat& result, const GLenum internalFormat)
{
    result = UnmapTextureFormat(internalFormat);
}


} // /namespace GLTypes

} // /namespace LLGL



// ================================================================================
