/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_UTILITY_H
#define LLGL_UTILITY_H

#ifdef LLGL_ENABLE_UTILITY

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include "Export.h"
#include "TextureFlags.h"
#include "BufferFlags.h"
#include "ResourceViewHeapFlags.h"


namespace LLGL
{


class Buffer;
class Texture;
class Sampler;

/**
\defgroup group_util Global utility functions, especially to fill descriptor structures.
\addtogroup group_util
@{
*/

/* ----- TextureDescriptor utility functions ----- */

//! Returns a TextureDescriptor structure with the TextureType::Texture1D type.
LLGL_EXPORT TextureDescriptor Texture1DDesc(TextureFormat format, std::uint32_t width, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture2D type.
LLGL_EXPORT TextureDescriptor Texture2DDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture3D type.
LLGL_EXPORT TextureDescriptor Texture3DDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::TextureCube type.
LLGL_EXPORT TextureDescriptor TextureCubeDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t layers, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, bool fixedSamples = true, long flags = TextureFlags::GenerateMips);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(TextureFormat format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, std::uint32_t samples, bool fixedSamples = true, long flags = TextureFlags::GenerateMips);

/* ----- BufferDescriptor utility functions ----- */

//! Returns a BufferDescriptor structure for a vertex buffer.
LLGL_EXPORT BufferDescriptor VertexBufferDesc(uint64_t size, const VertexFormat& vertexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for an index buffer.
LLGL_EXPORT BufferDescriptor IndexBufferDesc(uint64_t size, const IndexFormat& indexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for a constant buffer.
LLGL_EXPORT BufferDescriptor ConstantBufferDesc(uint64_t size, long flags = BufferFlags::DynamicUsage);

//! Returns a BufferDescriptor structure for a storage buffer.
LLGL_EXPORT BufferDescriptor StorageBufferDesc(uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long flags = BufferFlags::MapReadAccess | BufferFlags::MapWriteAccess);

/* ----- ResourceViewDescriptor utility functions ----- */

//! Returns a ResourceViewDescriptor structure for the specified buffer.
LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Buffer* buffer);

//! Returns a ResourceViewDescriptor structure for the specified texture.
LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Texture* texture);

//! Returns a ResourceViewDescriptor structure for the specified sampler.
LLGL_EXPORT ResourceViewDescriptor ResourceViewDesc(Sampler* sampler);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
