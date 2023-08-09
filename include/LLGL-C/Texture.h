/*
 * Texture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_TEXTURE_H
#define LLGL_C99_TEXTURE_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT LLGLTextureType llglGetTextureType(LLGLTexture texture);
LLGL_C_EXPORT long llglGetTextureBindFlags(LLGLTexture texture);
LLGL_C_EXPORT void llglGetTextureDesc(LLGLTexture texture, LLGLTextureDescriptor* outDesc);
LLGL_C_EXPORT LLGLFormat llglGetTextureFormat(LLGLTexture texture);
LLGL_C_EXPORT void llglGetTextureMipExtent(LLGLTexture texture, uint32_t mipLevel, LLGLExtent3D* outExtent);
LLGL_C_EXPORT void llglGetTextureSubresourceFootprint(LLGLTexture texture, uint32_t mipLevel, LLGLSubresourceFootprint* outFootprint);


#endif



// ================================================================================
