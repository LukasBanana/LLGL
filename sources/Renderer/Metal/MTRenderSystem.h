/*
 * MTRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_RENDER_SYSTEM_H
#define LLGL_MT_RENDER_SYSTEM_H


#import <MetalKit/MetalKit.h>

#include <LLGL/RenderSystem.h>
#include "../ContainerTypes.h"

#include "MTCommandQueue.h"
#include "MTCommandBuffer.h"
#include "MTRenderContext.h"

#include "Buffer/MTBuffer.h"
#include "Buffer/MTBufferArray.h"

#include "RenderState/MTGraphicsPipeline.h"
#include "RenderState/MTComputePipeline.h"
#include "RenderState/MTPipelineLayout.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTRenderPass.h"
#include "RenderState/MTFence.h"

#include "Shader/MTShader.h"
#include "Shader/MTShaderProgram.h"

#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"


namespace LLGL
{


class MTRenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        MTRenderSystem();
        ~MTRenderSystem();

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

        void WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize) override;

        void* MapBuffer(Buffer& buffer, const CPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        void WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc) override;
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

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& desc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        void CreateDeviceResources();
        void QueryRenderingCaps();

    private:
    
        /* ----- Common objects ----- */

        id<MTLDevice>                           device_             = nil;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<MTRenderContext>      renderContexts_;
        HWObjectInstance<MTCommandQueue>        commandQueue_;
        HWObjectContainer<MTCommandBuffer>      commandBuffers_;
        HWObjectContainer<MTBuffer>             buffers_;
        HWObjectContainer<MTBufferArray>        bufferArrays_;
        HWObjectContainer<MTTexture>            textures_;
        HWObjectContainer<MTSampler>            samplers_;
        HWObjectContainer<MTRenderPass>         renderPasses_;
        HWObjectContainer<MTRenderTarget>       renderTargets_;
        HWObjectContainer<MTShader>             shaders_;
        HWObjectContainer<MTShaderProgram>      shaderPrograms_;
        HWObjectContainer<MTPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<MTGraphicsPipeline>   graphicsPipelines_;
        HWObjectContainer<MTComputePipeline>    computePipelines_;
        HWObjectContainer<MTResourceHeap>       resourceHeaps_;
        //HWObjectContainer<MTQueryHeap>          queryHeaps_;
        HWObjectContainer<MTFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
