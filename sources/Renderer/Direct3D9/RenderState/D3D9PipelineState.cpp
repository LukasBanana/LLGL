/*
 * D3D9PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineState.h"
#include "D3D9StateManager.h"
#include "D3D9StatePool.h"
#include "../D3D9Types.h"
#include "../Shader/D3D9VertexShader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D9PipelineState::D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline) :
    isProgrammablePipeline_ { isProgrammablePipeline                                },
    primitiveType_          { D3D9Types::ToD3DPrimitiveType(desc.primitiveTopology) }
{
    if (desc.vertexShader != nullptr)
    {
        auto* vertexShaderD3D = LLGL_CAST(D3D9VertexShader*, desc.vertexShader);
        d3dVertexDecl_ = ComPtr<IDirect3DVertexDeclaration9>(vertexShaderD3D->GetVertexDeclaration());
    }

    /* Create depth-stencil, rasterizer, and blend states */
    depthStencilState_  = D3D9StatePool::Get().CreateDepthStencilState(desc.depth, desc.stencil);
    rasterizerState_    = D3D9StatePool::Get().CreateRasterizerState(desc.rasterizer);
    blendState_         = D3D9StatePool::Get().CreateBlendState(desc.blend);
}

D3D9PipelineState::~D3D9PipelineState()
{
    D3D9StatePool::Get().ReleaseDepthStencilState(std::move(depthStencilState_));
    D3D9StatePool::Get().ReleaseRasterizerState(std::move(rasterizerState_));
    D3D9StatePool::Get().ReleaseBlendState(std::move(blendState_));
}

void D3D9PipelineState::Bind(D3D9StateManager& stateMngr)
{
    stateMngr.GetDevice()->SetVertexDeclaration(GetVertexDeclaration());

    stateMngr.BindDepthStencilState(depthStencilState_.get());
    stateMngr.BindRasterizerState(rasterizerState_.get());
    stateMngr.BindBlendState(blendState_.get());
}

void D3D9PipelineState::SetDebugName(const char* name)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
