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
    return std::max(numStaticViewports_, 1u);
}

// Returns the size (in bytes) for the static-state buffer with the specified number of viewports and scissor rectangles
static std::size_t GetStaticStateBufferSize(std::size_t numViewports, std::size_t numScissors)
{
    return (numViewports * sizeof(D3D12_VIEWPORT) + numScissors * sizeof(D3D12_RECT));
}

void D3D12RenderPSOBase::BuildStaticStateBuffer(const ArrayView<Viewport>& staticViewports, const ArrayView<Scissor>& staticScissors)
{
    /* Allocate packed raw buffer */
    const std::size_t bufferSize = GetStaticStateBufferSize(staticViewports.size(), staticScissors.size());
    staticStateBuffer_ = DynamicByteArray{ bufferSize };

    ByteBufferIterator byteBufferIter{ staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!staticViewports.empty())
        BuildStaticViewports(staticViewports.size(), staticViewports.data(), byteBufferIter);

    /* Build static scissors in raw buffer */
    if (!staticScissors.empty())
        BuildStaticScissors(staticScissors.size(), staticScissors.data(), byteBufferIter);
}

void D3D12RenderPSOBase::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
{
    /* Store number of viewports and validate limit */
    numStaticViewports_ = static_cast<UINT>(numViewports);

    if (numStaticViewports_ > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        GetMutableReport().Errorf(
            "too many viewports in graphics pipeline state; %u specified, but limit is %d",
            numStaticViewports_, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
        );
        return;
    }

    /* Build <D3D12_VIEWPORT> entries */
    for_range(i, numViewports)
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
}

void D3D12RenderPSOBase::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
{
    /* Store number of scissors and validate limit */
    numStaticScissors_ = static_cast<UINT>(numScissors);

    if (numStaticScissors_ > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
    {
        GetMutableReport().Errorf(
            "too many scissors in graphics pipeline state; %u specified, but limit is %d",
            numStaticScissors_, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
        );
        return;
    }

    /* Build <D3D12_RECT> entries */
    for_range(i, numScissors)
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
        ByteBufferIterator byteBufferIter{ staticStateBuffer_.get() };
        if (numStaticViewports_ > 0)
        {
            commandList->RSSetViewports(
                numStaticViewports_,
                byteBufferIter.Next<D3D12_VIEWPORT>(numStaticViewports_)
            );
        }
        if (numStaticScissors_ > 0)
        {
            commandList->RSSetScissorRects(
                numStaticScissors_,
                byteBufferIter.Next<D3D12_RECT>(numStaticScissors_)
            );
        }
    }
}


} // /namespace LLGL



// ================================================================================
