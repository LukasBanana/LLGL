/*
 * D3D9StatePool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_STATE_POOL_H
#define LLGL_D3D9_STATE_POOL_H


#include "D3D9DepthStencilState.h"
#include "D3D9RasterizerState.h"
#include "D3D9BlendState.h"
#include <vector>


namespace LLGL
{


// Singleton pool for D3D9 depth-stencil-, rasterizer-, and blend states.
class D3D9StatePool
{

    public:

        D3D9StatePool(const D3D9StatePool&) = delete;
        D3D9StatePool& operator = (const D3D9StatePool&) = delete;

        // Returns the instance of this pool.
        static D3D9StatePool& Get();

        // Clear all resource containers of this pool (used by D3D9RenderSystem).
        void Clear();

        /* ----- Depth-stencil states ----- */

        D3D9DepthStencilStateSPtr CreateDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc);
        void ReleaseDepthStencilState(D3D9DepthStencilStateSPtr&& depthStencilState);

        /* ----- Rasterizer states ----- */

        D3D9RasterizerStateSPtr CreateRasterizerState(const RasterizerDescriptor& rasterizerDesc);
        void ReleaseRasterizerState(D3D9RasterizerStateSPtr&& rasterizerState);

        /* ----- Blend states ----- */

        D3D9BlendStateSPtr CreateBlendState(const BlendDescriptor& blendDesc);
        void ReleaseBlendState(D3D9BlendStateSPtr&& blendState);

    private:

        D3D9StatePool() = default;

    private:

        std::vector<D3D9DepthStencilStateSPtr>  depthStencilStates_;
        std::vector<D3D9RasterizerStateSPtr>    rasterizerStates_;
        std::vector<D3D9BlendStateSPtr>         blendStates_;

};


} // /namespace LLGL


#endif



// ================================================================================
