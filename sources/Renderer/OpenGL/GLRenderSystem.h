/*
 * GLRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RENDER_SYSTEM_H
#define LLGL_GL_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "Ext/GLExtensionLoader.h"
#include "../ContainerTypes.h"

#include "GLCommandQueue.h"
#include "GLCommandBuffer.h"
#include "GLRenderContext.h"

#include "Buffer/GLBuffer.h"
#include "Buffer/GLBufferArray.h"

#include "Shader/GLShader.h"
#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLSampler.h"
#include "Texture/GLRenderTarget.h"

#include "RenderState/GLQueryHeap.h"
#include "RenderState/GLFence.h"
#include "RenderState/GLRenderPass.h"
#include "RenderState/GLPipelineLayout.h"
#include "RenderState/GLGraphicsPipeline.h"
#include "RenderState/GLComputePipeline.h"
#include "RenderState/GLResourceHeap.h"

#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


//TODO: performance tests show that the manual MIP-map generation is almost always slower than the default MIP-map generation
//#define LLGL_ENABLE_CUSTOM_SUB_MIPGEN

class GLRenderSystem final : public RenderSystem
{

    public:

        /* ----- Common ----- */

        GLRenderSystem();

        void SetConfiguration(const RenderSystemConfiguration& config) override;

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

    protected:

        RenderContext* AddRenderContext(std::unique_ptr<GLRenderContext>&& renderContext, const RenderContextDescriptor& desc);

    private:

        void LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc);
        void SetDebugCallback(const DebugCallback& debugCallback);

        void QueryRendererInfo();
        void QueryRenderingCaps();

        GLRenderContext* GetSharedRenderContext() const;

        GLBuffer* CreateGLBuffer(const BufferDescriptor& desc, const void* initialData);

        void GenerateMipsPrimary(GLuint texID, const TextureType texType);
        void GenerateSubMipsWithFBO(GLTexture& textureGL, const Extent3D& extent, GLint baseMipLevel, GLint numMipLevels, GLint baseArrayLayer, GLint numArrayLayers);
        void GenerateSubMipsWithTextureView(GLTexture& textureGL, GLuint baseMipLevel, GLuint numMipLevels, GLuint baseArrayLayer, GLuint numArrayLayers);

        #ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN
        struct MipGenerationFBOPair
        {
            ~MipGenerationFBOPair();

            void CreateFBOs();
            void ReleaseFBOs();

            GLuint fbos[2] = { 0, 0 };
        };
        #endif // /LLGL_ENABLE_CUSTOM_SUB_MIPGEN

        /* ----- Hardware object containers ----- */

        HWObjectContainer<GLRenderContext>      renderContexts_;
        HWObjectInstance<GLCommandQueue>        commandQueue_;
        HWObjectContainer<GLCommandBuffer>      commandBuffers_;
        HWObjectContainer<GLBuffer>             buffers_;
        HWObjectContainer<GLBufferArray>        bufferArrays_;
        HWObjectContainer<GLTexture>            textures_;
        HWObjectContainer<GLSampler>            samplers_;
        HWObjectContainer<GLRenderPass>         renderPasses_;
        HWObjectContainer<GLRenderTarget>       renderTargets_;
        HWObjectContainer<GLShader>             shaders_;
        HWObjectContainer<GLShaderProgram>      shaderPrograms_;
        HWObjectContainer<GLPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<GLGraphicsPipeline>   graphicsPipelines_;
        HWObjectContainer<GLComputePipeline>    computePipelines_;
        HWObjectContainer<GLResourceHeap>       resourceHeaps_;
        HWObjectContainer<GLQueryHeap>          queryHeaps_;
        HWObjectContainer<GLFence>              fences_;

        DebugCallback                           debugCallback_;

        #ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN
        MipGenerationFBOPair                    mipGenerationFBOPair_;
        #endif // /LLGL_ENABLE_CUSTOM_SUB_MIPGEN

};


} // /namespace LLGL


#endif



// ================================================================================
