/*
 * TextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/TextureFlags.h>
#include <algorithm>
#include <cmath>


namespace LLGL
{


LLGL_EXPORT std::uint32_t NumMipLevels(std::uint32_t width, std::uint32_t height, std::uint32_t depth)
{
    auto maxSize = std::max({ width, height, depth });
    auto log2Size = static_cast<std::uint32_t>(std::log2(maxSize));
    return (1 + log2Size);
}

LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc)
{
    if ((textureDesc.flags & TextureFlags::GenerateMips) != 0)
    {
        switch (textureDesc.type)
        {
            case TextureType::Texture1D:        return NumMipLevels(textureDesc.texture1D.width);
            case TextureType::Texture2D:        return NumMipLevels(textureDesc.texture2D.width, textureDesc.texture2D.height);
            case TextureType::Texture3D:        return NumMipLevels(textureDesc.texture3D.width, textureDesc.texture3D.height, textureDesc.texture3D.depth);
            case TextureType::TextureCube:      return NumMipLevels(textureDesc.textureCube.width, textureDesc.textureCube.height);
            case TextureType::Texture1DArray:   return NumMipLevels(textureDesc.texture1D.width);
            case TextureType::Texture2DArray:   return NumMipLevels(textureDesc.texture2D.width, textureDesc.texture2D.height);
            case TextureType::TextureCubeArray: return NumMipLevels(textureDesc.textureCube.width, textureDesc.textureCube.height);
            case TextureType::Texture2DMS:      return 1u;
            case TextureType::Texture2DMSArray: return 1u;
        }
    }
    return 1u;
}

LLGL_EXPORT std::uint32_t NumArrayLayers(const TextureDescriptor& textureDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1DArray:   return textureDesc.texture1D.layers;
        case TextureType::Texture2DArray:   return textureDesc.texture2D.layers;
        case TextureType::TextureCubeArray: return textureDesc.textureCube.layers;
        case TextureType::Texture2DMSArray: return textureDesc.texture2DMS.layers;
        default:                            return 1u;
    }
}

std::uint32_t TextureBufferSize(const TextureFormat textureFormat, std::uint32_t numTexels)
{
    switch (textureFormat)
    {
        /* --- Color formats --- */
        case TextureFormat::R8UNorm:            return numTexels;
        case TextureFormat::R8SNorm:            return numTexels;

        case TextureFormat::R16UNorm:           return numTexels * 2;
        case TextureFormat::R16SNorm:           return numTexels * 2;
        case TextureFormat::R16Float:           return numTexels * 2;

        case TextureFormat::R32UInt:            return numTexels * 4;
        case TextureFormat::R32SInt:            return numTexels * 4;
        case TextureFormat::R32Float:           return numTexels * 4;

        case TextureFormat::RG8UNorm:           return numTexels * 2;
        case TextureFormat::RG8SNorm:           return numTexels * 2;

        case TextureFormat::RG16UNorm:          return numTexels * 4;
        case TextureFormat::RG16SNorm:          return numTexels * 4;
        case TextureFormat::RG16Float:          return numTexels * 4;

        case TextureFormat::RG32UInt:           return numTexels * 8;
        case TextureFormat::RG32SInt:           return numTexels * 8;
        case TextureFormat::RG32Float:          return numTexels * 8;

        case TextureFormat::RGB8UNorm:          return numTexels * 3;
        case TextureFormat::RGB8SNorm:          return numTexels * 3;

        case TextureFormat::RGB16UNorm:         return numTexels * 6;
        case TextureFormat::RGB16SNorm:         return numTexels * 6;
        case TextureFormat::RGB16Float:         return numTexels * 6;

        case TextureFormat::RGB32UInt:          return numTexels * 12;
        case TextureFormat::RGB32SInt:          return numTexels * 12;
        case TextureFormat::RGB32Float:         return numTexels * 12;

        case TextureFormat::RGBA8UNorm:         return numTexels * 4;
        case TextureFormat::RGBA8SNorm:         return numTexels * 4;

        case TextureFormat::RGBA16UNorm:        return numTexels * 8;
        case TextureFormat::RGBA16SNorm:        return numTexels * 8;
        case TextureFormat::RGBA16Float:        return numTexels * 8;

        case TextureFormat::RGBA32UInt:         return numTexels * 16;
        case TextureFormat::RGBA32SInt:         return numTexels * 16;
        case TextureFormat::RGBA32Float:        return numTexels * 16;

        /* --- Depth-stencil formats --- */
        case TextureFormat::D32Float:           return numTexels * 4; // 32-bit depth
        case TextureFormat::D24UNormS8UInt:     return numTexels * 4; // 24-bit depth, 8-bit stencil

        /* --- Compressed color formats --- */
        case TextureFormat::BC1RGB:             return numTexels / 4; // 64-bit per 4x4 block
        case TextureFormat::BC1RGBA:            return numTexels / 4; // 64-bit per 4x4 block
        case TextureFormat::BC2RGBA:            return numTexels / 8; // 128-bit per 4x4 block
        case TextureFormat::BC3RGBA:            return numTexels / 8; // 128-bit per 4x4 block

        default:                                return 0;
    }
}

LLGL_EXPORT std::uint32_t TextureSize(const TextureDescriptor& textureDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:        return textureDesc.texture1D.width;
        case TextureType::Texture2D:        return textureDesc.texture2D.width * textureDesc.texture2D.height;
        case TextureType::Texture3D:        return textureDesc.texture3D.width * textureDesc.texture3D.height * textureDesc.texture3D.depth;
        case TextureType::TextureCube:      return textureDesc.textureCube.width * textureDesc.textureCube.height * 6;
        case TextureType::Texture1DArray:   return textureDesc.texture1D.width * textureDesc.texture1D.layers;
        case TextureType::Texture2DArray:   return textureDesc.texture2D.width * textureDesc.texture2D.height * textureDesc.texture2D.layers;
        case TextureType::TextureCubeArray: return textureDesc.textureCube.width * textureDesc.textureCube.height * 6 * textureDesc.textureCube.layers;
        case TextureType::Texture2DMS:      return textureDesc.texture2D.width * textureDesc.texture2D.height;
        case TextureType::Texture2DMSArray: return textureDesc.texture2D.width * textureDesc.texture2D.height * textureDesc.texture2D.layers;
        default:                            return 0;
    }
}

LLGL_EXPORT bool IsCompressedFormat(const TextureFormat format)
{
    return (format >= TextureFormat::BC1RGB);
}

LLGL_EXPORT bool IsDepthStencilFormat(const TextureFormat format)
{
    return (format == TextureFormat::D32Float || format == TextureFormat::D24UNormS8UInt);
}

LLGL_EXPORT bool IsArrayTexture(const TextureType type)
{
    return (type >= TextureType::Texture1DArray && type != TextureType::Texture2DMS);
}

LLGL_EXPORT bool IsMultiSampleTexture(const TextureType type)
{
    return (type >= TextureType::Texture2DMS);
}

LLGL_EXPORT bool IsCubeTexture(const TextureType type)
{
    return (type == TextureType::TextureCube || type == TextureType::TextureCubeArray);
}


} // /namespace LLGL



// ================================================================================
