/*
 * PipelineStateUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_STATE_UTILS_H
#define LLGL_PIPELINE_STATE_UTILS_H


#include <LLGL/BufferFlags.h>


namespace LLGL
{


struct StencilDescriptor;
struct BlendDescriptor;

/* ----- Functions ----- */

// Returns true if the stencil reference will be used for a static pipeline state.
LLGL_EXPORT bool IsStaticStencilRefEnabled(const StencilDescriptor& desc);

// Returns true if any of the enabled blend targets makes use the blending factor (RGBA) for a static pipeline state.
LLGL_EXPORT bool IsStaticBlendFactorEnabled(const BlendDescriptor& desc);


} // /namespace LLGL


#endif



// ================================================================================
