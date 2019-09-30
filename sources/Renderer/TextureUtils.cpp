/*
 * TextureUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TextureUtils.h"
#include <LLGL/StaticLimits.h>
#include "../Core/Helper.h"


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


} // /namespace LLGL



// ================================================================================
