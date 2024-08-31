/*
 * MTRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_RENDER_SYSTEM_H
#define LLGL_MT_RENDER_SYSTEM_H


#import <MetalKit/MetalKit.h>

#include <LLGL/RenderSystem.h>
#include "../ContainerTypes.h"
#include "../ProxyPipelineCache.h"

#include "Command/MTCommandQueue.h"
#include "Command/MTCommandBuffer.h"
#include "MTSwapChain.h"

#include "Buffer/MTBuffer.h"
#include "Buffer/MTBufferArray.h"
#include "Buffer/MTIntermediateBuffer.h"

#include "RenderState/MTPipelineLayout.h"
#include "RenderState/MTPipelineState.h"
#include "RenderState/MTQueryHeap.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTRenderPass.h"
#include "RenderState/MTFence.h"

#include "Shader/MTShader.h"

#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"

#include <memory>


namespace LLGL
{


class MTRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        MTRenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~MTRenderSystem();

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void CreateDeviceResources(id<MTLDevice> sharedDevice = nil);
        void QueryRendererInfo(RendererInfo& outInfo);

        const char* QueryMetalVersion() const;

        MTLFeatureSet QueryHighestFeatureSet() const;

        const MTRenderPass* GetDefaultRenderPass() const;

    private:

        /* ----- Common objects ----- */

        id<MTLDevice>                           device_             = nil;
        std::unique_ptr<MTIntermediateBuffer>   intermediateBuffer_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<MTSwapChain>          swapChains_;
        HWObjectInstance<MTCommandQueue>        commandQueue_;
        HWObjectContainer<MTCommandBuffer>      commandBuffers_;
        HWObjectContainer<MTBuffer>             buffers_;
        HWObjectContainer<MTBufferArray>        bufferArrays_;
        HWObjectContainer<MTTexture>            textures_;
        HWObjectContainer<MTSampler>            samplers_;
        HWObjectContainer<MTRenderPass>         renderPasses_;
        HWObjectContainer<MTRenderTarget>       renderTargets_;
        HWObjectContainer<MTShader>             shaders_;
        HWObjectContainer<MTPipelineLayout>     pipelineLayouts_;
        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;
        HWObjectContainer<MTPipelineState>  	pipelineStates_;
        HWObjectContainer<MTResourceHeap>       resourceHeaps_;
        HWObjectContainer<MTQueryHeap>          queryHeaps_;
        HWObjectContainer<MTFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
