/*
 * SamplerFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SAMPLER_FLAGS_H
#define LLGL_C99_SAMPLER_FLAGS_H


#include <LLGL-C/PipelineStateFlags.h>
#include <stdint.h>
#include <stdbool.h>


/* ----- Enumerations ----- */

typedef enum LLGLSamplerAddressMode
{
    LLGLSamplerAddressModeRepeat,
    LLGLSamplerAddressModeMirror,
    LLGLSamplerAddressModeClamp,
    LLGLSamplerAddressModeBorder,
    LLGLSamplerAddressModeMirrorOnce,
}
LLGLSamplerAddressMode;

typedef enum LLGLSamplerFilter
{
    LLGLSamplerFilterNearest,
    LLGLSamplerFilterLinear,
}
LLGLSamplerFilter;


/* ----- Structures ----- */

typedef struct LLGLSamplerDescriptor
{
    LLGLSamplerAddressMode  addressModeU;
    LLGLSamplerAddressMode  addressModeV;
    LLGLSamplerAddressMode  addressModeW;
    LLGLSamplerFilter       minFilter;
    LLGLSamplerFilter       magFilter;
    LLGLSamplerFilter       mipMapFilter;
    bool                    mipMapEnabled;
    float                   mipMapLODBias;
    float                   minLOD;
    float                   maxLOD;
    uint32_t                maxAnisotropy;
    bool                    compareEnabled;
    LLGLCompareOp           compareOp;
    float                   borderColor[4];
}
LLGLSamplerDescriptor;


#endif



// ================================================================================
