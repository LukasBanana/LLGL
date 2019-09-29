/*
 * DbgCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMMAND_BUFFER_H
#define LLGL_DBG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/RenderingProfiler.h>
#include "DbgGraphicsPipeline.h"
#include "DbgQueryHeap.h"
#include "DbgQueryTimerManager.h"
#include <cstdint>
#include <string>
#include <stack>


namespace LLGL
{


class DbgBuffer;
class DbgTexture;
class DbgRenderContext;
class DbgRenderTarget;
class DbgComputePipeline;
class DbgShaderProgram;
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

        void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
        ) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, const TextureSubresource& subresource) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Clear ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) override;

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput() override;
        void EndStreamOutput() override;

        /* ----- Resources ----- */

        void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;
        void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;

        void SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long stageFlags = StageFlags::AllStages) override;

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
            const ClearValue*   clearValues     = nullptr
        ) override;

        void EndRenderPass() override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        void SetUniform(
            UniformLocation location,
            const void*     data,
            std::uint32_t   dataSize
        ) override;

        void SetUniforms(
            UniformLocation location,
            std::uint32_t   count,
            const void*     data,
            std::uint32_t   dataSize
        ) override;

        /* ----- Queries ----- */

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

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
        void ValidateVertexLayoutAttributes(const std::vector<VertexAttribute>& shaderVertexAttribs, DbgBuffer* const * vertexBuffers, std::uint32_t numVertexBuffers);

        void ValidateNumVertices(std::uint32_t numVertices);
        void ValidateNumInstances(std::uint32_t numInstances);
        void ValidateVertexID(std::uint32_t firstVertex);
        void ValidateInstanceID(std::uint32_t firstInstance);

        void ValidateDrawCmd(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance);
        void ValidateDrawIndexedCmd(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance);

        void ValidateVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit);
        void ValidateThreadGroupLimit(std::uint32_t size, std::uint32_t limit);
        void ValidateAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound);

        void ValidateBindFlags(long resourceFlags, long bindFlags, long validFlags, const char* resourceName = nullptr);
        void ValidateBindBufferFlags(DbgBuffer& bufferDbg, long bindFlags);
        void ValidateIndexType(const Format format);

        void ValidateStageFlags(long stageFlags, long validFlags);
        void ValidateBufferRange(DbgBuffer& bufferDbg, std::uint64_t offset, std::uint64_t size, const char* rangeName = nullptr);
        void ValidateAddressAlignment(std::uint64_t address, std::uint64_t alignment, const char* addressName);

        bool ValidateQueryIndex(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        DbgQueryHeap::State* GetAndValidateQueryState(DbgQueryHeap& queryHeapDbg, std::uint32_t query);
        void ValidateRenderCondition(DbgQueryHeap& queryHeapDbg, std::uint32_t query);

        void AssertRecording();
        void AssertInsideRenderPass();
        void AssertGraphicsPipelineBound();
        void AssertComputePipelineBound();
        void AssertVertexBufferBound();
        void AssertIndexBufferBound();

        void AssertInstancingSupported();
        void AssertOffsetInstancingSupported();
        void AssertIndirectDrawingSupported();

        void AssertNullPointer(const void* ptr, const char* name);

        void WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices);

        void ResetFrameProfile();
        void ResetBindings();
        void ResetStates();

        void StartTimer(const char* annotation);
        void EndTimer();

    private:

        /* ----- Common objects ----- */

        RenderingDebugger*          debugger_                       = nullptr;
        RenderingProfiler*          profiler_                       = nullptr;

        const RenderingFeatures&    features_;
        const RenderingLimits&      limits_;

        std::stack<std::string>     debugGroups_;

        DbgQueryTimerManager        timerMngr_;
        bool                        perfProfilerEnabled_            = false;

        /* ----- Render states ----- */

        FrameProfile                profile_;

        PrimitiveTopology           topology_                       = PrimitiveTopology::TriangleList;

        struct Bindings
        {
            DbgRenderContext*       renderContext           = nullptr;
            DbgRenderTarget*        renderTarget            = nullptr;
            DbgBuffer*              vertexBufferStore[1]    = { nullptr };
            DbgBuffer* const *      vertexBuffers           = nullptr;
            std::uint32_t           numVertexBuffers        = 0;
            bool                    anyNonEmptyVertexBuffer = false;
            bool                    anyShaderAttributes     = false;
            DbgBuffer*              indexBuffer             = nullptr;
            DbgBuffer*              streamOutput            = nullptr;
            DbgGraphicsPipeline*    graphicsPipeline        = nullptr;
            DbgComputePipeline*     computePipeline         = nullptr;
            const DbgShaderProgram* shaderProgram_          = nullptr;
        }
        bindings_;

        struct States
        {
            bool recording          = false;
            bool insideRenderPass   = false;
            bool streamOutputBusy   = false;
        }
        states_;

};


} // /namespace LLGL


#endif



// ================================================================================
