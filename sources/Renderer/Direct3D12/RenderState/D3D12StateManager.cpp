/*
 * D3D12StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12StateManager.h"
#include <algorithm>
#include <cstddef>


namespace LLGL
{


void D3D12StateManager::SetViewports(std::uint32_t numViewports, const Viewport* viewportArray)
{
    viewports_.resize(numViewports);
    
    /* Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if ( sizeof(D3D12_VIEWPORT)             == sizeof(Viewport)             &&
         offsetof(D3D12_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
         offsetof(D3D12_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
         offsetof(D3D12_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
         offsetof(D3D12_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
         offsetof(D3D12_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
         offsetof(D3D12_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        auto viewportsD3D = reinterpret_cast<const D3D12_VIEWPORT*>(viewportArray);
        std::copy(viewportsD3D, viewportsD3D + numViewports, viewports_.data());
    }
    else
    {
        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewportArray[i];
            auto& dest = viewports_[i];

            dest.TopLeftX   = src.x;
            dest.TopLeftY   = src.y;
            dest.Width      = src.width;
            dest.Height     = src.height;
            dest.MinDepth   = src.minDepth;
            dest.MaxDepth   = src.maxDepth;
        }
    }
}

void D3D12StateManager::SubmitViewports(ID3D12GraphicsCommandList* commandList)
{
    if (!viewports_.empty())
        commandList->RSSetViewports(static_cast<UINT>(viewports_.size()), viewports_.data());
}

void D3D12StateManager::SetScissors(std::uint32_t numScissors, const Scissor* scissorArray)
{
    scissors_.resize(numScissors);
    
    for (std::uint32_t i = 0; i < numScissors; ++i)
    {
        const auto& src = scissorArray[i];
        auto& dest = scissors_[i];

        dest.left   = src.x;
        dest.top    = src.y;
        dest.right  = src.x + src.width;
        dest.bottom = src.y + src.height;
    }
}

void D3D12StateManager::SubmitScissors(ID3D12GraphicsCommandList* commandList)
{
    if (!scissors_.empty())
        commandList->RSSetScissorRects(static_cast<UINT>(scissors_.size()), scissors_.data());
}


} // /namespace LLGL



// ================================================================================
