/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_UTILITY_H__
#define __LLGL_UTILITY_H__

#ifdef LLGL_ENABLE_UTILITY

/*
THIS HEADER MUST BE EXPLICITLY INCLUDED
*/

#include "Export.h"
#include "TextureFlags.h"


namespace LLGL
{


/**
\defgroup group_util Global utility functions, especially to fill descriptor structures.
\addtogroup group_util
@{
*/

//! Returns a TextureDescriptor structure with the TextureType::Texture1D type.
LLGL_EXPORT TextureDescriptor Texture1DDesc(const TextureFormat format, unsigned int width);

//! Returns a TextureDescriptor structure with the TextureType::Texture2D type.
LLGL_EXPORT TextureDescriptor Texture2DDesc(const TextureFormat format, unsigned int width, unsigned int height);

//! Returns a TextureDescriptor structure with the TextureType::Texture3D type.
LLGL_EXPORT TextureDescriptor Texture3DDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int depth);

//! Returns a TextureDescriptor structure with the TextureType::TextureCube type.
LLGL_EXPORT TextureDescriptor TextureCubeDesc(const TextureFormat format, unsigned int width, unsigned int height);

//! Returns a TextureDescriptor structure with the TextureType::Texture1DArray type.
LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(const TextureFormat format, unsigned int width, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DArray type.
LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::TextureCubeArray type.
LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMS type.
LLGL_EXPORT TextureDescriptor Texture2DMSDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int samples, bool fixedSamples = true);

//! Returns a TextureDescriptor structure with the TextureType::Texture2DMSArray type.
LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(const TextureFormat format, unsigned int width, unsigned int height, unsigned int layers, unsigned int samples, bool fixedSamples = true);

/** @} */


} // /namespace LLGL


#else

#error LLGL was not compiled with LLGL_ENABLE_UTILITY option

#endif

#endif



// ================================================================================
