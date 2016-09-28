/*
 * D3D11StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StateManager.h"
#include <cstddef>


namespace LLGL
{


D3D11StateManager::D3D11StateManager(ComPtr<ID3D11DeviceContext>& context) :
    context_( context )
{
}

void D3D11StateManager::SetViewports(std::size_t numViewports, const Viewport* viewports)
{
    /* Check (at compile time) if D3D11_VIEWPORT and Viewport structures can be safly reinterpret-casted */
    if ( sizeof(D3D11_VIEWPORT)             == sizeof(Viewport)             &&
         offsetof(D3D11_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
         offsetof(D3D11_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
         offsetof(D3D11_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
         offsetof(D3D11_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
         offsetof(D3D11_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
         offsetof(D3D11_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        context_->RSSetViewports(static_cast<UINT>(numViewports), reinterpret_cast<const D3D11_VIEWPORT*>(viewports));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        std::vector<D3D11_VIEWPORT> viewportsD3D(numViewports);
    
        for (std::size_t i = 0; i < numViewports; ++i)
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
}

void D3D11StateManager::SetScissors(std::size_t numScissors, const Scissor* scissors)
{
    std::vector<D3D11_RECT> scissorsD3D(numScissors);
    
    for (std::size_t i = 0; i < numScissors; ++i)
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
