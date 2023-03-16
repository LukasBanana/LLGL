/*
 * PipelineStateUtils.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_STATE_UTILS_H
#define LLGL_PIPELINE_STATE_UTILS_H


#include <LLGL/BufferFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "CheckedCast.h"


namespace LLGL
{


struct StencilDescriptor;
struct BlendDescriptor;
struct GraphicsPipelineDescriptor;
struct ComputePipelineDescriptor;
class Shader;

/* ----- Functions ----- */

// Returns true if the stencil reference will be used for a static pipeline state.
LLGL_EXPORT bool IsStaticStencilRefEnabled(const StencilDescriptor& desc);

// Returns true if any of the enabled blend targets makes use the blending factor (RGBA) for a static pipeline state.
LLGL_EXPORT bool IsStaticBlendFactorEnabled(const BlendDescriptor& desc);

// Returns the set of graphics PSO shaders as array.
LLGL_EXPORT SmallVector<Shader*, 5> GetShadersAsArray(const GraphicsPipelineDescriptor& desc);

// Returns the set of compute PSO shaders as array.
LLGL_EXPORT SmallVector<Shader*, 1> GetShadersAsArray(const ComputePipelineDescriptor& desc);

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
