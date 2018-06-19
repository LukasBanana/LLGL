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

std::uint32_t TextureBufferSize(const Format textureFormat, std::uint32_t numTexels)
{
    switch (textureFormat)
    {
        /* --- Color formats --- */
        case Format::R8UNorm:           return numTexels;
        case Format::R8SNorm:           return numTexels;

        case Format::R16UNorm:          return numTexels * 2;
        case Format::R16SNorm:          return numTexels * 2;
        case Format::R16Float:          return numTexels * 2;

        case Format::R32UInt:           return numTexels * 4;
        case Format::R32SInt:           return numTexels * 4;
        case Format::R32Float:          return numTexels * 4;

        case Format::RG8UNorm:          return numTexels * 2;
        case Format::RG8SNorm:          return numTexels * 2;

        case Format::RG16UNorm:         return numTexels * 4;
        case Format::RG16SNorm:         return numTexels * 4;
        case Format::RG16Float:         return numTexels * 4;

        case Format::RG32UInt:          return numTexels * 8;
        case Format::RG32SInt:          return numTexels * 8;
        case Format::RG32Float:         return numTexels * 8;

        case Format::RGB8UNorm:         return numTexels * 3;
        case Format::RGB8SNorm:         return numTexels * 3;

        case Format::RGB16UNorm:        return numTexels * 6;
        case Format::RGB16SNorm:        return numTexels * 6;
        case Format::RGB16Float:        return numTexels * 6;

        case Format::RGB32UInt:         return numTexels * 12;
        case Format::RGB32SInt:         return numTexels * 12;
        case Format::RGB32Float:        return numTexels * 12;

        case Format::RGBA8UNorm:        return numTexels * 4;
        case Format::RGBA8SNorm:        return numTexels * 4;

        case Format::RGBA16UNorm:       return numTexels * 8;
        case Format::RGBA16SNorm:       return numTexels * 8;
        case Format::RGBA16Float:       return numTexels * 8;

        case Format::RGBA32UInt:        return numTexels * 16;
        case Format::RGBA32SInt:        return numTexels * 16;
        case Format::RGBA32Float:       return numTexels * 16;

        /* --- Depth-stencil formats --- */
        case Format::D32Float:          return numTexels * 4; // 32-bit depth
        case Format::D24UNormS8UInt:    return numTexels * 4; // 24-bit depth, 8-bit stencil

        /* --- Compressed color formats --- */
        case Format::BC1RGB:            return numTexels / 4; // 64-bit per 4x4 block
        case Format::BC1RGBA:           return numTexels / 4; // 64-bit per 4x4 block
        case Format::BC2RGBA:           return numTexels / 8; // 128-bit per 4x4 block
        case Format::BC3RGBA:           return numTexels / 8; // 128-bit per 4x4 block

        default:                        return 0;
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
