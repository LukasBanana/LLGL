/*
 * D3D12RenderPSOBase.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RenderPSOBase.h"
#include "D3D12PipelineStateUtils.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


D3D12RenderPSOBase::D3D12RenderPSOBase(
    D3D12PipelineType           type,

    const StencilDescriptor&    stencilDesc,
    const BlendDescriptor&      blendDesc,
    bool                        isScissorEnabled,
    const ArrayView<Viewport>&  staticViewports,
    const ArrayView<Scissor>&   staticScissors,

    const PipelineLayout*       pipelineLayout,
    const ArrayView<Shader*>&   shaders,
    D3D12PipelineLayout&        defaultPipelineLayout)
:
    D3D12PipelineState { type, pipelineLayout, shaders, defaultPipelineLayout }
{
    /* Store dynamic pipeline states */
    scissorEnabled_     = (isScissorEnabled ? 1 : 0);

    stencilRefEnabled_  = (IsStaticStencilRefEnabled(stencilDesc) ? 1 : 0);
    stencilRef_         = stencilDesc.front.reference;

    blendFactorEnabled_ = IsStaticBlendFactorEnabled(blendDesc);
    blendFactor_[0]     = blendDesc.blendFactor[0];
    blendFactor_[1]     = blendDesc.blendFactor[1];
    blendFactor_[2]     = blendDesc.blendFactor[2];
    blendFactor_[3]     = blendDesc.blendFactor[3];

    /* Build static state buffer for viewports and scissors */
    if (!staticViewports.empty() || !staticScissors.empty())
        BuildStaticStateBuffer(staticViewports, staticScissors);
}

void D3D12RenderPSOBase::BindOutputMergerAndStaticStates(ID3D12GraphicsCommandList* commandList)
{
    if (stencilRefEnabled_ != 0)
        commandList->OMSetStencilRef(stencilRef_);
    if (blendFactorEnabled_ != 0)
        commandList->OMSetBlendFactor(blendFactor_);

    SetStaticViewportsAndScissors(commandList);
}

UINT D3D12RenderPSOBase::NumDefaultScissorRects() const
{
    return std::max<UINT>(staticStateBuffer_.GetNumViewports(), 1u);
}

void D3D12RenderPSOBase::BuildStaticStateBuffer(const ArrayView<Viewport>& viewports, const ArrayView<Scissor>& scissors)
{
    ByteBufferIterator byteBufferIter = staticStateBuffer_.Allocate(
        viewports.size(), scissors.size(),
        sizeof(D3D12_VIEWPORT), sizeof(D3D12_RECT),
        GetMutableReport(), D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
    );

    /* Build <D3D12_VIEWPORT> entries */
    for_range(i, staticStateBuffer_.GetNumViewports())
    {
        D3D12_VIEWPORT* dst = byteBufferIter.Next<D3D12_VIEWPORT>();
        {
            dst->TopLeftX   = viewports[i].x;
            dst->TopLeftY   = viewports[i].y;
            dst->Width      = viewports[i].width;
            dst->Height     = viewports[i].height;
            dst->MinDepth   = viewports[i].minDepth;
            dst->MaxDepth   = viewports[i].maxDepth;
        }
    }

    /* Build <D3D12_RECT> entries */
    for_range(i, staticStateBuffer_.GetNumScissors())
    {
        D3D12_RECT* dst = byteBufferIter.Next<D3D12_RECT>();
        {
            dst->left   = static_cast<LONG>(scissors[i].x);
            dst->top    = static_cast<LONG>(scissors[i].y);
            dst->right  = static_cast<LONG>(scissors[i].x + scissors[i].width);
            dst->bottom = static_cast<LONG>(scissors[i].y + scissors[i].height);
        }
    }
}

void D3D12RenderPSOBase::SetStaticViewportsAndScissors(ID3D12GraphicsCommandList* commandList)
{
    if (staticStateBuffer_)
    {
        ByteBufferConstIterator byteBufferIter = staticStateBuffer_.GetBufferIterator();
        if (staticStateBuffer_.GetNumViewports() > 0)
        {
            commandList->RSSetViewports(
                staticStateBuffer_.GetNumViewports(),
                byteBufferIter.Next<D3D12_VIEWPORT>(staticStateBuffer_.GetNumViewports())
            );
        }
        if (staticStateBuffer_.GetNumScissors() > 0)
        {
            commandList->RSSetScissorRects(
                staticStateBuffer_.GetNumScissors(),
                byteBufferIter.Next<D3D12_RECT>(staticStateBuffer_.GetNumScissors())
            );
        }
    }
}


} // /namespace LLGL



// ================================================================================
