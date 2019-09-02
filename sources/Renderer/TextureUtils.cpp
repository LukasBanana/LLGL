/*
 * TextureUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TextureUtils.h"


namespace LLGL
{


LLGL_EXPORT Offset3D CalcTextureOffset(const TextureType type, const Offset3D& offset, std::uint32_t arrayLayer)
{
    switch (type)
    {
        case TextureType::Texture1D:
            return Offset3D{ offset.x, 0, 0 };
        case TextureType::Texture1DArray:
            return Offset3D{ offset.x, static_cast<std::int32_t>(arrayLayer), 0 };
        case TextureType::Texture2D:
        case TextureType::Texture2DMS:
            return Offset3D{ offset.x, offset.y, 0 };
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMSArray:
            return Offset3D{ offset.x, offset.y, static_cast<std::int32_t>(arrayLayer) };
        case TextureType::Texture3D:
            return offset;
        default:
            return Offset3D{};
    }
}

LLGL_EXPORT SubresourceLayout CalcSubresourceLayout(const Format format, const Extent3D& extent)
{
    const auto& formatDesc = GetFormatDesc(format);
    SubresourceLayout layout;
    {
        layout.rowStride    = (extent.width * formatDesc.bitSize) / formatDesc.blockWidth / 8;
        layout.layerStride  = (extent.height * layout.rowStride) / formatDesc.blockHeight;
        layout.dataSize     = extent.depth * layout.layerStride;
    }
    return layout;
}


} // /namespace LLGL



// ================================================================================
