/*
 * ComputePipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMPUTE_PIPELINE_FLAGS_H
#define LLGL_COMPUTE_PIPELINE_FLAGS_H


namespace LLGL
{


class ShaderProgram;
class PipelineLayout;

//! Compute pipeline descriptor structure.
struct ComputePipelineDescriptor
{
    ComputePipelineDescriptor() = default;

    //! Constructor to initialize the entire descriptor.
    inline ComputePipelineDescriptor(ShaderProgram* shaderProgram, PipelineLayout* pipelineLayout = nullptr) :
        shaderProgram  { shaderProgram  },
        pipelineLayout { pipelineLayout }
    {
    }

    /**
    \brief Pointer to the shader program for the compute pipeline.
    \remarks This must never be null when "RenderSystem::CreateComputePipeline" is called with this structure.
    \see RenderSystem::CreateComputePipeline
    \see RenderSystem::CreateShaderProgram
    */
    ShaderProgram*  shaderProgram   = nullptr;

    /**
    \brief Pointer to an optional pipeline layout for the graphics pipeline.
    \remarks This layout determines at which slots buffer resources can be bound.
    This is ignored by render systems which do not support pipeline layouts.
    \note Only supported with: Vulkan, Direct3D 12
    */
    PipelineLayout* pipelineLayout  = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
