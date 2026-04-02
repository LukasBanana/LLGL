/*
 * D3D9PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineState.h"
#include "D3D9PipelineLayout.h"
#include "D3D9StateManager.h"
#include "D3D9StatePool.h"
#include "../D3D9Types.h"
#include "../Shader/D3D9VertexShader.h"
#include "../../CheckedCast.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


D3D9PipelineState::D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline) :
    isProgrammablePipeline_ { isProgrammablePipeline                                },
    primitiveType_          { D3D9Types::ToD3DPrimitiveType(desc.primitiveTopology) }
{
    if (desc.pipelineLayout != nullptr)
        pipelineLayout_ = LLGL_CAST(const D3D9PipelineLayout*, desc.pipelineLayout);

    if (desc.vertexShader != nullptr)
    {
        auto* vertexShaderD3D = LLGL_CAST(D3D9VertexShader*, desc.vertexShader);
        d3dVertexDecl_ = ComPtr<IDirect3DVertexDeclaration9>(vertexShaderD3D->GetVertexDeclaration());
    }

    /* Create depth-stencil, rasterizer, and blend states */
    depthStencilState_  = D3D9StatePool::Get().CreateDepthStencilState(desc.depth, desc.stencil);
    rasterizerState_    = D3D9StatePool::Get().CreateRasterizerState(desc.rasterizer);
    blendState_         = D3D9StatePool::Get().CreateBlendState(desc.blend);

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);
}

D3D9PipelineState::~D3D9PipelineState()
{
    D3D9StatePool::Get().ReleaseDepthStencilState(std::move(depthStencilState_));
    D3D9StatePool::Get().ReleaseRasterizerState(std::move(rasterizerState_));
    D3D9StatePool::Get().ReleaseBlendState(std::move(blendState_));
}

void D3D9PipelineState::SetDebugName(const char* name)
{
    // dummy
}

const Report* D3D9PipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

void D3D9PipelineState::Bind(D3D9StateManager& stateMngr)
{
    stateMngr.GetDevice()->SetVertexDeclaration(GetVertexDeclaration());

    stateMngr.BindDepthStencilState(depthStencilState_.get());
    stateMngr.BindRasterizerState(rasterizerState_.get());
    stateMngr.BindBlendState(blendState_.get());

    if (pipelineLayout_ != nullptr)
        pipelineLayout_->BindStaticSamplers(stateMngr);

    /* Set static viewports and scissors */
    SetStaticViewportsAndScissors(stateMngr.GetDevice());
}


/*
 * ======= Private: =======
 */

void D3D9PipelineState::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    constexpr UINT d3dMaxNumViewportsAndScissors = 1;
    ByteBufferIterator byteBufferIter = staticStateBuffer_.Allocate(
        desc.viewports.size(), desc.scissors.size(),
        sizeof(D3DVIEWPORT9), sizeof(RECT),
        GetMutableReport(),
        d3dMaxNumViewportsAndScissors
    );

    /* Build <GLViewport> entries */
    for_range(i, staticStateBuffer_.GetNumViewports())
    {
        auto dst = byteBufferIter.Next<D3DVIEWPORT9>();
        {
            dst->X      = static_cast<DWORD>(desc.viewports[i].x);
            dst->Y      = static_cast<DWORD>(desc.viewports[i].y);
            dst->Width  = static_cast<DWORD>(desc.viewports[i].width);
            dst->Height = static_cast<DWORD>(desc.viewports[i].height);
            dst->MinZ   = desc.viewports[i].minDepth;
            dst->MaxZ   = desc.viewports[i].maxDepth;
        }
    }

    /* Build <GLScissor> entries */
    for_range(i, staticStateBuffer_.GetNumScissors())
    {
        auto dst = byteBufferIter.Next<RECT>();
        {
            dst->left   = static_cast<LONG>(desc.scissors[i].x);
            dst->top    = static_cast<LONG>(desc.scissors[i].y);
            dst->right  = static_cast<LONG>(desc.scissors[i].x + desc.scissors[i].width);
            dst->bottom = static_cast<LONG>(desc.scissors[i].y + desc.scissors[i].height);
        }
    }
}

void D3D9PipelineState::SetStaticViewportsAndScissors(IDirect3DDevice9* device)
{
    if (staticStateBuffer_)
    {
        ByteBufferConstIterator byteBufferIter = staticStateBuffer_.GetBufferIterator();
        if (staticStateBuffer_.GetNumViewports() > 0)
            device->SetViewport(byteBufferIter.Next<D3DVIEWPORT9>(staticStateBuffer_.GetNumViewports()));
        if (staticStateBuffer_.GetNumScissors() > 0)
            device->SetScissorRect(byteBufferIter.Next<RECT>(staticStateBuffer_.GetNumScissors()));
    }
}


} // /namespace LLGL



// ================================================================================
