/*
 * TextureUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TextureUtils.h"
#include <LLGL/StaticLimits.h>
#include "../Core/Helper.h"
#include "../Core/HelperMacros.h"


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

LLGL_EXPORT SubresourceLayout CalcSubresourceLayout(const Format format, const Extent3D& extent)
{
    const auto& formatDesc = GetFormatAttribs(format);
    SubresourceLayout layout;
    {
        layout.rowStride    = (extent.width * formatDesc.bitSize) / formatDesc.blockWidth / 8;
        layout.layerStride  = (extent.height * layout.rowStride) / formatDesc.blockHeight;
        layout.dataSize     = extent.depth * layout.layerStride;
    }
    return layout;
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
    return Clamp(samples, 1u, LLGL_MAX_NUM_SAMPLES);
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
    LLGL_COMPARE_MEMBER_SWO( base       );
    LLGL_COMPARE_MEMBER_SWO( firstMip   );
    LLGL_COMPARE_MEMBER_SWO( numLayers  );
    LLGL_COMPARE_MEMBER_SWO( firstLayer );
    return 0;
}


} // /namespace LLGL



// ================================================================================
