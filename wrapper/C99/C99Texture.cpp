/*
 * C99Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Texture.h>
#include <LLGL-C/Texture.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT LLGLTextureType llglGetTextureType(LLGLTexture texture)
{
    return static_cast<LLGLTextureType>(LLGL_PTR(Texture, texture)->GetType());
}

LLGL_C_EXPORT long llglGetTextureBindFlags(LLGLTexture texture)
{
    return LLGL_PTR(Texture, texture)->GetBindFlags();
}

LLGL_C_EXPORT void llglGetTextureDesc(LLGLTexture texture, LLGLTextureDescriptor* outDesc)
{
    LLGL_ASSERT_PTR(outDesc);
    const TextureDescriptor internalDesc = LLGL_PTR(Texture, texture)->GetDesc();
    *outDesc = *reinterpret_cast<const LLGLTextureDescriptor*>(&internalDesc);
}

LLGL_C_EXPORT LLGLFormat llglGetTextureFormat(LLGLTexture texture)
{
    return static_cast<LLGLFormat>(LLGL_PTR(Texture, texture)->GetFormat());
}

LLGL_C_EXPORT void llglGetTextureMipExtent(LLGLTexture texture, uint32_t mipLevel, LLGLExtent3D* outExtent)
{
    LLGL_ASSERT_PTR(outExtent);
    const Extent3D internalExtent = LLGL_PTR(Texture, texture)->GetMipExtent(mipLevel);
    *outExtent = *reinterpret_cast<const LLGLExtent3D*>(&internalExtent);
}

LLGL_C_EXPORT void llglGetTextureSubresourceFootprint(LLGLTexture texture, uint32_t mipLevel, LLGLSubresourceFootprint* outFootprint)
{
    LLGL_ASSERT_PTR(outFootprint);
    const SubresourceFootprint internalFootprint = LLGL_PTR(Texture, texture)->GetSubresourceFootprint(mipLevel);
    *outFootprint = *reinterpret_cast<const LLGLSubresourceFootprint*>(&internalFootprint);
}


// } /namespace LLGL



// ================================================================================
