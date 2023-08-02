/*
 * TextureFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_TEXTURE_FLAGS_H
#define LLGL_C99_TEXTURE_FLAGS_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Format.h>
#include <LLGL-C/CommandBufferFlags.h>
#include <stddef.h>
#include <stdint.h>


/* ----- Enumerations ----- */

typedef enum LLGLTextureType
{
    LLGLTextureTypeTexture1D,
    LLGLTextureTypeTexture2D,
    LLGLTextureTypeTexture3D,
    LLGLTextureTypeTextureCube,
    LLGLTextureTypeTexture1DArray,
    LLGLTextureTypeTexture2DArray,
    LLGLTextureTypeTextureCubeArray,
    LLGLTextureTypeTexture2DMS,
    LLGLTextureTypeTexture2DMSArray,
}
LLGLTextureType;

typedef enum LLGLTextureSwizzle
{
    LLGLTextureSwizzleZero,
    LLGLTextureSwizzleOne,
    LLGLTextureSwizzleRed,
    LLGLTextureSwizzleGreen,
    LLGLTextureSwizzleBlue,
    LLGLTextureSwizzleAlpha,
}
LLGLTextureSwizzle;


/* ----- Structures ----- */

typedef struct LLGLTextureSwizzleRGBA
{
    LLGLTextureSwizzle r : 8;
    LLGLTextureSwizzle g : 8;
    LLGLTextureSwizzle b : 8;
    LLGLTextureSwizzle a : 8;
}
LLGLTextureSwizzleRGBA;

typedef struct LLGLTextureSubresource
{
    uint32_t baseArrayLayer;
    uint32_t numArrayLayers;
    uint32_t baseMipLevel;
    uint32_t numMipLevels;
}
LLGLTextureSubresource;

typedef struct LLGLTextureLocation
{
    LLGLOffset3D    offset;
    uint32_t        arrayLayer;
    uint32_t        mipLevel;
}
LLGLTextureLocation;

typedef struct LLGLTextureRegion
{
    LLGLTextureSubresource  subresource;
    LLGLOffset3D            offset;
    LLGLExtent3D            extent;
}
LLGLTextureRegion;

typedef struct LLGLTextureDescriptor
{
    LLGLTextureType type;
    long            bindFlags;
    long            miscFlags;
    LLGLFormat      format;
    LLGLExtent3D    extent;
    uint32_t        arrayLayers;
    uint32_t        mipLevels;
    uint32_t        samples;
    LLGLClearValue  clearValue;
}
LLGLTextureDescriptor;

typedef struct LLGLTextureViewDescriptor
{
    LLGLTextureType         type;
    LLGLFormat              format;
    LLGLTextureSubresource  subresource;
    LLGLTextureSwizzleRGBA  swizzle;
}
LLGLTextureViewDescriptor;

typedef struct LLGLSubresourceFootprint
{
    uint64_t size;
    uint32_t rowAlignment;
    uint32_t rowSize;
    uint32_t rowStride;
    uint32_t layerSize;
    uint32_t layerStride;
}
LLGLSubresourceFootprint;


#endif



// ================================================================================
