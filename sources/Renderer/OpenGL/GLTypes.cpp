/*
 * GLTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace GLTypes
{


[[noreturn]]
static void MapFailed(const std::string& typeName)
{
    throw std::invalid_argument("failed to map '" + typeName + "' to OpenGL parameter");
}

[[noreturn]]
static void UnmapFailed(const std::string& typeName)
{
    throw std::invalid_argument("failed to unmap '" + typeName + "' from OpenGL parameter");
}

GLenum Map(const BufferUsage bufferUsage)
{
    switch (bufferUsage)
    {
        case BufferUsage::Static:   return GL_STATIC_DRAW;
        case BufferUsage::Dynamic:  return GL_DYNAMIC_DRAW;
    }
    MapFailed("BufferUsage");
}

GLenum Map(const BufferCPUAccess cpuAccess)
{
    switch (cpuAccess)
    {
        case BufferCPUAccess::ReadOnly:     return GL_READ_ONLY;
        case BufferCPUAccess::WriteOnly:    return GL_WRITE_ONLY;
        case BufferCPUAccess::ReadWrite:    return GL_READ_WRITE;
    }
    MapFailed("BufferCPUAccess");
}

GLenum Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Float:   return GL_FLOAT;
        case DataType::Double:  return GL_DOUBLE;
        case DataType::Byte:    return GL_BYTE;
        case DataType::UByte:   return GL_UNSIGNED_BYTE;
        case DataType::Short:   return GL_SHORT;
        case DataType::UShort:  return GL_UNSIGNED_SHORT;
        case DataType::Int:     return GL_INT;
        case DataType::UInt:    return GL_UNSIGNED_INT;
    }
    MapFailed("DataType");
}

GLenum Map(const DrawMode drawMode)
{
    switch (drawMode)
    {
        case DrawMode::Points:                  return GL_POINTS;
        case DrawMode::Lines:                   return GL_LINES;
        case DrawMode::LineStrip:               return GL_LINE_STRIP;
        case DrawMode::LineLoop:                return GL_LINE_LOOP;
        case DrawMode::LinesAdjacency:          return GL_LINES_ADJACENCY;
        case DrawMode::LineStripAdjacency:      return GL_LINE_STRIP_ADJACENCY;
        case DrawMode::Triangles:               return GL_TRIANGLES;
        case DrawMode::TriangleStrip:           return GL_TRIANGLE_STRIP;
        case DrawMode::TriangleFan:             return GL_TRIANGLE_FAN;
        case DrawMode::TrianglesAdjacency:      return GL_TRIANGLES_ADJACENCY;
        case DrawMode::TriangleStripAdjacency:  return GL_TRIANGLE_STRIP_ADJACENCY;
        case DrawMode::Patches:                 return GL_PATCHES;
    }
    MapFailed("DrawMode");
}

GLenum Map(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return GL_TEXTURE_1D;
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP;
        case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
        default:                            break;
    }
    MapFailed("TextureType");
}

GLenum Map(const TextureFormat textureFormat)
{
    switch (textureFormat)
    {
        /* --- Base internal formats --- */
        case TextureFormat::DepthComponent: return GL_DEPTH_COMPONENT;
        case TextureFormat::DepthStencil:   return GL_DEPTH_STENCIL;
        case TextureFormat::R:              return GL_RED;
        case TextureFormat::RG:             return GL_RG;
        case TextureFormat::RGB:            return GL_RGB;
        case TextureFormat::RGBA:           return GL_RGBA;

        /* --- Sized internal formats --- */
        case TextureFormat::R8UInt:         return GL_R8UI;
        case TextureFormat::R8SInt:         return GL_R8I;

        case TextureFormat::R16UInt:        return GL_R16UI;
        case TextureFormat::R16SInt:        return GL_R16I;
        case TextureFormat::R16Float:       return GL_R16F;

        case TextureFormat::R32UInt:        return GL_R32UI;
        case TextureFormat::R32SInt:        return GL_R32I;
        case TextureFormat::R32Float:       return GL_R32F;

        case TextureFormat::RG8UInt:        return GL_RG8UI;
        case TextureFormat::RG8SInt:        return GL_RG8I;

        case TextureFormat::RG16UInt:       return GL_RG16UI;
        case TextureFormat::RG16SInt:       return GL_RG16I;
        case TextureFormat::RG16Float:      return GL_RG16F;

        case TextureFormat::RG32UInt:       return GL_RG32UI;
        case TextureFormat::RG32SInt:       return GL_RG32I;
        case TextureFormat::RG32Float:      return GL_RG32F;

        case TextureFormat::RGB8UInt:       return GL_RGB8UI;
        case TextureFormat::RGB8SInt:       return GL_RGB8I;

        case TextureFormat::RGB16UInt:      return GL_RGB16UI;
        case TextureFormat::RGB16SInt:      return GL_RGB16I;
        case TextureFormat::RGB16Float:     return GL_RGB16F;

        case TextureFormat::RGB32UInt:      return GL_RGB32UI;
        case TextureFormat::RGB32SInt:      return GL_RGB32I;
        case TextureFormat::RGB32Float:     return GL_RGB32F;

        case TextureFormat::RGBA8UInt:      return GL_RGBA8UI;
        case TextureFormat::RGBA8SInt:      return GL_RGBA8I;

        case TextureFormat::RGBA16UInt:     return GL_RGBA16UI;
        case TextureFormat::RGBA16SInt:     return GL_RGBA16I;
        case TextureFormat::RGBA16Float:    return GL_RGBA16F;

        case TextureFormat::RGBA32UInt:     return GL_RGBA32UI;
        case TextureFormat::RGBA32SInt:     return GL_RGBA32I;
        case TextureFormat::RGBA32Float:    return GL_RGBA32F;
    }
    MapFailed("TextureFormat");
}

GLenum Map(const ColorFormat colorFormat)
{
    switch (colorFormat)
    {
        case ColorFormat::Gray:         return GL_RED;
        case ColorFormat::GrayAlpha:    return GL_RG;
        case ColorFormat::RGB:          return GL_RGB;
        case ColorFormat::BGR:          return GL_BGR;
        case ColorFormat::RGBA:         return GL_RGBA;
        case ColorFormat::BGRA:         return GL_BGRA;
        case ColorFormat::Depth:        return GL_DEPTH_COMPONENT;  
        case ColorFormat::DepthStencil: return GL_DEPTH_STENCIL;
    }
    MapFailed("ColorFormat");
}

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
        case BlendOp::Zero:         return GL_ZERO;
        case BlendOp::One:          return GL_ONE;
        case BlendOp::SrcColor:     return GL_SRC_COLOR;
        case BlendOp::InvSrcColor:  return GL_ONE_MINUS_SRC_COLOR;
        case BlendOp::SrcAlpha:     return GL_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:  return GL_ONE_MINUS_SRC_ALPHA;
        case BlendOp::DestColor:    return GL_DST_COLOR;
        case BlendOp::InvDestColor: return GL_ONE_MINUS_DST_COLOR;
        case BlendOp::DestAlpha:    return GL_DST_ALPHA;
        case BlendOp::InvDestAlpha: return GL_ONE_MINUS_DST_ALPHA;
    }
    MapFailed("BlendOp");
}

GLenum Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return GL_FILL;
        case PolygonMode::Wireframe:    return GL_LINE;
        case PolygonMode::Points:       return GL_POINT;
    }
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

void Unmap(UniformType& result, const GLenum uniformType)
{
    switch (uniformType)
    {
        case GL_FLOAT:
            result = UniformType::Float;
            return;
        case GL_FLOAT_VEC2:
            result = UniformType::Float2;
            return;
        case GL_FLOAT_VEC3:
            result = UniformType::Float3;
            return;
        case GL_FLOAT_VEC4:
            result = UniformType::Float4;
            return;
        case GL_DOUBLE:
            result = UniformType::Double;
            return;
        case GL_DOUBLE_VEC2:
            result = UniformType::Double2;
            return;
        case GL_DOUBLE_VEC3:
            result = UniformType::Double3;
            return;
        case GL_DOUBLE_VEC4:
            result = UniformType::Double4;
            return;
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
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        case GL_INT:
            result = UniformType::Int;
            return;
        case GL_INT_VEC2:
            result = UniformType::Int2;
            return;
        case GL_INT_VEC3:
            result = UniformType::Int3;
            return;
        case GL_INT_VEC4:
            result = UniformType::Int4;
            return;
        /*case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:*/
        /*case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:*/
        case GL_FLOAT_MAT2:
            result = UniformType::Float2x2;
            return;
        case GL_FLOAT_MAT3:
            result = UniformType::Float3x3;
            return;
        case GL_FLOAT_MAT4:
            result = UniformType::Float4x4;
            return;
        /*case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:*/
        case GL_DOUBLE_MAT2:
            result = UniformType::Double2x2;
            return;
        case GL_DOUBLE_MAT3:
            result = UniformType::Double3x3;
            return;
        case GL_DOUBLE_MAT4:
            result = UniformType::Double4x4;
            return;
        /*case GL_DOUBLE_MAT2x3:
        case GL_DOUBLE_MAT2x4:
        case GL_DOUBLE_MAT3x2:
        case GL_DOUBLE_MAT3x4:
        case GL_DOUBLE_MAT4x2:
        case GL_DOUBLE_MAT4x3:*/
    }
    UnmapFailed("UniformType");
}


} // /namespace GLTypeConversion

} // /namespace LLGL



// ================================================================================
