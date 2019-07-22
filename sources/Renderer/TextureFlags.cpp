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
    if (textureDesc.mipLevels == 0)
    {
        switch (textureDesc.type)
        {
            case TextureType::Texture1D:        return NumMipLevels(textureDesc.extent.width);
            case TextureType::Texture2D:        return NumMipLevels(textureDesc.extent.width, textureDesc.extent.height);
            case TextureType::Texture3D:        return NumMipLevels(textureDesc.extent.width, textureDesc.extent.height, textureDesc.extent.depth);
            case TextureType::TextureCube:      return NumMipLevels(textureDesc.extent.width, textureDesc.extent.height);
            case TextureType::Texture1DArray:   return NumMipLevels(textureDesc.extent.width);
            case TextureType::Texture2DArray:   return NumMipLevels(textureDesc.extent.width, textureDesc.extent.height);
            case TextureType::TextureCubeArray: return NumMipLevels(textureDesc.extent.width, textureDesc.extent.height);
            case TextureType::Texture2DMS:      return 1u;
            case TextureType::Texture2DMSArray: return 1u;
        }
    }
    return textureDesc.mipLevels;
}

std::uint32_t TextureBufferSize(const Format format, std::uint32_t numTexels)
{
    return ((FormatBitSize(format) * numTexels) / 8);
}

LLGL_EXPORT std::uint32_t TextureSize(const TextureDescriptor& textureDesc)
{
    const auto& extent = textureDesc.extent;
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:        return extent.width;
        case TextureType::Texture2D:        return extent.width * extent.height;
        case TextureType::Texture3D:        return extent.width * extent.height * extent.depth;
        case TextureType::TextureCube:      return extent.width * extent.height * textureDesc.arrayLayers;
        case TextureType::Texture1DArray:   return extent.width * textureDesc.arrayLayers;
        case TextureType::Texture2DArray:   return extent.width * extent.height * textureDesc.arrayLayers;
        case TextureType::TextureCubeArray: return extent.width * extent.height * textureDesc.arrayLayers;
        case TextureType::Texture2DMS:      return extent.width * extent.height;
        case TextureType::Texture2DMSArray: return extent.width * extent.height * textureDesc.arrayLayers;
        default:                            return 0;
    }
}

LLGL_EXPORT bool IsMipMappedTexture(const TextureDescriptor& textureDesc)
{
    return (!IsMultiSampleTexture(textureDesc.type) && (textureDesc.mipLevels == 0 || textureDesc.mipLevels > 1));
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

LLGL_EXPORT bool IsTextureSwizzleIdentity(const TextureSwizzleRGBA& swizzle)
{
    return
    (
        swizzle.r == TextureSwizzle::Red    &&
        swizzle.g == TextureSwizzle::Green  &&
        swizzle.b == TextureSwizzle::Blue   &&
        swizzle.a == TextureSwizzle::Alpha
    );
}


} // /namespace LLGL



// ================================================================================
