/*
 * D3D9RenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_RENDER_SYSTEM_H
#define LLGL_D3D9_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "D3D9SwapChain.h"
#include "Command/D3D9CommandBuffer.h"
#include "Command/D3D9CommandQueue.h"
#include "Buffer/D3D9Buffer.h"
#include "Buffer/D3D9BufferArray.h"
#include "RenderState/D3D9Fence.h"
#include "RenderState/D3D9PipelineLayout.h"
#include "RenderState/D3D9PipelineState.h"
#include "RenderState/D3D9QueryHeap.h"
#include "RenderState/D3D9ResourceHeap.h"
#include "RenderState/D3D9RenderPass.h"
#include "RenderState/D3D9StateManager.h"
#include "Shader/D3D9Shader.h"
#include "Texture/D3D9Texture.h"
#include "Texture/D3D9RenderTarget.h"
#include "Texture/D3D9Sampler.h"
#include "../ProxyPipelineCache.h"

#include "../ContainerTypes.h"
#include "../DXCommon/ComPtr.h"

#include "Direct3D9.h"


namespace LLGL
{


class D3D9RenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        D3D9RenderSystem(const RenderSystemDescriptor& renderSystemDesc);

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void CreateDevice();

        HWND CreateFocusWindow();

    private:

        /* ----- Common objects ----- */

        const RenderSystemDescriptor            desc_;

        ComPtr<IDirect3D9>                      direct3d_;
        ComPtr<IDirect3DDevice9>                device_;

        std::unique_ptr<D3D9StateManager>       stateMngr_;

        std::unique_ptr<Surface>                focusWnd_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D9SwapChain>        swapChains_;
        HWObjectInstance<D3D9CommandQueue>      commandQueue_;
        HWObjectContainer<D3D9CommandBuffer>    commandBuffers_;
        HWObjectContainer<D3D9Buffer>           buffers_;
        HWObjectContainer<D3D9BufferArray>      bufferArrays_;
        HWObjectContainer<D3D9Texture>          textures_;
        HWObjectContainer<D3D9RenderPass>       renderPasses_;
        HWObjectContainer<D3D9RenderTarget>     renderTargets_;
        HWObjectContainer<D3D9Shader>           shaders_;
        HWObjectContainer<D3D9PipelineLayout>   pipelineLayouts_;
        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;
        HWObjectContainer<D3D9PipelineState>    pipelineStates_;
        HWObjectContainer<D3D9ResourceHeap>     resourceHeaps_;
        HWObjectContainer<D3D9Sampler>          samplers_;
        HWObjectContainer<D3D9QueryHeap>        queryHeaps_;
        HWObjectContainer<D3D9Fence>            fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
