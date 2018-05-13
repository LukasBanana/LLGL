/*
 * D3D11StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StateManager.h"
#include <algorithm>
#include <cstddef>


namespace LLGL
{


D3D11StateManager::D3D11StateManager(ComPtr<ID3D11DeviceContext>& context) :
    context_ { context }
{
}

void D3D11StateManager::SetViewports(std::uint32_t numViewports, const Viewport* viewportArray)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    /* Check if D3D11_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if ( sizeof(D3D11_VIEWPORT)             == sizeof(Viewport)             &&
         offsetof(D3D11_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
         offsetof(D3D11_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
         offsetof(D3D11_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
         offsetof(D3D11_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
         offsetof(D3D11_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
         offsetof(D3D11_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        context_->RSSetViewports(numViewports, reinterpret_cast<const D3D11_VIEWPORT*>(viewportArray));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D11_VIEWPORT viewportsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewportArray[i];
            auto& dst       = viewportsD3D[i];

            dst.TopLeftX    = src.x;
            dst.TopLeftY    = src.y;
            dst.Width       = src.width;
            dst.Height      = src.height;
            dst.MinDepth    = src.minDepth;
            dst.MaxDepth    = src.maxDepth;
        }

        context_->RSSetViewports(numViewports, viewportsD3D);
    }
}

void D3D11StateManager::SetScissors(std::uint32_t numScissors, const Scissor* scissorArray)
{
    numScissors = std::min(numScissors, std::uint32_t(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    D3D11_RECT scissorsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    
    for (std::uint32_t i = 0; i < numScissors; ++i)
    {
        const auto& src = scissorArray[i];
        auto& dst       = scissorsD3D[i];

        dst.left        = src.x;
        dst.top         = src.y;
        dst.right       = src.x + src.width;
        dst.bottom      = src.y + src.height;
    }
    
    context_->RSSetScissorRects(numScissors, scissorsD3D);
}


} // /namespace LLGL



// ================================================================================
