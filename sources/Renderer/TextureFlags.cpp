/*
 * TextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/TextureFlags.h>
#include "TextureUtils.h"
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
    auto mipExtent = GetMipExtent(type, extent, mipLevel);
    return (mipExtent.width * mipExtent.height * mipExtent.depth);
}

LLGL_EXPORT std::uint32_t NumMipTexels(const TextureType type, const Extent3D& extent, const TextureSubresource& subresource)
{
    std::uint32_t numTexels = 0;

    const auto subresourceExtent = CalcTextureExtent(type, extent, subresource.numArrayLayers);
    for (std::uint32_t mipLevel = 0; mipLevel < subresource.numMipLevels; ++mipLevel)
        numTexels += NumMipTexels(type, subresourceExtent, subresource.baseMipLevel + mipLevel);

    return numTexels;
}

LLGL_EXPORT std::uint32_t NumMipTexels(const TextureDescriptor& textureDesc, std::uint32_t mipLevel)
{
    const auto extent = CalcTextureExtent(textureDesc.type, textureDesc.extent, textureDesc.arrayLayers);

    if (mipLevel == ~0u)
    {
        std::uint32_t numTexels = 0;

        const auto numMipLevels = NumMipLevels(textureDesc);
        for (mipLevel = 0; mipLevel < numMipLevels; ++mipLevel)
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

LLGL_EXPORT Extent3D GetMipExtent(const TextureType type, const Extent3D& e, std::uint32_t m)
{
    if (m < NumMipLevels(type, e))
    {
        switch (type)
        {
            case TextureType::Texture1D:        return Extent3D{ MipExtent(e.width, m), 1u, 1u };
            case TextureType::Texture2D:        return Extent3D{ MipExtent(e.width, m), MipExtent(e.height, m), 1u };
            case TextureType::Texture3D:        return Extent3D{ MipExtent(e.width, m), MipExtent(e.height, m), MipExtent(e.depth, m) };
            case TextureType::TextureCube:      return Extent3D{ MipExtent(e.width, m), MipExtent(e.height, m), 1u };
            case TextureType::Texture1DArray:   return Extent3D{ MipExtent(e.width, m), e.height, 1u };
            case TextureType::Texture2DArray:   return Extent3D{ MipExtent(e.width, m), MipExtent(e.height, m), e.depth };
            case TextureType::TextureCubeArray: return Extent3D{ MipExtent(e.width, m), MipExtent(e.height, m), e.depth };
            case TextureType::Texture2DMS:      return Extent3D{ e.width, e.height, 1u };
            case TextureType::Texture2DMSArray: return Extent3D{ e.width, e.height, e.depth };
        }
    }
    return {};
}

std::uint32_t GetMemoryFootprint(const TextureType type, const Format format, const Extent3D& extent, const TextureSubresource& subresource)
{
    const auto numTexels = NumMipTexels(type, extent, subresource);
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
