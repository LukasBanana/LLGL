/*
 * VKRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_SYSTEM_H
#define LLGL_VK_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "VKPhysicalDevice.h"
#include "VKDevice.h"
#include "../ContainerTypes.h"
#include "Memory/VKDeviceMemoryManager.h"

#include "VKCommandQueue.h"
#include "VKCommandBuffer.h"
#include "VKRenderContext.h"

#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"

#include "Shader/VKShader.h"
#include "Shader/VKShaderProgram.h"

#include "Texture/VKTexture.h"
#include "Texture/VKSampler.h"
#include "Texture/VKRenderTarget.h"

#include "RenderState/VKQuery.h"
#include "RenderState/VKFence.h"
#include "RenderState/VKRenderPass.h"
#include "RenderState/VKPipelineLayout.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "RenderState/VKComputePipeline.h"
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

        /* ----- Common ----- */

        VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~VKRenderSystem();

        /* ----- Render Context ----- */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& desc = {}) override;
        CommandBufferExt* CreateCommandBufferExt(const CommandBufferDescriptor& desc = {}) override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        void* MapBuffer(Buffer& buffer, const CPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc) override;
        void ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer = 0, std::uint32_t numArrayLayers = 1) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;

        void Release(Sampler& sampler) override;

        /* ----- Resource Heaps ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& desc) override;

        void Release(ResourceHeap& resourceHeap) override;

        /* ----- Render Passes ----- */

        RenderPass* CreateRenderPass(const RenderPassDescriptor& desc) override;

        void Release(RenderPass& renderPass) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderDescriptor& desc) override;
        ShaderProgram* CreateShaderProgram(const ShaderProgramDescriptor& desc) override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;

        /* ----- Pipeline Layouts ----- */

        PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& desc) override;

        void Release(PipelineLayout& pipelineLayout) override;

        /* ----- Pipeline States ----- */

        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) override;
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;

        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryDescriptor& desc) override;

        void Release(Query& query) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        void CreateInstance(const ApplicationDescriptor* applicationDesc);
        void CreateDebugReportCallback();
        void LoadExtensions();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateDefaultPipelineLayout();

        bool IsLayerRequired(const std::string& name) const;
        bool IsExtensionRequired(const std::string& name) const;

        VKBuffer* CreateGpuBuffer(const BufferDescriptor& desc, VkBufferUsageFlags usage = 0);

        VKDeviceBuffer CreateStagingBuffer(
            const VkBufferCreateInfo&   createInfo,
            const void*                 initialData     = nullptr,
            std::size_t                 initialDataSize = 0
        );

        /* ----- Common objects ----- */

        VKPtr<VkInstance>                       instance_;

        VKPhysicalDevice                        physicalDevice_;
        VKDevice                                device_;

        VKPtr<VkDebugReportCallbackEXT>         debugReportCallback_;
        VKPtr<VkPipelineLayout>                 defaultPipelineLayout_;

        bool                                    debugLayerEnabled_      = false;

        std::unique_ptr<VKDeviceMemoryManager>  deviceMemoryMngr_;

        VKGraphicsPipelineLimits                gfxPipelineLimits_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<VKRenderContext>      renderContexts_;
        HWObjectInstance<VKCommandQueue>        commandQueue_;
        HWObjectContainer<VKCommandBuffer>      commandBuffers_;
        HWObjectContainer<VKBuffer>             buffers_;
        HWObjectContainer<VKBufferArray>        bufferArrays_;
        HWObjectContainer<VKTexture>            textures_;
        HWObjectContainer<VKSampler>            samplers_;
        HWObjectContainer<VKRenderPass>         renderPasses_;
        HWObjectContainer<VKRenderTarget>       renderTargets_;
        HWObjectContainer<VKShader>             shaders_;
        HWObjectContainer<VKShaderProgram>      shaderPrograms_;
        HWObjectContainer<VKPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<VKGraphicsPipeline>   graphicsPipelines_;
        HWObjectContainer<VKComputePipeline>    computePipelines_;
        HWObjectContainer<VKResourceHeap>       resourceHeaps_;
        HWObjectContainer<VKQuery>              queries_;
        HWObjectContainer<VKFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
