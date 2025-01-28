/*
 * TextureUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "TextureUtils.h"
#include <LLGL/Constants.h>
#include "../Core/CoreUtils.h"
#include "../Core/MacroUtils.h"
#include <cstring>


namespace LLGL
{


LLGL_EXPORT Offset3D CalcTextureOffset(const TextureType type, const Offset3D& offset, std::uint32_t baseArrayLayer)
{
    switch (type)
    {
        case TextureType::Texture1D:
            return Offset3D{ offset.x, 0, 0 };

        case TextureType::Texture1DArray:
            return Offset3D{ offset.x, static_cast<std::int32_t>(baseArrayLayer), 0 };

        case TextureType::Texture2D:
        case TextureType::Texture2DMS:
            return Offset3D{ offset.x, offset.y, 0 };

        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMSArray:
            return Offset3D{ offset.x, offset.y, static_cast<std::int32_t>(baseArrayLayer) };

        case TextureType::Texture3D:
            return offset;
    }
    return {};
}

LLGL_EXPORT Extent3D CalcTextureExtent(const TextureType type, const Extent3D& extent, std::uint32_t numArrayLayers)
{
    switch (type)
    {
        case TextureType::Texture1D:
            return Extent3D{ extent.width, 1u, 1u };

        case TextureType::Texture1DArray:
            return Extent3D{ extent.width, numArrayLayers, 1u };

        case TextureType::Texture2D:
        case TextureType::Texture2DMS:
            return Extent3D{ extent.width, extent.height, 1u };

        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMSArray:
            return Extent3D{ extent.width, extent.height, numArrayLayers };

        case TextureType::Texture3D:
            return extent;
    }
    return {};
}

static void CalcSubresourceLayoutPrimary(
    SubresourceLayout&  outLayout,
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       numArrayLayers)
{
    const FormatAttributes& formatAttribs = GetFormatAttribs(format);
    if (formatAttribs.blockWidth > 0 && formatAttribs.blockHeight > 0)
    {
        outLayout.rowStride         = (extent.width * formatAttribs.bitSize) / formatAttribs.blockWidth / 8;
        outLayout.layerStride       = (extent.height * outLayout.rowStride) / formatAttribs.blockHeight;
        outLayout.subresourceSize   = extent.depth * outLayout.layerStride * std::max(1u, numArrayLayers);
    }
}

LLGL_EXPORT SubresourceLayout CalcSubresourceLayout(const Format format, const Extent3D& extent, std::uint32_t numArrayLayers)
{
    SubresourceLayout layout;
    CalcSubresourceLayoutPrimary(layout, format, extent, numArrayLayers);
    return layout;
}

LLGL_EXPORT SubresourceCPUMappingLayout CalcSubresourceCPUMappingLayout(
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       numArrayLayers,
    const ImageFormat   imageFormat,
    const DataType      imageDataType)
{
    SubresourceCPUMappingLayout layout;
    {
        CalcSubresourceLayoutPrimary(layout, format, extent, numArrayLayers);
        layout.numTexelsPerLayer    = extent.width * extent.height * extent.depth;
        layout.numTexelsTotal       = layout.numTexelsPerLayer * numArrayLayers;
        layout.imageSize            = GetMemoryFootprint(imageFormat, imageDataType, layout.numTexelsTotal);
    }
    return layout;
}

LLGL_EXPORT SubresourceFootprint CalcPackedSubresourceFootprint(
    const TextureType   type,
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       mipLevel,
    std::uint32_t       numArrayLayers,
    std::uint32_t       alignment)
{
    SubresourceFootprint footprint;
    {
        const Extent3D      mipExtent = GetMipExtent(type, extent, mipLevel);
        const std::uint32_t numLayers = mipExtent.depth * numArrayLayers;
        footprint.rowAlignment  = alignment;
        footprint.rowSize       = static_cast<std::uint32_t>(GetMemoryFootprint(format, mipExtent.width));
        footprint.rowStride     = GetAlignedSize(footprint.rowSize, alignment);
        footprint.layerSize     = (mipExtent.height > 1 ? footprint.rowStride * (mipExtent.height - 1) + footprint.rowSize : footprint.rowSize * mipExtent.height);
        footprint.layerStride   = footprint.rowStride * mipExtent.height;
        footprint.size          = (numLayers > 1 ? footprint.layerStride * (numLayers - 1) + footprint.layerSize : footprint.layerSize * numLayers);
    }
    return footprint;
}

LLGL_EXPORT bool MustGenerateMipsOnCreate(const TextureDescriptor& textureDesc)
{
    return
    (
        NumMipLevels(textureDesc) > 1 &&
        (textureDesc.miscFlags & (MiscFlags::GenerateMips | MiscFlags::NoInitialData)) == MiscFlags::GenerateMips
    );
}

LLGL_EXPORT std::uint32_t GetClampedSamples(std::uint32_t samples)
{
    return std::max(1u, std::min(samples, LLGL_MAX_NUM_SAMPLES));
}

// Compresses the specified texture swizzle parameter into 3 bits
static std::uint32_t CompressTextureSwizzle3Bits(const TextureSwizzle swizzle)
{
    return ((static_cast<std::uint32_t>(swizzle) - static_cast<std::uint32_t>(TextureSwizzle::Zero)) & 0x7);
}

// Compresses the specified texture swizzle parameters into 12 bits
static std::uint32_t CompressTextureSwizzleRGBA12Bits(const TextureSwizzleRGBA& swizzle)
{
    return
    (
        (CompressTextureSwizzle3Bits(swizzle.r) << 9) |
        (CompressTextureSwizzle3Bits(swizzle.g) << 6) |
        (CompressTextureSwizzle3Bits(swizzle.b) << 3) |
        (CompressTextureSwizzle3Bits(swizzle.a)     )
    );
}

LLGL_EXPORT void CompressTextureViewDesc(CompressedTexView& dst, const TextureViewDescriptor& src)
{
    dst.type        = static_cast<std::uint32_t>(src.type);
    dst.format      = static_cast<std::uint32_t>(src.format);
    dst.numMips     = src.subresource.numMipLevels;
    dst.swizzle     = CompressTextureSwizzleRGBA12Bits(src.swizzle);
    dst.firstMip    = src.subresource.baseMipLevel;
    dst.numLayers   = src.subresource.numArrayLayers;
    dst.firstLayer  = src.subresource.baseArrayLayer;
}

LLGL_EXPORT int CompareCompressedTexViewSWO(const CompressedTexView& lhs, const CompressedTexView& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(CompressedTexView));
}


} // /namespace LLGL



// ================================================================================
