/*
 * D3D11ComputePSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ComputePSO.h"
#include "D3D11StateManager.h"
#include "../Shader/D3D11ShaderProgram.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D11ComputePSO::D3D11ComputePSO(const ComputePipelineDescriptor& desc)
{
    /* Convert shader state */
    auto shaderProgramD3D = LLGL_CAST(const D3D11ShaderProgram*, desc.shaderProgram);
    if (shaderProgramD3D && shaderProgramD3D->GetCS())
        cs_ = shaderProgramD3D->GetCS()->GetNative().cs;
    else
        throw std::invalid_argument("failed to create compute pipeline due to missing compute shader program");
}

void D3D11ComputePSO::Bind(D3D11StateManager& stateMngr)
{
    stateMngr.SetComputeShader(cs_.Get());
}


} // /namespace LLGL



// ================================================================================
