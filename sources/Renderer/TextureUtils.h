/*
 * TextureUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_UTILS_H
#define LLGL_TEXTURE_UTILS_H


#include <LLGL/TextureFlags.h>


namespace LLGL
{


/* ----- Structures ----- */

// Subresource data size structure with stride per row, stride per array layer, and whole data size.
struct SubresourceLayout
{
    std::uint32_t rowStride     = 0;
    std::uint32_t layerStride   = 0;
    std::uint32_t dataSize      = 0;
};


/* ----- Functions ----- */

// Calculates the actual 3D dimensional offset for the specified texture type.
LLGL_EXPORT Offset3D CalcTextureOffset(const TextureType type, const Offset3D& offset, std::uint32_t arrayLayer);

// Calculates the size and strides for a subresource of the specified format and extent.
LLGL_EXPORT SubresourceLayout CalcSubresourceLayout(const Format format, const Extent3D& extent);

// Returns true if the specified flags for texture creation require MIP-map generation at creation time.
LLGL_EXPORT bool MustGenerateMipsOnCreate(const TextureDescriptor& textureDesc);


} // /namespace LLGL


#endif



// ================================================================================
