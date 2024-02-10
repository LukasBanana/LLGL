/*
 * TextureFlags.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/TextureFlags.h>
#include <LLGL/Utils/ForRange.h>
#include "TextureUtils.h"
#include "../Core/CoreUtils.h"
#include <algorithm>
#include <cmath>


namespace LLGL
{


LLGL_EXPORT std::uint32_t NumMipLevels(std::uint32_t width, std::uint32_t height, std::uint32_t depth)
{
    const auto maxSize = std::max({ width, height, depth });
    const auto log2Size = static_cast<std::uint32_t>(std::log2(maxSize));
    return (1u + log2Size);
}

LLGL_EXPORT std::uint32_t NumMipLevels(const TextureType type, const Extent3D& extent)
{
    switch (type)
    {
        case TextureType::Texture1D:        return NumMipLevels(extent.width);
        case TextureType::Texture2D:        return NumMipLevels(extent.width, extent.height);
        case TextureType::Texture3D:        return NumMipLevels(extent.width, extent.height, extent.depth);
        case TextureType::TextureCube:      return NumMipLevels(extent.width, extent.height);
        case TextureType::Texture1DArray:   return NumMipLevels(extent.width);
        case TextureType::Texture2DArray:   return NumMipLevels(extent.width, extent.height);
        case TextureType::TextureCubeArray: return NumMipLevels(extent.width, extent.height);
        case TextureType::Texture2DMS:      return 1u;
        case TextureType::Texture2DMSArray: return 1u;
    }
    return 0u;
}

LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc)
{
    if (textureDesc.mipLevels == 0)
        return NumMipLevels(textureDesc.type, textureDesc.extent);
    else
        return textureDesc.mipLevels;
}

LLGL_EXPORT std::uint32_t NumMipTexels(const TextureType type, const Extent3D& extent, std::uint32_t mipLevel)
{
    const Extent3D mipExtent = GetMipExtent(type, extent, mipLevel);
    return (mipExtent.width * mipExtent.height * mipExtent.depth);
}

LLGL_EXPORT std::uint32_t NumMipTexels(const TextureType type, const Extent3D& extent, const TextureSubresource& subresource)
{
    std::uint32_t numTexels = 0;

    const Extent3D subresourceExtent = CalcTextureExtent(type, extent, subresource.numArrayLayers);
    for_range(mipLevel, subresource.numMipLevels)
        numTexels += NumMipTexels(type, subresourceExtent, subresource.baseMipLevel + mipLevel);

    return numTexels;
}

LLGL_EXPORT std::uint32_t NumMipTexels(const TextureDescriptor& textureDesc, std::uint32_t mipLevel)
{
    const Extent3D extent = CalcTextureExtent(textureDesc.type, textureDesc.extent, textureDesc.arrayLayers);

    if (mipLevel == ~0u)
    {
        std::uint32_t numTexels = 0;

        for_range(mipLevel, NumMipLevels(textureDesc))
            numTexels += NumMipTexels(textureDesc.type, extent, mipLevel);

        return numTexels;
    }

    return NumMipTexels(textureDesc.type, extent, mipLevel);
}

LLGL_EXPORT std::uint32_t NumMipDimensions(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return 1;
        case TextureType::Texture2D:        return 2;
        case TextureType::Texture3D:        return 3;
        case TextureType::TextureCube:      return 2;
        case TextureType::Texture1DArray:   return 2; // Array layer +1 dimension
        case TextureType::Texture2DArray:   return 3; // Array layer +1 dimension
        case TextureType::TextureCubeArray: return 3; // Array layer +1 dimension
        case TextureType::Texture2DMS:      return 2;
        case TextureType::Texture2DMSArray: return 3; // Array layer +1 dimension
    }
    return 0;
}

LLGL_EXPORT std::uint32_t NumTextureDimensions(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return 1;
        case TextureType::Texture2D:        return 2;
        case TextureType::Texture3D:        return 3;
        case TextureType::TextureCube:      return 2;
        case TextureType::Texture1DArray:   return 1;
        case TextureType::Texture2DArray:   return 2;
        case TextureType::TextureCubeArray: return 2;
        case TextureType::Texture2DMS:      return 2;
        case TextureType::Texture2DMSArray: return 2;
    }
    return 0;
}

// Returns the 1D extent for the specified MIP-map
static std::uint32_t MipExtent(std::uint32_t extent, std::uint32_t mipLevel)
{
    return std::max(1u, extent >> mipLevel);
}

LLGL_EXPORT Extent3D GetMipExtent(const TextureType type, const Extent3D& extent, std::uint32_t mipLevel)
{
    if (mipLevel < NumMipLevels(type, extent))
    {
        switch (type)
        {
            case TextureType::Texture1D:        return Extent3D{ MipExtent(extent.width, mipLevel), 1u, 1u };
            case TextureType::Texture2D:        return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), 1u };
            case TextureType::Texture3D:        return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), MipExtent(extent.depth, mipLevel) };
            case TextureType::TextureCube:      return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), extent.depth };
            case TextureType::Texture1DArray:   return Extent3D{ MipExtent(extent.width, mipLevel), extent.height, 1u };
            case TextureType::Texture2DArray:   return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), extent.depth };
            case TextureType::TextureCubeArray: return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), extent.depth };
            case TextureType::Texture2DMS:      return Extent3D{ extent.width, extent.height, 1u };
            case TextureType::Texture2DMSArray: return Extent3D{ extent.width, extent.height, extent.depth };
        }
    }
    return {};
}

LLGL_EXPORT Extent3D GetMipExtent(const TextureDescriptor& textureDesc, std::uint32_t mipLevel)
{
    const auto& extent = textureDesc.extent;
    if (mipLevel < NumMipLevels(textureDesc.type, extent))
    {
        const auto arrayLayers = textureDesc.arrayLayers;
        switch (textureDesc.type)
        {
            case TextureType::Texture1D:        return Extent3D{ MipExtent(extent.width, mipLevel), 1u, 1u };
            case TextureType::Texture2D:        return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), 1u };
            case TextureType::Texture3D:        return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), MipExtent(extent.depth, mipLevel) };
            case TextureType::TextureCube:      return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), 6u };
            case TextureType::Texture1DArray:   return Extent3D{ MipExtent(extent.width, mipLevel), arrayLayers, 1u };
            case TextureType::Texture2DArray:   return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), arrayLayers };
            case TextureType::TextureCubeArray: return Extent3D{ MipExtent(extent.width, mipLevel), MipExtent(extent.height, mipLevel), GetAlignedSize(arrayLayers, 6u) };
            case TextureType::Texture2DMS:      return Extent3D{ extent.width, extent.height, 1u };
            case TextureType::Texture2DMSArray: return Extent3D{ extent.width, extent.height, arrayLayers };
        }
    }
    return {};
}

LLGL_EXPORT std::size_t GetMemoryFootprint(const TextureType type, const Format format, const Extent3D& extent, const TextureSubresource& subresource)
{
    const std::uint32_t numTexels = NumMipTexels(type, extent, subresource);
    return GetMemoryFootprint(format, numTexels);
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
