/*
 * ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMPUTE_PIPELINE_H
#define LLGL_COMPUTE_PIPELINE_H


#include "Export.h"


namespace LLGL
{


class ShaderProgram;

//! Compute pipeline descriptor structure.
struct ComputePipelineDescriptor
{
    ComputePipelineDescriptor() = default;
    ComputePipelineDescriptor(ShaderProgram* shaderProgram) :
        shaderProgram { shaderProgram }
    {
    }

    /**
    \brief Pointer to the shader program for the compute pipeline.
    \remarks This must never be null when "RenderSystem::CreateComputePipeline" is called with this structure.
    \see RenderSystem::CreateComputePipeline
    \see RenderSystem::CreateShaderProgram
    */
    ShaderProgram* shaderProgram = nullptr;
};


//! Compute pipeline interface.
class LLGL_EXPORT ComputePipeline
{

    public:

        virtual ~ComputePipeline()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
