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
            case TextureType::Texture1D:        return NumMipLevels(textureDesc.width);
            case TextureType::Texture2D:        return NumMipLevels(textureDesc.width, textureDesc.height);
            case TextureType::Texture3D:        return NumMipLevels(textureDesc.width, textureDesc.height, textureDesc.depth);
            case TextureType::TextureCube:      return NumMipLevels(textureDesc.width, textureDesc.height);
            case TextureType::Texture1DArray:   return NumMipLevels(textureDesc.width);
            case TextureType::Texture2DArray:   return NumMipLevels(textureDesc.width, textureDesc.height);
            case TextureType::TextureCubeArray: return NumMipLevels(textureDesc.width, textureDesc.height);
            case TextureType::Texture2DMS:      return 1u;
            case TextureType::Texture2DMSArray: return 1u;
        }
    }
    return 1u;
}

std::uint32_t TextureBufferSize(const Format format, std::uint32_t numTexels)
{
    return ((FormatBitSize(format) * numTexels) / 8);
}

LLGL_EXPORT std::uint32_t TextureSize(const TextureDescriptor& textureDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:        return textureDesc.width;
        case TextureType::Texture2D:        return textureDesc.width * textureDesc.height;
        case TextureType::Texture3D:        return textureDesc.width * textureDesc.height * textureDesc.depth;
        case TextureType::TextureCube:      return textureDesc.width * textureDesc.height * 6;
        case TextureType::Texture1DArray:   return textureDesc.width * textureDesc.layers;
        case TextureType::Texture2DArray:   return textureDesc.width * textureDesc.height * textureDesc.layers;
        case TextureType::TextureCubeArray: return textureDesc.width * textureDesc.height * 6 * textureDesc.layers;
        case TextureType::Texture2DMS:      return textureDesc.width * textureDesc.height;
        case TextureType::Texture2DMSArray: return textureDesc.width * textureDesc.height * textureDesc.layers;
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
