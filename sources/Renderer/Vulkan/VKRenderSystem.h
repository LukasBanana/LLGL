/*
 * VKRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_RENDER_SYSTEM_H
#define LLGL_VK_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "VKPhysicalDevice.h"
#include "VKDevice.h"
#include "../ContainerTypes.h"
#include "Memory/VKDeviceMemoryManager.h"

#include "Command/VKCommandQueue.h"
#include "Command/VKCommandBuffer.h"
#include "Command/VKCommandContext.h"
#include "VKSwapChain.h"

#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"

#include "Shader/VKShader.h"

#include "Texture/VKTexture.h"
#include "Texture/VKSampler.h"
#include "Texture/VKRenderTarget.h"

#include "RenderState/VKQueryHeap.h"
#include "RenderState/VKFence.h"
#include "RenderState/VKRenderPass.h"
#include "RenderState/VKPipelineLayout.h"
#include "RenderState/VKPipelineCache.h"
#include "RenderState/VKGraphicsPSO.h"
#include "RenderState/VKResourceHeap.h"

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <tuple>


namespace LLGL
{


class VKRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~VKRenderSystem();

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void CreateInstance(const RendererConfigurationVulkan* config);
        void CreateDebugReportCallback();
        bool PickPhysicalDevice(long preferredDeviceFlags, VkPhysicalDevice customPhysicalDevice = VK_NULL_HANDLE);
        void CreateLogicalDevice(VkDevice customLogicalDevice = VK_NULL_HANDLE);

        bool IsLayerRequired(const char* name, const RendererConfigurationVulkan* config) const;

        VKDeviceBuffer CreateStagingBuffer(const VkBufferCreateInfo& createInfo);

        VKDeviceBuffer CreateStagingBufferAndInitialize(
            const VkBufferCreateInfo&   createInfo,
            const void*                 data,
            VkDeviceSize                dataSize
        );

        VKDeviceBuffer CreateTextureStagingBufferAndInitialize(
            const VkBufferCreateInfo&   createInfo,
            const void*                 data,
            VkDeviceSize                dataSize,
            const Extent3D&             extent,
            std::uint32_t               srcRowStride,
            std::uint32_t               bpp
        );

        VkCommandBuffer AllocCommandBuffer(bool begin = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);

    private:

        /* ----- Common objects ----- */

        VKPtr<VkInstance>                       instance_;

        VKPhysicalDevice                        physicalDevice_;
        VKDevice                                device_;
        VKCommandContext                        context_;

        bool                                    debugLayerEnabled_      = false;
        VKPtr<VkDebugReportCallbackEXT>         debugReportCallback_;

        std::unique_ptr<VKDeviceMemoryManager>  deviceMemoryMngr_;

        VKGraphicsPipelineLimits                graphicsPipelineLimits_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<VKSwapChain>          swapChains_;
        HWObjectInstance<VKCommandQueue>        commandQueue_;
        HWObjectContainer<VKCommandBuffer>      commandBuffers_;
        HWObjectContainer<VKBuffer>             buffers_;
        HWObjectContainer<VKBufferArray>        bufferArrays_;
        HWObjectContainer<VKTexture>            textures_;
        HWObjectContainer<VKSampler>            samplers_;
        HWObjectContainer<VKRenderPass>         renderPasses_;
        HWObjectContainer<VKRenderTarget>       renderTargets_;
        HWObjectContainer<VKShader>             shaders_;
        HWObjectContainer<VKPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<VKPipelineCache>      pipelineCaches_;
        HWObjectContainer<VKPipelineState>      pipelineStates_;
        HWObjectContainer<VKResourceHeap>       resourceHeaps_;
        HWObjectContainer<VKQueryHeap>          queryHeaps_;
        HWObjectContainer<VKFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
