/*
 * D3D11StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StateManager.h"


namespace LLGL
{


D3D11StateManager::D3D11StateManager(ComPtr<ID3D11DeviceContext>& context) :
    context_( context )
{
}

void D3D11StateManager::SetViewports(const std::vector<Viewport>& viewports)
{
    std::vector<D3D11_VIEWPORT> viewportsD3D(viewports.size());
    
    for (std::size_t i = 0, n = viewports.size(); i < n; ++i)
    {
        const auto& src = viewports[i];
        auto& dest = viewportsD3D[i];

        dest.TopLeftX   = src.x;
        dest.TopLeftY   = src.y;
        dest.Width      = src.width;
        dest.Height     = src.height;
        dest.MinDepth   = src.minDepth;
        dest.MaxDepth   = src.maxDepth;
    }

    context_->RSSetViewports(static_cast<UINT>(viewportsD3D.size()), viewportsD3D.data());
}

void D3D11StateManager::SetScissors(const std::vector<Scissor>& scissors)
{
    std::vector<D3D11_RECT> scissorsD3D(scissors.size());
    
    for (std::size_t i = 0, n = scissors.size(); i < n; ++i)
    {
        const auto& src = scissors[i];
        auto& dest = scissorsD3D[i];

        dest.left   = src.x;
        dest.top    = src.y;
        dest.right  = src.x + src.width;
        dest.bottom = src.y + src.height;
    }

    context_->RSSetScissorRects(static_cast<UINT>(scissorsD3D.size()), scissorsD3D.data());
}


} // /namespace LLGL



// ================================================================================
