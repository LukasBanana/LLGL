/*
 * D3D9ProgrammablePSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ProgrammablePSO.h"
#include "D3D9PipelineLayout.h"
#include "D3D9StateManager.h"
#include "../Shader/D3D9VertexShader.h"
#include "../Shader/D3D9PixelShader.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D9ProgrammablePSO::D3D9ProgrammablePSO(const GraphicsPipelineDescriptor& desc) :
    D3D9PipelineState { desc, true }
{
    if (desc.vertexShader != nullptr)
    {
        auto* vertexShaderD3D = LLGL_CAST(D3D9VertexShader*, desc.vertexShader);
        d3dVertexShader_ = ComPtr<IDirect3DVertexShader9>(vertexShaderD3D->GetNative());
        streamSourceFreq_ = vertexShaderD3D->GetStreamSourceFreq();
    }
    if (desc.fragmentShader != nullptr)
    {
        auto* pixelShaderD3D = LLGL_CAST(D3D9PixelShader*, desc.fragmentShader);
        d3dPixelShader_ = ComPtr<IDirect3DPixelShader9>(pixelShaderD3D->GetNative());
    }

    if (desc.pipelineLayout != nullptr)
    {
        const D3D9PipelineLayout* pipelineLayoutD3D = LLGL_CAST(const D3D9PipelineLayout*, desc.pipelineLayout);
        const std::vector<UniformDescriptor>& uniformDescs = pipelineLayoutD3D->GetUniforms();
        if (!uniformDescs.empty())
            constantsCache_ = MakeUnique<D3D9ConstantsCache>(desc.vertexShader, desc.fragmentShader, uniformDescs);
    }
}

const Report* D3D9ProgrammablePSO::GetReport() const
{
    return nullptr; //TODO
}

void D3D9ProgrammablePSO::Bind(D3D9StateManager& stateMngr)
{
    D3D9PipelineState::Bind(stateMngr);

    IDirect3DDevice9* device = stateMngr.GetDevice();
    device->SetVertexShader(GetVertexShader());
    device->SetPixelShader(GetPixelShader());

    stateMngr.SetStreamSourceFreqInstanceData(static_cast<UINT>(streamSourceFreq_.size()), streamSourceFreq_.data());
}


} // /namespace LLGL



// ================================================================================
