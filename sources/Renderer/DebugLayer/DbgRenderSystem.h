/*
 * DbgRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_RENDER_SYSTEM_H__
#define __LLGL_DBG_RENDER_SYSTEM_H__


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

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer() override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Hardware Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) override;
        TextureArray* CreateTextureArray(unsigned int numTextures, Texture* const * textureArray) override;

        void Release(Texture& texture) override;
        void Release(TextureArray& textureArray) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;
        
        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;

        void Release(Sampler& sampler) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) override;

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

    private:

        //TODO: replace this by 'RendererInfo::rendererID'
        void DetermineRenderer(const std::string& rendererName);

        void DebugBufferSize(std::size_t bufferSize, std::size_t dataSize, std::size_t dataOffset, const std::string& source);
        void DebugMipLevelLimit(int mipLevel, int mipLevelCount, const std::string& source);

        void DebugTextureDescriptor(const TextureDescriptor& desc, const std::string& source);
        void DebugTextureSize(unsigned int size, const std::string& source);
        void WarnTextureLayersGreaterOne(const std::string& source);
        void ErrTextureLayersEqualZero(const std::string& source);

        template <typename T, typename TBase>
        void ReleaseDbg(std::set<std::unique_ptr<T>>& cont, TBase& entry);

        /* ----- Common objects ----- */

        std::shared_ptr<RenderSystem>           instance_;

        RenderingProfiler*                      profiler_   = nullptr;
        RenderingDebugger*                      debugger_   = nullptr;

        struct Renderer
        {
            bool isOpenGL   = false;
            bool isDirect3D = false;
            bool isVulkan   = false;
        }
        renderer_;

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
