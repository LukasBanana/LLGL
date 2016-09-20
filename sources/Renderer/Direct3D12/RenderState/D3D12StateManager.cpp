/*
 * D3D12StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12StateManager.h"


namespace LLGL
{


D3D12StateManager::D3D12StateManager(ComPtr<ID3D12GraphicsCommandList>& commandList) :
    commandList_( commandList )
{
}

void D3D12StateManager::SetViewports(const std::vector<Viewport>& viewports)
{
    viewports_.resize(viewports.size());
    
    for (std::size_t i = 0, n = viewports.size(); i < n; ++i)
    {
        const auto& src = viewports[i];
        auto& dest = viewports_[i];

        dest.TopLeftX   = src.x;
        dest.TopLeftY   = src.y;
        dest.Width      = src.width;
        dest.Height     = src.height;
        dest.MinDepth   = src.minDepth;
        dest.MaxDepth   = src.maxDepth;
    }
}

void D3D12StateManager::SubmitViewports()
{
    if (!viewports_.empty())
        commandList_->RSSetViewports(static_cast<UINT>(viewports_.size()), viewports_.data());
}

void D3D12StateManager::SetScissors(const std::vector<Scissor>& scissors)
{
    scissors_.resize(scissors.size());
    
    for (std::size_t i = 0, n = scissors.size(); i < n; ++i)
    {
        const auto& src = scissors[i];
        auto& dest = scissors_[i];

        dest.left   = src.x;
        dest.top    = src.y;
        dest.right  = src.x + src.width;
        dest.bottom = src.y + src.height;
    }
}

void D3D12StateManager::SubmitScissors()
{
    if (!scissors_.empty())
        commandList_->RSSetScissorRects(static_cast<UINT>(scissors_.size()), scissors_.data());
}


} // /namespace LLGL



// ================================================================================
