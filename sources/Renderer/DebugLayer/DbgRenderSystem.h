/*
 * DbgRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_SYSTEM_H
#define LLGL_DBG_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "DbgRenderContext.h"
#include "DbgCommandBuffer.h"
#include "DbgCommandQueue.h"

#include "DbgBuffer.h"
#include "DbgBufferArray.h"
#include "DbgGraphicsPipeline.h"
#include "DbgTexture.h"
#include "DbgRenderTarget.h"
#include "DbgShader.h"
#include "DbgShaderProgram.h"
#include "DbgQueryHeap.h"

#include "../ContainerTypes.h"


namespace LLGL
{


class DbgRenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        DbgRenderSystem(const std::shared_ptr<RenderSystem>& instance, RenderingProfiler* profiler, RenderingDebugger* debugger);

        void SetConfiguration(const RenderSystemConfiguration& config) override;

        /* ----- Render Context ------ */

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

        /* ----- Resource Views ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& desc) override;

        void Release(ResourceHeap& resourceViewHeap) override;

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

        void ValidateBufferDesc(const BufferDescriptor& desc, std::uint32_t* formatSize = nullptr);
        void ValidateBufferSize(std::uint64_t size);
        void ValidateConstantBufferSize(std::uint64_t size);
        void ValidateBufferBoundary(std::uint64_t bufferSize, std::uint64_t dstOffset, std::uint64_t dataSize);
        void ValidateBufferCPUAccess(DbgBuffer& bufferDbg, const CPUAccess access);
        void ValidateBufferMapping(DbgBuffer& bufferDbg, bool mapMemory);

        void ValidateTextureDesc(const TextureDescriptor& desc);
        void ValidateTextureDescMipLevels(const TextureDescriptor& desc);
        void ValidateTextureSize(std::uint32_t size, std::uint32_t limit, const char* textureTypeName);
        void ValidateTextureSizeDefault(std::uint32_t size);
        void Validate1DTextureSize(std::uint32_t size);
        void Validate2DTextureSize(std::uint32_t size);
        void Validate3DTextureSize(std::uint32_t size);
        void ValidateCubeTextureSize(std::uint32_t width, std::uint32_t height);
        void ValidateArrayTextureLayers(const TextureType type, std::uint32_t layers);
        void ValidateMipLevelLimit(std::uint32_t mipLevel, std::uint32_t mipLevelCount);
        void ValidateTextureImageDataSize(std::size_t dataSize, std::size_t requiredDataSize);
        bool ValidateTextureMips(const DbgTexture& textureDbg);
        void ValidateTextureMipRange(const DbgTexture& textureDbg, std::uint32_t baseMipLevel, std::uint32_t numMipLevels);
        void ValidateTextureArrayRange(const DbgTexture& textureDbg, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers);
        void ValidateTextureArrayRangeWithEnd(std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, std::uint32_t arrayLayerLimit);

        void ValidateAttachmentDesc(const AttachmentDescriptor& desc);

        void ValidateBlendDescriptor(const BlendDescriptor& desc);
        void ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& desc);
        void ValidatePrimitiveTopology(const PrimitiveTopology primitiveTopology);

        void Assert3DTextures();
        void AssertCubeTextures();
        void AssertArrayTextures();
        void AssertCubeArrayTextures();
        void AssertMultiSampleTextures();

        template <typename T, typename TBase>
        void ReleaseDbg(std::set<std::unique_ptr<T>>& cont, TBase& entry);

        /* ----- Common objects ----- */

        std::shared_ptr<RenderSystem>           instance_;

        RenderingProfiler*                      profiler_   = nullptr;
        RenderingDebugger*                      debugger_   = nullptr;

        const RenderingCapabilities&            caps_;
        const RenderingFeatures&                features_;
        const RenderingLimits&                  limits_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<DbgRenderContext>     renderContexts_;
        HWObjectInstance<DbgCommandQueue>       commandQueue_;
        HWObjectContainer<DbgCommandBuffer>     commandBuffers_;
        HWObjectContainer<DbgBuffer>            buffers_;
        HWObjectContainer<DbgBufferArray>       bufferArrays_;
        HWObjectContainer<DbgTexture>           textures_;
        //HWObjectContainer<DbgRenderPass>        renderPasses_;
        HWObjectContainer<DbgRenderTarget>      renderTargets_;
        HWObjectContainer<DbgShader>            shaders_;
        HWObjectContainer<DbgShaderProgram>     shaderPrograms_;
        HWObjectContainer<DbgGraphicsPipeline>  graphicsPipelines_;
        //HWObjectContainer<DbgComputePipeline>   computePipelines_;
        //HWObjectContainer<DbgSampler>           samplers_;
        HWObjectContainer<DbgQueryHeap>         queryHeaps_;

};


} // /namespace LLGL


#endif



// ================================================================================
