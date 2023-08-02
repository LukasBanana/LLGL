/*
 * Format.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_FORMAT_H
#define LLGL_C99_FORMAT_H


#include <LLGL-C/Export.h>
#include <stdint.h>


/* ----- Enumerations ----- */

typedef enum LLGLFormat
{
    LLGLFormatUndefined,

    /* --- Alpha channel color formats --- */
    LLGLFormatA8UNorm,

    /* --- Red channel color formats --- */
    LLGLFormatR8UNorm,
    LLGLFormatR8SNorm,
    LLGLFormatR8UInt,
    LLGLFormatR8SInt,

    LLGLFormatR16UNorm,
    LLGLFormatR16SNorm,
    LLGLFormatR16UInt,
    LLGLFormatR16SInt,
    LLGLFormatR16Float,

    LLGLFormatR32UInt,
    LLGLFormatR32SInt,
    LLGLFormatR32Float,

    LLGLFormatR64Float,

    /* --- RG color formats --- */
    LLGLFormatRG8UNorm,
    LLGLFormatRG8SNorm,
    LLGLFormatRG8UInt,
    LLGLFormatRG8SInt,

    LLGLFormatRG16UNorm,
    LLGLFormatRG16SNorm,
    LLGLFormatRG16UInt,
    LLGLFormatRG16SInt,
    LLGLFormatRG16Float,

    LLGLFormatRG32UInt,
    LLGLFormatRG32SInt,
    LLGLFormatRG32Float,

    LLGLFormatRG64Float,

    /* --- RGB color formats --- */
    LLGLFormatRGB8UNorm,
    LLGLFormatRGB8UNorm_sRGB,
    LLGLFormatRGB8SNorm,
    LLGLFormatRGB8UInt,
    LLGLFormatRGB8SInt,

    LLGLFormatRGB16UNorm,
    LLGLFormatRGB16SNorm,
    LLGLFormatRGB16UInt,
    LLGLFormatRGB16SInt,
    LLGLFormatRGB16Float,

    LLGLFormatRGB32UInt,
    LLGLFormatRGB32SInt,
    LLGLFormatRGB32Float,

    LLGLFormatRGB64Float,

    /* --- RGBA color formats --- */
    LLGLFormatRGBA8UNorm,
    LLGLFormatRGBA8UNorm_sRGB,
    LLGLFormatRGBA8SNorm,
    LLGLFormatRGBA8UInt,
    LLGLFormatRGBA8SInt,

    LLGLFormatRGBA16UNorm,
    LLGLFormatRGBA16SNorm,
    LLGLFormatRGBA16UInt,
    LLGLFormatRGBA16SInt,
    LLGLFormatRGBA16Float,

    LLGLFormatRGBA32UInt,
    LLGLFormatRGBA32SInt,
    LLGLFormatRGBA32Float,

    LLGLFormatRGBA64Float,

    /* --- BGRA color formats --- */
    LLGLFormatBGRA8UNorm,
    LLGLFormatBGRA8UNorm_sRGB,
    LLGLFormatBGRA8SNorm,
    LLGLFormatBGRA8UInt,
    LLGLFormatBGRA8SInt,

    /* --- Packed formats --- */
    LLGLFormatRGB10A2UNorm,
    LLGLFormatRGB10A2UInt,
    LLGLFormatRG11B10Float,
    LLGLFormatRGB9E5Float,

    /* --- Depth-stencil formats --- */
    LLGLFormatD16UNorm,
    LLGLFormatD24UNormS8UInt,
    LLGLFormatD32Float,
    LLGLFormatD32FloatS8X24UInt,

    /* --- Block compression (BC) formats --- */
    LLGLFormatBC1UNorm,
    LLGLFormatBC1UNorm_sRGB,
    LLGLFormatBC2UNorm,
    LLGLFormatBC2UNorm_sRGB,
    LLGLFormatBC3UNorm,
    LLGLFormatBC3UNorm_sRGB,
    LLGLFormatBC4UNorm,
    LLGLFormatBC4SNorm,
    LLGLFormatBC5UNorm,
    LLGLFormatBC5SNorm,
}
LLGLFormat;

typedef enum LLGLImageFormat
{
    /* Color formats */
    LLGLImageFormatAlpha,
    LLGLImageFormatR,
    LLGLImageFormatRG,
    LLGLImageFormatRGB,
    LLGLImageFormatBGR,
    LLGLImageFormatRGBA,
    LLGLImageFormatBGRA,
    LLGLImageFormatARGB,
    LLGLImageFormatABGR,

    /* Depth-stencil formats */
    LLGLImageFormatDepth,
    LLGLImageFormatDepthStencil,

    /* Compressed formats */
    LLGLImageFormatBC1,
    LLGLImageFormatBC2,
    LLGLImageFormatBC3,
    LLGLImageFormatBC4,
    LLGLImageFormatBC5,
}
LLGLImageFormat;

typedef enum LLGLDataType
{
    LLGLDataTypeUndefined,

    LLGLDataTypeInt8,
    LLGLDataTypeUInt8,

    LLGLDataTypeInt16,
    LLGLDataTypeUInt16,

    LLGLDataTypeInt32,
    LLGLDataTypeUInt32,

    LLGLDataTypeFloat16,
    LLGLDataTypeFloat32,
    LLGLDataTypeFloat64,
}
LLGLDataType;

typedef enum LLGLFormatFlags
{
    LLGLFormatHasDepth              = (1 << 0),
    LLGLFormatHasStencil            = (1 << 1),
    LLGLFormatIsColorSpace_sRGB     = (1 << 2),
    LLGLFormatIsCompressed          = (1 << 3),
    LLGLFormatIsNormalized          = (1 << 4),
    LLGLFormatIsInteger             = (1 << 5),
    LLGLFormatIsUnsigned            = (1 << 6),
    LLGLFormatIsPacked              = (1 << 7),
    LLGLFormatSupportsRenderTarget  = (1 << 8),
    LLGLFormatSupportsMips          = (1 << 9),
    LLGLFormatSupportsGenerateMips  = (1 << 10),
    LLGLFormatSupportsTexture1D     = (1 << 11),
    LLGLFormatSupportsTexture2D     = (1 << 12),
    LLGLFormatSupportsTexture3D     = (1 << 13),
    LLGLFormatSupportsTextureCube   = (1 << 14),
    LLGLFormatSupportsVertex        = (1 << 15),
    LLGLFormatIsUnsignedInteger     = (LLGLFormatIsUnsigned | LLGLFormatIsInteger),
    LLGLFormatHasDepthStencil       = (LLGLFormatHasDepth | LLGLFormatHasStencil),
}
LLGLFormatFlags;


/* ----- Structures ----- */

typedef struct LLGLFormatAttributes
{
    uint16_t        bitSize;
    uint8_t         blockWidth;
    uint8_t         blockHeight;
    uint8_t         components;
    LLGLImageFormat format;
    LLGLDataType    dataType;
    long            flags;
}
LLGLFormatAttributes;


/* ----- Functions ----- */

LLGL_C_EXPORT const LLGLFormatAttributes* llglGetFormatAttribs(LLGLFormat format);


#endif



// ================================================================================
