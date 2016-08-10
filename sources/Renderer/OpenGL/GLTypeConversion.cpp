/*
 * GLTypeConversion.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTypeConversion.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace GLTypeConversion
{


[[noreturn]]
static void MapFailed(const std::string& typeName)
{
    throw std::invalid_argument("failed to map '" + typeName + "' to OpenGL parameter");
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
    }
    MapFailed("TextureType");
}

GLenum Map(const TextureFormat textureFormat)
{
    switch (textureFormat)
    {
        /* --- Base internal formats --- */
        case TextureFormat::DepthComponent:
            return GL_DEPTH_COMPONENT;
        case TextureFormat::DepthStencil:
            return GL_DEPTH_STENCIL;
        case TextureFormat::R:
            return GL_RED;
        case TextureFormat::RG:
            return GL_RG;
        case TextureFormat::RGB:
            return GL_RGB;
        case TextureFormat::RGBA:
            return GL_RGBA;

        /* --- Sized internal formats --- */
        case TextureFormat::R8UInt:
            return GL_R8UI;
        case TextureFormat::R8SInt:
            return GL_R8I;

        case TextureFormat::R16UInt:
            return GL_R16UI;
        case TextureFormat::R16SInt:
            return GL_R16I;
        case TextureFormat::R16Float:
            return GL_R16F;

        case TextureFormat::R32UInt:
            return GL_R32UI;
        case TextureFormat::R32SInt:
            return GL_R32I;
        case TextureFormat::R32Float:
            return GL_R32F;

        case TextureFormat::RG8UInt:
            return GL_RG8UI;
        case TextureFormat::RG8SInt:
            return GL_RG8I;

        case TextureFormat::RG16UInt:
            return GL_RG16UI;
        case TextureFormat::RG16SInt:
            return GL_RG16I;
        case TextureFormat::RG16Float:
            return GL_RG16F;

        case TextureFormat::RG32UInt:
            return GL_RG32UI;
        case TextureFormat::RG32SInt:
            return GL_RG32I;
        case TextureFormat::RG32Float:
            return GL_RG32F;

        case TextureFormat::RGB8UInt:
            return GL_RGB8UI;
        case TextureFormat::RGB8SInt:
            return GL_RGB8I;

        case TextureFormat::RGB16UInt:
            return GL_RGB16UI;
        case TextureFormat::RGB16SInt:
            return GL_RGB16I;
        case TextureFormat::RGB16Float:
            return GL_RGB16F;

        case TextureFormat::RGB32UInt:
            return GL_RGB32UI;
        case TextureFormat::RGB32SInt:
            return GL_RGB32I;
        case TextureFormat::RGB32Float:
            return GL_RGB32F;

        case TextureFormat::RGBA8UInt:
            return GL_RGBA8UI;
        case TextureFormat::RGBA8SInt:
            return GL_RGBA8I;

        case TextureFormat::RGBA16UInt:
            return GL_RGBA16UI;
        case TextureFormat::RGBA16SInt:
            return GL_RGBA16I;
        case TextureFormat::RGBA16Float:
            return GL_RGBA16F;

        case TextureFormat::RGBA32UInt:
            return GL_RGBA32UI;
        case TextureFormat::RGBA32SInt:
            return GL_RGBA32I;
        case TextureFormat::RGBA32Float:
            return GL_RGBA32F;
    }
    MapFailed("TextureFormat");
}

GLenum Map(const ColorFormat colorFormat)
{
    switch (colorFormat)
    {
        case ColorFormat::Gray:
            return GL_RED;
        case ColorFormat::GrayAlpha:
            return GL_RG;
        case ColorFormat::RGB:
            return GL_RGB;
        case ColorFormat::BGR:
            return GL_BGR;
        case ColorFormat::RGBA:
            return GL_RGBA;
        case ColorFormat::BGRA:
            return GL_BGRA;
        case ColorFormat::Depth:
            return GL_DEPTH_COMPONENT;
        case ColorFormat::DepthStencil:
            return GL_DEPTH_STENCIL;
    }
    MapFailed("ColorFormat");
}


} // /namespace GLTypeConversion

} // /namespace LLGL



// ================================================================================
