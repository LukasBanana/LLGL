/*
 * DbgRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_SYSTEM_H
#define LLGL_DBG_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "DbgRenderContext.h"
#include "DbgCommandBuffer.h"

#include "DbgBuffer.h"
#include "DbgGraphicsPipeline.h"
#include "DbgTexture.h"
#include "DbgRenderTarget.h"
#include "DbgShader.h"
#include "DbgShaderProgram.h"
#include "DbgQuery.h"

#include "../ContainerTypes.h"


namespace LLGL
{


class DbgRenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        DbgRenderSystem(const std::shared_ptr<RenderSystem>& instance, RenderingProfiler* profiler, RenderingDebugger* debugger);
        ~DbgRenderSystem();

        void SetConfiguration(const RenderSystemConfiguration& config) override;

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer() override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) override;
        TextureArray* CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray) override;

        void Release(Texture& texture) override;
        void Release(TextureArray& textureArray) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;
        
        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;
        SamplerArray* CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray) override;

        void Release(Sampler& sampler) override;
        void Release(SamplerArray& samplerArray) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderType type) override;
        ShaderProgram* CreateShaderProgram() override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;

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

        void DebugBufferSize(std::uint64_t bufferSize, std::size_t dataSize, std::size_t dataOffset);
        void DebugMipLevelLimit(int mipLevel, int mipLevelCount);

        void DebugTextureDescriptor(const TextureDescriptor& desc);
        void DebugTextureSize(std::uint32_t size);
        void WarnTextureLayersGreaterOne();
        void ErrTextureLayersEqualZero();

        template <typename T, typename TBase>
        void ReleaseDbg(std::set<std::unique_ptr<T>>& cont, TBase& entry);

        /* ----- Common objects ----- */

        std::shared_ptr<RenderSystem>           instance_;

        RenderingProfiler*                      profiler_   = nullptr;
        RenderingDebugger*                      debugger_   = nullptr;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<DbgRenderContext>     renderContexts_;
        HWObjectContainer<DbgCommandBuffer>     commandBuffers_;
        HWObjectContainer<DbgBuffer>            buffers_;
        HWObjectContainer<DbgTexture>           textures_;
        HWObjectContainer<DbgRenderTarget>      renderTargets_;
        HWObjectContainer<DbgShader>            shaders_;
        HWObjectContainer<DbgShaderProgram>     shaderPrograms_;
        HWObjectContainer<DbgGraphicsPipeline>  graphicsPipelines_;
        //HWObjectContainer<DbgComputePipeline>   computePipelines_;
        //HWObjectContainer<DbgSampler>           samplers_;
        HWObjectContainer<DbgQuery>             queries_;

};


} // /namespace LLGL


#endif



// ================================================================================
