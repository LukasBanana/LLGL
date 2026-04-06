/*
 * WGRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RENDER_SYSTEM_H
#define LLGL_WG_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>

#include "WGSwapChain.h"
#include "Command/WGCommandBuffer.h"
#include "Command/WGCommandQueue.h"

#include "Buffer/WGBuffer.h"
#include "Buffer/WGBufferArray.h"

#include "RenderState/WGFence.h"
#include "RenderState/WGPipelineLayout.h"
#include "RenderState/WGPipelineState.h"
#include "RenderState/WGQueryHeap.h"
#include "RenderState/WGResourceHeap.h"
#include "RenderState/WGRenderPass.h"

#include "Shader/WGShader.h"

#include "Texture/WGTexture.h"
#include "Texture/WGRenderTarget.h"
#include "Texture/WGSampler.h"

#include "../ProxyPipelineCache.h"
#include "../ContainerTypes.h"

#include <webgpu/webgpu.h>


namespace LLGL
{


class WGRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        WGRenderSystem(const RenderSystemDescriptor& desc);
        ~WGRenderSystem();

    public:

        // Returns the native WebGPU instance object.
        inline WGPUInstance GetNativeInstance() const
        {
            return instance_;
        }

        // Returns the native WebGPU device object.
        inline WGPUDevice GetNativeDevice() const
        {
            return device_;
        }

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void CreateWebGpuInstance();
        bool RequestWebGpuAdapter();
        bool RequestWebGpuDevice(long renderSystemFlags);
        void CreateCommandQueue();

    private:

        /* ----- Common objects ----- */

        WGPUInstance                            instance_           = nullptr;
        WGPUAdapter                             adapter_            = nullptr;
        WGPUDevice                              device_             = nullptr;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<WGSwapChain>          swapChains_;
        HWObjectInstance<WGCommandQueue>        commandQueue_;
        HWObjectContainer<WGCommandBuffer>      commandBuffers_;
        HWObjectContainer<WGBuffer>             buffers_;
        HWObjectContainer<WGBufferArray>        bufferArrays_;
        HWObjectContainer<WGTexture>            textures_;
        HWObjectContainer<WGRenderPass>         renderPasses_;
        HWObjectContainer<WGRenderTarget>       renderTargets_;
        HWObjectContainer<WGShader>             shaders_;
        HWObjectContainer<WGPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<WGPipelineState>      pipelineStates_;
        HWObjectContainer<WGResourceHeap>       resourceHeaps_;
        HWObjectContainer<WGSampler>            samplers_;
        HWObjectContainer<WGQueryHeap>          queryHeaps_;
        HWObjectContainer<WGFence>              fences_;

        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;


};


} // /namespace LLGL


#endif



// ================================================================================
