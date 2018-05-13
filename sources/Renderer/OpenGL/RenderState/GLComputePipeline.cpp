/*
 * GLComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLComputePipeline.h"
#include "GLStateManager.h"
#include "../Shader/GLShaderProgram.h"
#include "../../CheckedCast.h"


namespace LLGL
{


GLComputePipeline::GLComputePipeline(const ComputePipelineDescriptor& desc)
{
    /* Convert shader state */
    shaderProgram_ = LLGL_CAST(GLShaderProgram*, desc.shaderProgram);
    if (!shaderProgram_)
        throw std::invalid_argument("failed to create compute pipeline due to missing shader program");
}

void GLComputePipeline::Bind(GLStateManager& stateMngr)
{
    /* Setup shader state */
    stateMngr.BindShaderProgram(shaderProgram_->GetID());
}


} // /namespace LLGL



// ================================================================================
