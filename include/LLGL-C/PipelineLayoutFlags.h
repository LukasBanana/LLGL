/*
 * PipelineLayoutFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_PIPELINE_LAYOUT_FLAGS_H
#define LLGL_C99_PIPELINE_LAYOUT_FLAGS_H


#include <LLGL-C/ResourceFlags.h>
#include <LLGL-C/BufferFlags.h>
#include <LLGL-C/SamplerFlags.h>
#include <LLGL-C/ShaderFlags.h>
#include <stddef.h>
#include <stdint.h>


/* ----- Enumerations ----- */

typedef enum LLGLUniformType
{
    LLGLUniformTypeUndefined,

    /* ----- Scalars & Vectors ----- */
    LLGLUniformTypeFloat1,
    LLGLUniformTypeFloat2,
    LLGLUniformTypeFloat3,
    LLGLUniformTypeFloat4,
    LLGLUniformTypeDouble1,
    LLGLUniformTypeDouble2,
    LLGLUniformTypeDouble3,
    LLGLUniformTypeDouble4,
    LLGLUniformTypeInt1,
    LLGLUniformTypeInt2,
    LLGLUniformTypeInt3,
    LLGLUniformTypeInt4,
    LLGLUniformTypeUInt1,
    LLGLUniformTypeUInt2,
    LLGLUniformTypeUInt3,
    LLGLUniformTypeUInt4,
    LLGLUniformTypeBool1,
    LLGLUniformTypeBool2,
    LLGLUniformTypeBool3,
    LLGLUniformTypeBool4,

    /* ----- Matrices ----- */
    LLGLUniformTypeFloat2x2,
    LLGLUniformTypeFloat2x3,
    LLGLUniformTypeFloat2x4,
    LLGLUniformTypeFloat3x2,
    LLGLUniformTypeFloat3x3,
    LLGLUniformTypeFloat3x4,
    LLGLUniformTypeFloat4x2,
    LLGLUniformTypeFloat4x3,
    LLGLUniformTypeFloat4x4,
    LLGLUniformTypeDouble2x2,
    LLGLUniformTypeDouble2x3,
    LLGLUniformTypeDouble2x4,
    LLGLUniformTypeDouble3x2,
    LLGLUniformTypeDouble3x3,
    LLGLUniformTypeDouble3x4,
    LLGLUniformTypeDouble4x2,
    LLGLUniformTypeDouble4x3,
    LLGLUniformTypeDouble4x4,

    /* ----- Resources ----- */
    LLGLUniformTypeSampler,
    LLGLUniformTypeImage,
    LLGLUniformTypeAtomicCounter,
}
LLGLUniformType;


/* ----- Structures ----- */

typedef struct LLGLBindingSlot
{
    uint32_t index;
    uint32_t set;
}
LLGLBindingSlot;

typedef struct LLGLBindingDescriptor
{
    const char*         name;
    LLGLResourceType    type;
    long                bindFlags;
    long                stageFlags;
    LLGLBindingSlot     slot;
    uint32_t            arraySize;
}
LLGLBindingDescriptor;

typedef struct LLGLStaticSamplerDescriptor
{
    const char*             name;
    long                    stageFlags;
    LLGLBindingSlot         slot;
    LLGLSamplerDescriptor   sampler;
}
LLGLStaticSamplerDescriptor;

typedef struct LLGLUniformDescriptor
{
    const char*     name;
    LLGLUniformType type;
    uint32_t        arraySize;
}
LLGLUniformDescriptor;

typedef struct LLGLPipelineLayoutDescriptor
{
    size_t                              numHeapBindings;
    const LLGLBindingDescriptor*        heapBindings;
    size_t                              numBindings;
    const LLGLBindingDescriptor*        bindings;
    size_t                              numStaticSamplers;
    const LLGLStaticSamplerDescriptor*  staticSamplers;
    size_t                              numUniforms;
    const LLGLUniformDescriptor*        uniforms;
}
LLGLPipelineLayoutDescriptor;


#endif



// ================================================================================
