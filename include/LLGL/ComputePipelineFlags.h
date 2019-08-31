/*
 * ComputePipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMPUTE_PIPELINE_FLAGS_H
#define LLGL_COMPUTE_PIPELINE_FLAGS_H


namespace LLGL
{


class ShaderProgram;
class PipelineLayout;

/**
\brief Compute pipeline descriptor structure.
\see RenderSystem::CreateComputePipeline
*/
struct ComputePipelineDescriptor
{
    /**
    \brief Pointer to the shader program for the compute pipeline.
    \remarks This must never be null when RenderSystem::CreateComputePipeline is called with this structure.
    \see RenderSystem::CreateComputePipeline
    \see RenderSystem::CreateShaderProgram
    */
    const ShaderProgram*  shaderProgram     = nullptr;

    /**
    \brief Pointer to an optional pipeline layout for the graphics pipeline.
    \remarks This layout determines at which slots buffer resources can be bound.
    This is ignored by render systems which do not support pipeline layouts.
    \note Only supported with: Vulkan, Direct3D 12
    */
    const PipelineLayout* pipelineLayout    = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
