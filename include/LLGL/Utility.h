/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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


namespace LLGL
{


/**
\defgroup group_util Global utility functions, especially to fill descriptor structures.
\addtogroup group_util
@{
*/

/* ----- TextureDescriptor utility functions ----- */

//! Returns a TextureDescriptor structure with the TextureType::Texture1D type.
LLGL_EXPORT TextureDescriptor Texture1DDesc(TextureFormat format, unsigned int width);

//! Returns a TextureDescriptor structure with the TextureType::Texture2D type.
LLGL_EXPORT TextureDescriptor Texture2DDesc(TextureFormat format, unsigned int width, unsigned int height);

//! Returns a TextureDescriptor structure with the TextureType::Texture3D type.
LLGL_EXPORT TextureDescriptor Texture3DDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int depth);

//! Returns a TextureDescriptor structure with the TextureType::TextureCube type.
LLGL_EXPORT TextureDescriptor TextureCubeDesc(TextureFormat format, unsigned int width, unsigned int height);

//! Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(TextureFormat format, unsigned int width, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int samples, bool fixedSamples = true);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(TextureFormat format, unsigned int width, unsigned int height, unsigned int layers, unsigned int samples, bool fixedSamples = true);

/* ----- BufferDescriptor utility functions ----- */

//! Returns a BufferDescriptor structure for a vertex buffer.
LLGL_EXPORT BufferDescriptor VertexBufferDesc(unsigned int size, const VertexFormat& vertexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for an index buffer.
LLGL_EXPORT BufferDescriptor IndexBufferDesc(unsigned int size, const IndexFormat& indexFormat, long flags = 0);

//! Returns a BufferDescriptor structure for a constant buffer.
LLGL_EXPORT BufferDescriptor ConstantBufferDesc(unsigned int size, long flags = BufferFlags::DynamicUsage);

//! Returns a BufferDescriptor structure for a storage buffer.
LLGL_EXPORT BufferDescriptor StorageBufferDesc(unsigned int size, const StorageBufferType storageType, unsigned int stride, long flags = BufferFlags::MapReadAccess | BufferFlags::MapWriteAccess);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
