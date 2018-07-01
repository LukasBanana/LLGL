/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
#include "RenderTargetFlags.h"
#include "ResourceHeapFlags.h"
#include "ShaderFlags.h"


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
LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture2D type.
LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture3D type.
LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::TextureCube type.
LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t layers, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long flags = TextureFlags::Default);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, std::uint32_t samples, long flags = TextureFlags::Default);

/* ----- BufferDescriptor utility functions ----- */

//! Returns a BufferDescriptor structure for a vertex buffer.
LLGL_EXPORT BufferDescriptor VertexBufferDesc(uint64_t size, const VertexFormat& vertexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for an index buffer.
LLGL_EXPORT BufferDescriptor IndexBufferDesc(uint64_t size, const IndexFormat& indexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for a constant buffer.
LLGL_EXPORT BufferDescriptor ConstantBufferDesc(uint64_t size, long flags = BufferFlags::DynamicUsage);

//! Returns a BufferDescriptor structure for a storage buffer.
LLGL_EXPORT BufferDescriptor StorageBufferDesc(uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long flags = BufferFlags::MapReadAccess | BufferFlags::MapWriteAccess);

/* ----- ShaderDescriptor utility functions ----- */

/**
\brief Returns a ShaderDescriptor structure.
\remarks The source type is determined by the filename extension using the following rules:
- .hlsl, .fx, .glsl, .vert, .tesc, .tese, .geom, .frag, .comp ==> code file (i.e. ShaderSourceType::CodeFile)
- Otherwise ==> binary file (i.e. ShaderSourceType::BinaryFile).
*/
LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint = nullptr, const char* profile = nullptr, long flags = 0);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
