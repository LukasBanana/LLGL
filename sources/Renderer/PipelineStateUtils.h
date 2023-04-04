/*
 * PipelineStateUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PIPELINE_STATE_UTILS_H
#define LLGL_PIPELINE_STATE_UTILS_H


#include <LLGL/BufferFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "CheckedCast.h"


namespace LLGL
{


struct StencilDescriptor;
struct BlendDescriptor;
struct GraphicsPipelineDescriptor;
struct ComputePipelineDescriptor;
class Shader;

/* ----- Functions ----- */

// Returns true if the stencil reference will be used by a PSO with the specified stencil descriptor.
LLGL_EXPORT bool IsStencilRefEnabled(const StencilDescriptor& desc);

// Returns true if the stencil reference will be used for a static pipeline state.
LLGL_EXPORT bool IsStaticStencilRefEnabled(const StencilDescriptor& desc);

// Returns true if the blend factor will be used by a PSO with the specified blending descriptor.
LLGL_EXPORT bool IsBlendFactorEnabled(const BlendDescriptor& desc);

// Returns true if any of the enabled blend targets makes use the blending factor (RGBA) for a static pipeline state.
LLGL_EXPORT bool IsStaticBlendFactorEnabled(const BlendDescriptor& desc);

// Returns the set of graphics PSO shaders as array.
LLGL_EXPORT SmallVector<Shader*, 5> GetShadersAsArray(const GraphicsPipelineDescriptor& desc);

// Returns the set of compute PSO shaders as array.
LLGL_EXPORT SmallVector<Shader*, 1> GetShadersAsArray(const ComputePipelineDescriptor& desc);

// Returns the size (in bytes) of the specified uniform with optional array size. This includes padding between array elements.
LLGL_EXPORT std::uint32_t GetUniformTypeSize(UniformType type, std::uint32_t arraySize = 0);

// Casts the specified array of shaders to their backend implementation.
template <typename T>
inline SmallVector<T*, 5> CastShaderArray(const ArrayView<Shader*>& shaders)
{
    SmallVector<T*, 5> outShaders;
    for (auto* sh : shaders)
        outShaders.push_back(LLGL_CAST(T*, sh));
    return outShaders;
}


} // /namespace LLGL


#endif



// ================================================================================
