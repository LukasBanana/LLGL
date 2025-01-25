/*
 * DbgCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_COMMAND_BUFFER_H
#define LLGL_DBG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/RenderingDebugger.h>
#include <LLGL/Constants.h>
#include <LLGL/Container/ArrayView.h>
#include "RenderState/DbgQueryHeap.h"
#include "DbgQueryTimerPool.h"
#include <cstdint>
#include <string>
#include <stack>


namespace LLGL
{


class DbgBuffer;
class DbgTexture;
class DbgSwapChain;
class DbgRenderTarget;
class DbgPipelineState;
class DbgPipelineLayout;
class DbgShader;

class DbgCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        DbgCommandBuffer(
            RenderSystem&                   renderSystemInstance,
            CommandQueue&                   commandQueueInstance,
            CommandBuffer&                  commandBufferInstance,
            FrameProfile&                   commonProfile,
            RenderingDebugger*              debugger,
            const CommandBufferDescriptor&  desc,
            const RenderingCapabilities&    caps
        );

    public:

        void FlushProfile(FrameProfile& outProfile);

        void ValidateSubmit();

    public:

        CommandBuffer&                  instance;
        const CommandBufferDescriptor   desc;
        std::string                     label;

    private:

        struct BindingTable
        {
            ResourceHeap*           resourceHeap = nullptr;
            std::vector<Resource*>  resources;
            std::vector<char>       uniforms;
        };

        struct Bindings
        {
            // Framebuffers
            DbgSwapChain*       swapChain                                           = nullptr;
            DbgRenderTarget*    renderTarget                                        = nullptr;
            std::uint32_t       numViewports                                        = 0;
            bool                anyFragmentOutput                                   = false;

            // Stream inputs/outputs
            DbgBuffer*          vertexBufferStore[1]                                = {};
            DbgBuffer* const *  vertexBuffers                                       = nullptr;
            std::uint32_t       numVertexBuffers                                    = 0;
            bool                anyShaderAttributes                                 = false;
            DbgBuffer*          indexBuffer                                         = nullptr;
            std::uint64_t       indexBufferFormatSize                               = 0;
            std::uint64_t       indexBufferOffset                                   = 0;
            DbgBuffer*          streamOutputs[LLGL_MAX_NUM_SO_BUFFERS]              = {};
            std::uint32_t       numStreamOutputs                                    = 0;

            // PSO
            DbgPipelineState*   pipelineState                                       = nullptr;
            const DbgShader*    vertexShader                                        = nullptr;
            bool                blendFactorSet                                      = false;
            bool                stencilRefSet                                       = false;
            Scissor             scissorRects[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
            std::uint32_t       numScissorRects                                     = 0;
            BindingTable        bindingTable;
        };

        struct States
        {
            bool recording          = false;
            bool finishedRecording  = false;
            bool insideRenderPass   = false;
            bool streamOutputBusy   = false;
        };

        struct SwapChainFramePair
        {
            DbgSwapChain* swapChain;
            std::uint64_t frame;      // Frame index when the swap-chain render-pass was encoded
        };

        struct Records
        {
            std::vector<SwapChainFramePair> swapChainFrames;
        };

    private:

        void ValidateBeginOfRecording();
        void ValidateEndOfRecording();
        void ValidateCommandBufferForExecute(const States& cmdBufferStates, const char* cmdBufferName = nullptr);

        void ValidateGenerateMips(DbgTexture& textureDbg, const TextureSubresource* subresource = nullptr);
        void ValidateViewport(const Viewport& viewport);
        void ValidateAttachmentClear(const AttachmentClear& attachment);

        void ValidateVertexLayout();
        void ValidateVertexLayoutAttributes(const ArrayView<VertexAttribute>& shaderVertexAttribs, DbgBuffer* const * vertexBuffers, std::uint32_t numVertexBuffers);

        void ValidateNumVertices(std::uint32_t numVertices);
        void ValidateNumInstances(std::uint32_t numInstances);
        void ValidateVertexID(std::uint32_t firstVertex);
        void ValidateInstanceID(std::uint32_t firstInstance);

        void ValidateDrawCmd(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance);
        void ValidateDrawIndexedCmd(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance);
        void ValidateDrawStreamOutputCmd();

        void ValidateVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit);
        void ValidateThreadGroupLimit(std::uint32_t size, std::uint32_t limit);
        void ValidateAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound);
        void ValidateDescriptorSetIndex(std::uint32_t setIndex, std::uint32_t setUpperBound, const char* resourceHeapName = nullptr);

        void ValidateBindFlags(long resourceFlags, long bindFlags, long validFlags, const char* resourceName = nullptr);
        void ValidateBindBufferFlags(DbgBuffer& bufferDbg, long bindFlags);
        void ValidateBindTextureFlags(DbgTexture& textureDbg, long bindFlags);
        void ValidateTextureRegion(DbgTexture& textureDbg, const TextureRegion& region);
        void ValidateIndexType(const Format format);
        void ValidateTextureBufferCopyStrides(DbgTexture& textureDbg, std::uint32_t rowStride, std::uint32_t layerStride, const Extent3D& extent);
        void ValidateMemoryBarrierResourceFlags(ResourceType resourceType, long bindFlags, const std::string& label, std::uint32_t resourceIndex);

        void ValidateStageFlags(long stageFlags, long validFlags);
        void ValidateBufferRange(DbgBuffer& bufferDbg, std::uint64_t offset, std::uint64_t size, const char* rangeName = nullptr);
        void ValidateAddressAlignment(std::uint64_t address, std::uint64_t alignment, const char* addressName);

        bool ValidateQueryIndex(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        DbgQueryHeap::State* GetAndValidateQueryState(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        void ValidateQueryContext(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        void ValidateRenderCondition(DbgQueryHeap& queryHeapDbg, std::uint32_t query);

        void ValidateRenderTargetRange(DbgRenderTarget& renderTargetDbg, const Offset2D& offset, const Extent2D& extent);

        void ValidateSwapBufferIndex(DbgSwapChain& swapChainDbg, std::uint32_t swapBufferIndex);

        void ValidateStreamOutputs(std::uint32_t numBuffers);

        const BindingDescriptor* GetAndValidateResourceDescFromPipeline(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t descriptor, Resource& resource);

        void ValidateUniforms(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t first, std::uint16_t dataSize);

        void ValidateDynamicStates();
        void ValidateBindingTable();
        void ValidateBlendStates();

        DbgPipelineState* AssertAndGetGraphicsPSO();
        DbgPipelineState* AssertAndGetComputePSO();

        void AssertRecording();
        void AssertInsideRenderPass();
        void AssertGraphicsPipelineBound();
        void AssertComputePipelineBound();
        void AssertVertexBufferBound();
        void AssertIndexBufferBound();
        void AssertViewportBound();
        void AssertPrimaryCommandBuffer();

        void AssertInstancingSupported();
        void AssertOffsetInstancingSupported();
        void AssertIndirectDrawingSupported();
        void AssertStreamOutputSupported();

        void AssertNullPointer(const void* ptr, const char* name);

        void WarnImproperVertices(const char* topologyName, std::uint32_t unusedVertices);

        void ResetStates();
        void ResetRecords();
        void ResetBindingTable(const DbgPipelineLayout* pipelineLayoutDbg);

        void StartTimer(const char* annotation);
        void EndTimer();

        // Returns true if this command buffer inherits its state from a primary command buffer.
        bool IsSecondaryCmdBuffer() const;

        void SetAndValidateScissorRects(std::uint32_t numScissors, const Scissor* scissors);

    private:

        /* ----- Common objects ----- */

        RenderingDebugger*          debugger_               = nullptr;
        FrameProfile&               commonProfile_;

        const RenderingFeatures&    features_;
        const RenderingLimits&      limits_;

        std::stack<std::string>     debugGroups_;

        DbgQueryTimerPool           queryTimerPool_;
        bool                        perfProfilerEnabled_    = false;

        /* ----- Render states ----- */

        FrameProfile                profile_;
        PrimitiveTopology           topology_               = PrimitiveTopology::TriangleList;
        Bindings                    bindings_;
        States                      states_;
        Records                     records_;

};


} // /namespace LLGL


#endif



// ================================================================================
