/*
 * ShaderReflectionFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SHADER_REFLECTION_H
#define LLGL_C99_SHADER_REFLECTION_H


#include <LLGL-C/ResourceFlags.h>
#include <LLGL-C/BufferFlags.h>
#include <LLGL-C/PipelineLayoutFlags.h>
#include <LLGL-C/ShaderFlags.h>
#include <stddef.h>
#include <stdint.h>


/* ----- Enumerations ----- */

typedef enum LLGLStorageBufferType
{
    LLGLStorageBufferTypeUndefined,
    LLGLStorageBufferTypeTypedBuffer,
    LLGLStorageBufferTypeStructuredBuffer,
    LLGLStorageBufferTypeByteAddressBuffer,
    LLGLStorageBufferTypeRWTypedBuffer,
    LLGLStorageBufferTypeRWStructuredBuffer,
    LLGLStorageBufferTypeRWByteAddressBuffer,
    LLGLStorageBufferTypeAppendStructuredBuffer,
    LLGLStorageBufferTypeConsumeStructuredBuffer,
}
LLGLStorageBufferType;


/* ----- Structures ----- */

typedef struct LLGLShaderResourceReflection
{
    LLGLBindingDescriptor   binding;
    uint32_t                constantBufferSize;
    LLGLStorageBufferType   storageBufferType;
}
LLGLShaderResourceReflection;

typedef struct LLGLShaderReflection
{
    size_t                              numResources;
    const LLGLShaderResourceReflection* resources;
    size_t                              numUniforms;
    const LLGLUniformDescriptor*        uniforms;
    LLGLVertexShaderAttributes          vertex;
    LLGLFragmentShaderAttributes        fragment;
    LLGLComputeShaderAttributes         compute;
}
LLGLShaderReflection;


#endif



// ================================================================================
