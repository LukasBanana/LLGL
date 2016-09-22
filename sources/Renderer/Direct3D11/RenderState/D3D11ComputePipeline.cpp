/*
 * D3D11ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ComputePipeline.h"
#include "../Shader/D3D11ShaderProgram.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D11ComputePipeline::D3D11ComputePipeline(const ComputePipelineDescriptor& desc)
{
    /* Convert shader state */
    auto shaderProgramD3D = LLGL_CAST(D3D11ShaderProgram*, desc.shaderProgram);
    if (shaderProgramD3D && shaderProgramD3D->GetCS())
        cs_ = shaderProgramD3D->GetCS()->GetHardwareShader().cs;
    else
        throw std::invalid_argument("failed to create compute pipeline due to missing compute shader program");
}

void D3D11ComputePipeline::Bind(ID3D11DeviceContext* context)
{
    context->CSSetShader(cs_.Get(), nullptr, 0);
}


} // /namespace LLGL



// ================================================================================
