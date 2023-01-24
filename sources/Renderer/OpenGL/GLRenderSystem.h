/*
 * GLRenderSystem.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RENDER_SYSTEM_H
#define LLGL_GL_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "Ext/GLExtensionLoader.h"
#include "../ContainerTypes.h"

#include "Command/GLCommandQueue.h"
#include "Command/GLCommandBuffer.h"
#include "GLSwapChain.h"
#include "Platform/GLContextManager.h"

#include "Buffer/GLBuffer.h"
#include "Buffer/GLBufferArray.h"

#include "Shader/GLShader.h"
#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLSampler.h"
#include "Texture/GLRenderTarget.h"
#ifdef LLGL_GL_ENABLE_OPENGL2X
#   include "Texture/GL2XSampler.h"
#endif

#include "RenderState/GLQueryHeap.h"
#include "RenderState/GLFence.h"
#include "RenderState/GLRenderPass.h"
#include "RenderState/GLPipelineLayout.h"
#include "RenderState/GLPipelineState.h"
#include "RenderState/GLResourceHeap.h"

#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


class GLRenderSystem final : public RenderSystem
{

    public:

        /* ----- Common ----- */

        GLRenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~GLRenderSystem();

        /* ----- Swap-chain ----- */

        SwapChain* CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(SwapChain& swapChain) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc = {}) override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize) override;
        void ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize) override;

        void* MapBuffer(Buffer& buffer, const CPUAccess access) override;
        void* MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        void WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc) override;
        void ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& samplerDesc) override;

        void Release(Sampler& sampler) override;

        /* ----- Resource Heaps ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc) override;

        void Release(ResourceHeap& resourceHeap) override;

        /* ----- Render Passes ----- */

        RenderPass* CreateRenderPass(const RenderPassDescriptor& renderPassDesc) override;

        void Release(RenderPass& renderPass) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderDescriptor& shaderDesc) override;

        void Release(Shader& shader) override;

        /* ----- Pipeline Layouts ----- */

        PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc) override;

        void Release(PipelineLayout& pipelineLayout) override;

        /* ----- Pipeline States ----- */

        PipelineState* CreatePipelineState(const Blob& serializedCache) override;
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    protected:

        SwapChain* AddSwapChain(std::unique_ptr<GLSwapChain>&& swapChain);

    private:

        void CreateGLContextDependentDevices(GLStateManager& stateManager);

        void SetDebugCallback(const DebugCallback& debugCallback);

        void QueryRendererInfo();
        void QueryRenderingCaps();

        GLBuffer* CreateGLBuffer(const BufferDescriptor& bufferDesc, const void* initialData);

        void ValidateGLTextureType(const TextureType type);

    private:

        /* ----- Hardware object containers ----- */

        GLContextManager                        contextMngr_;

        HWObjectContainer<GLSwapChain>          swapChains_;
        HWObjectInstance<GLCommandQueue>        commandQueue_;
        HWObjectContainer<GLCommandBuffer>      commandBuffers_;
        HWObjectContainer<GLBuffer>             buffers_;
        HWObjectContainer<GLBufferArray>        bufferArrays_;
        HWObjectContainer<GLTexture>            textures_;
        HWObjectContainer<GLSampler>            samplers_;
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        HWObjectContainer<GL2XSampler>          samplersGL2X_;
        #endif
        HWObjectContainer<GLRenderPass>         renderPasses_;
        HWObjectContainer<GLRenderTarget>       renderTargets_;
        HWObjectContainer<GLShader>             shaders_;
        HWObjectContainer<GLPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<GLPipelineState>      pipelineStates_;
        HWObjectContainer<GLResourceHeap>       resourceHeaps_;
        HWObjectContainer<GLQueryHeap>          queryHeaps_;
        HWObjectContainer<GLFence>              fences_;

        DebugCallback                           debugCallback_;

};


} // /namespace LLGL


#endif



// ================================================================================
