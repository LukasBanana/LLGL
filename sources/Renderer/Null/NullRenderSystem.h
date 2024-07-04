/*
 * NullRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_RENDER_SYSTEM_H
#define LLGL_NULL_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "NullSwapChain.h"
#include "Command/NullCommandBuffer.h"
#include "Command/NullCommandQueue.h"
#include "Buffer/NullBuffer.h"
#include "Buffer/NullBufferArray.h"
#include "RenderState/NullFence.h"
#include "RenderState/NullPipelineLayout.h"
#include "RenderState/NullPipelineState.h"
#include "RenderState/NullQueryHeap.h"
#include "RenderState/NullResourceHeap.h"
#include "RenderState/NullRenderPass.h"
#include "Shader/NullShader.h"
#include "Texture/NullTexture.h"
#include "Texture/NullRenderTarget.h"
#include "Texture/NullSampler.h"
#include "../ProxyPipelineCache.h"

#include "../ContainerTypes.h"


namespace LLGL
{


class NullRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        NullRenderSystem(const RenderSystemDescriptor& renderSystemDesc);

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        /* ----- Common objects ----- */

        const RenderSystemDescriptor            desc_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<NullSwapChain>        swapChains_;
        HWObjectInstance<NullCommandQueue>      commandQueue_;
        HWObjectContainer<NullCommandBuffer>    commandBuffers_;
        HWObjectContainer<NullBuffer>           buffers_;
        HWObjectContainer<NullBufferArray>      bufferArrays_;
        HWObjectContainer<NullTexture>          textures_;
        HWObjectContainer<NullRenderPass>       renderPasses_;
        HWObjectContainer<NullRenderTarget>     renderTargets_;
        HWObjectContainer<NullShader>           shaders_;
        HWObjectContainer<NullPipelineLayout>   pipelineLayouts_;
        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;
        HWObjectContainer<NullPipelineState>    pipelineStates_;
        HWObjectContainer<NullResourceHeap>     resourceHeaps_;
        HWObjectContainer<NullSampler>          samplers_;
        HWObjectContainer<NullQueryHeap>        queryHeaps_;
        HWObjectContainer<NullFence>            fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
