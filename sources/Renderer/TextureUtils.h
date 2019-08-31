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


/* ----- Functions ----- */

// Calculates the actual 3D dimensional offset for the specified texture type.
LLGL_EXPORT Offset3D CalcTextureOffset(const TextureType type, const Offset3D& offset, std::uint32_t arrayLayer);

// Returns true if the specified flags for texture creation require MIP-map generation at creation time.
inline bool FlagsRequireGenerateMips(long miscFlags)
{
    return ((miscFlags & (MiscFlags::GenerateMips | MiscFlags::NoInitialData)) == MiscFlags::GenerateMips);
}


} // /namespace LLGL


#endif



// ================================================================================
