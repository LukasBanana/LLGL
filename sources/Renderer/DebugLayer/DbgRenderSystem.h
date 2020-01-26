/*
 * DbgRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
#include "DbgPipelineLayout.h"
#include "DbgPipelineState.h"
#include "DbgTexture.h"
#include "DbgRenderTarget.h"
#include "DbgShader.h"
#include "DbgShaderProgram.h"
#include "DbgQueryHeap.h"
#include "DbgResourceHeap.h"

#include "../ContainerTypes.h"


namespace LLGL
{


//TODO: move all validation functions into spearate class to make them sharable between RenderSystem and other classes
class DbgRenderSystem final : public RenderSystem
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
        void ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc) override;

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

        PipelineState* CreatePipelineState(const Blob& serializedCache) override;
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& desc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        void ValidateBindFlags(long flags);
        void ValidateCPUAccessFlags(long flags, long validFlags, const char* contextDesc = nullptr);
        void ValidateMiscFlags(long flags, long validFlags, const char* contextDesc = nullptr);
        void ValidateResourceCPUAccess(long cpuAccessFlags, const CPUAccess access, const char* resourceTypeName);

        void ValidateBufferDesc(const BufferDescriptor& desc, std::uint32_t* formatSizeOut = nullptr);
        void ValidateVertexAttributesForBuffer(const VertexAttribute& lhs, const VertexAttribute& rhs);
        void ValidateBufferSize(std::uint64_t size);
        void ValidateConstantBufferSize(std::uint64_t size);
        void ValidateBufferBoundary(std::uint64_t bufferSize, std::uint64_t dstOffset, std::uint64_t dataSize);
        void ValidateBufferMapping(DbgBuffer& bufferDbg, bool mapMemory);

        void ValidateTextureDesc(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc = nullptr);
        void ValidateTextureFormatSupported(const Format format);
        void ValidateTextureDescMipLevels(const TextureDescriptor& desc);
        void ValidateTextureSize(std::uint32_t size, std::uint32_t limit, const char* textureTypeName);
        void ValidateTextureSizeDefault(std::uint32_t size);
        void Validate1DTextureSize(std::uint32_t size);
        void Validate2DTextureSize(std::uint32_t size);
        void Validate3DTextureSize(std::uint32_t size);
        void ValidateCubeTextureSize(std::uint32_t width, std::uint32_t height);
        void ValidateArrayTextureLayers(const TextureType type, std::uint32_t layers);
        void ValidateMipLevelLimit(std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t maxNumMipLevels);
        void ValidateTextureArrayRange(const DbgTexture& textureDbg, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers);
        void ValidateTextureArrayRangeWithEnd(std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, std::uint32_t arrayLayerLimit);
        void ValidateTextureRegion(const DbgTexture& textureDbg, const TextureRegion& textureRegion);
        void ValidateTextureView(const DbgTexture& sharedTextureDbg, const TextureViewDescriptor& desc);
        void ValidateTextureViewType(const TextureType sharedTextureType, const TextureType textureViewType, const std::initializer_list<TextureType>& validTypes);
        void ValidateImageDataSize(const DbgTexture& textureDbg, const TextureRegion& textureRegion, ImageFormat imageFormat, DataType dataType, std::size_t dataSize);

        void ValidateAttachmentDesc(const AttachmentDescriptor& desc);

        void ValidateResourceHeapDesc(const ResourceHeapDescriptor& desc);
        void ValidateResourceViewForBinding(const ResourceViewDescriptor& rvDesc, const BindingDescriptor& bindingDesc);
        void ValidateBufferForBinding(const DbgBuffer& bufferDbg, const BindingDescriptor& bindingDesc);
        void ValidateTextureForBinding(const DbgTexture& textureDbg, const BindingDescriptor& bindingDesc);

        void ValidateColorMaskIsDisabled(const BlendTargetDescriptor& desc, std::size_t idx);
        void ValidateBlendDescriptor(const BlendDescriptor& desc, bool hasFragmentShader);
        void ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& desc);
        void ValidatePrimitiveTopology(const PrimitiveTopology primitiveTopology);

        void Assert3DTextures();
        void AssertCubeTextures();
        void AssertArrayTextures();
        void AssertCubeArrayTextures();
        void AssertMultiSampleTextures();

        template <typename T, typename TBase>
        void ReleaseDbg(std::set<std::unique_ptr<T>>& cont, TBase& entry);

    private:

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
        HWObjectContainer<DbgPipelineLayout>    pipelineLayouts_;
        HWObjectContainer<DbgPipelineState>     pipelineStates_;
        HWObjectContainer<DbgResourceHeap>      resourceHeaps_;
        //HWObjectContainer<DbgSampler>           samplers_;
        HWObjectContainer<DbgQueryHeap>         queryHeaps_;

};


} // /namespace LLGL


#endif



// ================================================================================
