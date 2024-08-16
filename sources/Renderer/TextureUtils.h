/*
 * TextureUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TEXTURE_UTILS_H
#define LLGL_TEXTURE_UTILS_H


#include <LLGL/TextureFlags.h>


namespace LLGL
{


/* ----- Structures ----- */

// Subresource layout structure with stride per row, stride per array layer, and whole data size.
struct SubresourceLayout
{
    std::uint32_t rowStride         = 0; // Bytes per row
    std::uint32_t layerStride       = 0; // Bytes per layer
    std::uint32_t subresourceSize   = 0; // Bytes per resource
};

// Subresource layout structure for CPU-GPU mapping when initializing a texture with CPU image data or reading from a CPU texture.
struct SubresourceCPUMappingLayout : SubresourceLayout
{
    std::uint32_t   numTexelsPerLayer   = 0; // Number of texture elements per layer
    std::uint32_t   numTexelsTotal      = 0; // Total number of texture elements for the respective subresource (i.e. for a single MIP-map).
    std::size_t     imageSize           = 0; // Required image size to read from or write to CPU image data.
};

// Compressed version of <TextureViewDescriptor> structure for faster insertion sort.
struct CompressedTexView
{
    std::uint32_t type      : 4;
    std::uint32_t format    : 8;
    std::uint32_t numMips   : 8;
    std::uint32_t swizzle   : 12;
    std::uint32_t firstMip;
    std::uint32_t numLayers;
    std::uint32_t firstLayer;
};


/* ----- Functions ----- */

// Calculates the actual 3D offset for the specified texture type.
LLGL_EXPORT Offset3D CalcTextureOffset(const TextureType type, const Offset3D& offset, std::uint32_t baseArrayLayer = 0);

// Calculates the actual 3D extent for the specified texture and range of array layers.
LLGL_EXPORT Extent3D CalcTextureExtent(const TextureType type, const Extent3D& extent, std::uint32_t numArrayLayers = 1);

// Calculates the size and strides for a subresource of the specified format and extent.
LLGL_EXPORT SubresourceLayout CalcSubresourceLayout(const Format format, const Extent3D& extent, std::uint32_t numArrayLayers = 1);

// Calculates the required sizes and strides for a subresource when mapped between GPU and CPU.
LLGL_EXPORT SubresourceCPUMappingLayout CalcSubresourceCPUMappingLayout(
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       numArrayLayers,
    const ImageFormat   imageFormat,
    const DataType      imageDataType
);

// Calculates the required sizes and strides for a subresource when mapped between GPU and CPU.
inline SubresourceCPUMappingLayout CalcSubresourceCPUMappingLayout(
    const Format            format,
    const TextureRegion&    textureRegion,
    const ImageFormat       imageFormat,
    const DataType          imageDataType)
{
    return CalcSubresourceCPUMappingLayout(format, textureRegion.extent, textureRegion.subresource.numArrayLayers, imageFormat, imageDataType);
}

// Calculates the subresource footprint for a tightly packed texture object. This is the default implementation of Texture::GetSubresourceFootprint().
LLGL_EXPORT SubresourceFootprint CalcPackedSubresourceFootprint(
    const TextureType   type,
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       mipLevel,
    std::uint32_t       numArrayLayers,
    std::uint32_t       alignment = 1
);

// Returns true if the specified flags for texture creation require MIP-map generation at creation time.
LLGL_EXPORT bool MustGenerateMipsOnCreate(const TextureDescriptor& textureDesc);

// Returns the samples clamped to the range [1, LLGL_MAX_NUM_SAMPLES].
LLGL_EXPORT std::uint32_t GetClampedSamples(std::uint32_t samples);

// Converts the source texture-view descriptor into a compressed version.
LLGL_EXPORT void CompressTextureViewDesc(CompressedTexView& dst, const TextureViewDescriptor& src);

// Compares the two texture views in a strict-weak-order (SWO).
LLGL_EXPORT int CompareCompressedTexViewSWO(const CompressedTexView& lhs, const CompressedTexView& rhs);

// Returns true if the texture-view in the specified resource-view descriptor is enabled.
inline bool IsTextureViewEnabled(const TextureViewDescriptor& textureViewDesc)
{
    return
    (
        textureViewDesc.format != Format::Undefined    &&
        textureViewDesc.subresource.numMipLevels   > 0 &&
        textureViewDesc.subresource.numArrayLayers > 0
    );
}


} // /namespace LLGL


#endif



// ================================================================================
