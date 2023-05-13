/*
 * DbgCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_COMMAND_BUFFER_H
#define LLGL_DBG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/RenderingProfiler.h>
#include <LLGL/StaticLimits.h>
#include <LLGL/Container/ArrayView.h>
#include "RenderState/DbgQueryHeap.h"
#include "DbgQueryTimerManager.h"
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
class RenderingDebugger;
class RenderingProfiler;

class DbgCommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        DbgCommandBuffer(
            RenderSystem&                   renderSystemInstance,
            CommandQueue&                   commandQueueInstance,
            CommandBuffer&                  commandBufferInstance,
            RenderingDebugger*              debugger,
            RenderingProfiler*              profiler,
            const CommandBufferDescriptor&  desc,
            const RenderingCapabilities&    caps
        );

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

        void Execute(CommandBuffer& deferredCommandBuffer) override;

        /* ----- Blitting ----- */

        void UpdateBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            const void*     data,
            std::uint16_t   dataSize
        ) override;

        void CopyBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            Buffer&         srcBuffer,
            std::uint64_t   srcOffset,
            std::uint64_t   size
        ) override;

        void CopyBufferFromTexture(
            Buffer&                 dstBuffer,
            std::uint64_t           dstOffset,
            Texture&                srcTexture,
            const TextureRegion&    srcRegion,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void FillBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            std::uint32_t   value,
            std::uint64_t   fillSize    = Constants::wholeSize
        ) override;

        void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
        ) override;

        void CopyTextureFromBuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            Buffer&                 srcBuffer,
            std::uint64_t           srcOffset,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, const TextureSubresource& subresource) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) override;

        /* ----- Resources ----- */

        void SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet = 0) override;
        void SetResource(std::uint32_t descriptor, Resource& resource) override;

        void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) override;

        /* ----- Render Passes ----- */

        void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr,
            std::uint32_t       swapBufferIndex = Constants::currentSwapIndex
        ) override;

        void EndRenderPass() override;

        void Clear(long flags, const ClearValue& clearValue) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Pipeline States ----- */

        void SetPipelineState(PipelineState& pipelineState) override;
        void SetBlendFactor(const float color[4]) override;
        void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) override;
        void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize) override;

        /* ----- Queries ----- */

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

        /* ----- Stream Output ------ */

        void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers) override;
        void EndStreamOutput() override;

        /* ----- Drawing ----- */

        void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;

        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) override;

        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) override;

        void DrawIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) override;
        void DispatchIndirect(Buffer& buffer, std::uint64_t offset) override;

        /* ----- Debugging ----- */

        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;

        /* ----- Extensions ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

    public:

        /* ----- Internal ----- */

        void NextProfile(FrameProfile& outputProfile);

        void ValidateSubmit();

    public:

        /* ----- Debugging members ----- */

        CommandBuffer&                  instance;
        const CommandBufferDescriptor   desc;

    private:

        void EnableRecording(bool enable);

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

        void ValidateStageFlags(long stageFlags, long validFlags);
        void ValidateBufferRange(DbgBuffer& bufferDbg, std::uint64_t offset, std::uint64_t size, const char* rangeName = nullptr);
        void ValidateAddressAlignment(std::uint64_t address, std::uint64_t alignment, const char* addressName);

        bool ValidateQueryIndex(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        DbgQueryHeap::State* GetAndValidateQueryState(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        void ValidateRenderCondition(DbgQueryHeap& queryHeapDbg, std::uint32_t query);

        void ValidateSwapBufferIndex(DbgSwapChain& swapChainDbg, std::uint32_t swapBufferIndex);

        void ValidateStreamOutputs(std::uint32_t numBuffers);

        const BindingDescriptor* GetAndValidateResourceDescFromPipeline(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t descriptor, Resource& resource);

        void ValidateUniforms(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t first, std::uint16_t dataSize);

        void ValidateDynamicStates();
        void ValidateBindingTable();

        DbgPipelineState* AssertAndGetGraphicsPSO();
        DbgPipelineState* AssertAndGetComputePSO();

        void AssertRecording();
        void AssertInsideRenderPass();
        void AssertGraphicsPipelineBound();
        void AssertComputePipelineBound();
        void AssertVertexBufferBound();
        void AssertIndexBufferBound();
        void AssertViewportBound();

        void AssertInstancingSupported();
        void AssertOffsetInstancingSupported();
        void AssertIndirectDrawingSupported();

        void AssertNullPointer(const void* ptr, const char* name);

        void WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices);

        void ResetStates();
        void ResetRecords();
        void ResetBindingTable(const DbgPipelineLayout* pipelineLayoutDbg);

        void StartTimer(const char* annotation);
        void EndTimer();

    private:

        struct SwapChainFramePair
        {
            DbgSwapChain*   swapChain;
            std::uint64_t   frame;      // Frame index when the swap-chain render-pass was encoded
        };

    private:

        /* ----- Common objects ----- */

        RenderingDebugger*          debugger_                               = nullptr;
        RenderingProfiler*          profiler_                               = nullptr;

        const RenderingFeatures&    features_;
        const RenderingLimits&      limits_;

        std::stack<std::string>     debugGroups_;

        DbgQueryTimerManager        timerMngr_;
        bool                        perfProfilerEnabled_                    = false;

        /* ----- Render states ----- */

        FrameProfile                profile_;

        PrimitiveTopology           topology_                               = PrimitiveTopology::TriangleList;

        struct Bindings
        {
            // Framebuffers
            DbgSwapChain*           swapChain                               = nullptr;
            DbgRenderTarget*        renderTarget                            = nullptr;
            std::uint32_t           numViewports                            = 0;

            // Stream inputs/outputs
            DbgBuffer*              vertexBufferStore[1]                    = {};
            DbgBuffer* const *      vertexBuffers                           = nullptr;
            std::uint32_t           numVertexBuffers                        = 0;
            bool                    anyShaderAttributes                     = false;
            DbgBuffer*              indexBuffer                             = nullptr;
            std::uint64_t           indexBufferFormatSize                   = 0;
            std::uint64_t           indexBufferOffset                       = 0;
            DbgBuffer*              streamOutputs[LLGL_MAX_NUM_SO_BUFFERS]  = {};
            std::uint32_t           numStreamOutputs                        = 0;

            // PSO
            DbgPipelineState*       pipelineState                           = nullptr;
            const DbgShader*        vertexShader                            = nullptr;
            bool                    blendFactorSet                          = false;
            bool                    stencilRefSet                           = false;

            struct BindingTable
            {
                ResourceHeap*           resourceHeap;
                std::vector<Resource*>  resources;
                std::vector<char>       uniforms;
            }
            bindingTable;
        }
        bindings_;

        struct States
        {
            bool                    recording                               = false;
            bool                    insideRenderPass                        = false;
            bool                    streamOutputBusy                        = false;
        }
        states_;

        struct Records
        {
            std::vector<SwapChainFramePair> swapChainFrames;
        }
        records_;

};


} // /namespace LLGL


#endif



// ================================================================================
