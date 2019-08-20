/*
 * VKCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COMMAND_BUFFER_H
#define LLGL_VK_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"

#include <vector>


namespace LLGL
{


class VKPhysicalDevice;
class VKResourceHeap;

class VKCommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        VKCommandBuffer(
            const VKPhysicalDevice&         physicalDevice,
            const VKPtr<VkDevice>&          device,
            VkQueue                         graphicsQueue,
            const QueueFamilyIndices&       queueFamilyIndices,
            const CommandBufferDescriptor&  desc
        );
        ~VKCommandBuffer();

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

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

        void Execute(CommandBuffer& deferredCommandBuffer) override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

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

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Resource Heaps ----- */

        void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;
        void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;

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

    public:

        // Acquires the next native VkCommandBuffer object.
        void AcquireNextBuffer();

        // Returns the native VkCommandBuffer object.
        inline VkCommandBuffer GetVkCommandBuffer() const
        {
            return commandBuffer_;
        }

        // Returns the fence used to submit the command buffer to the queue.
        inline VkFence GetQueueSubmitFence() const
        {
            return recordingFence_;
        }

    private:

        enum class RecordState
        {
            Undefined,          // before "Begin"
            OutsideRenderPass,  // after "Begin"
            InsideRenderPass,   // after "BeginRenderPass"
            ReadyForSubmit,     // after "End"
        };

        void CreateCommandPool(std::uint32_t queueFamilyIndex);
        void CreateCommandBuffers(std::size_t bufferCount);
        void CreateRecordingFences(VkQueue graphicsQueue, std::size_t numFences);

        void ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments);

        void PauseRenderPass();
        void ResumeRenderPass();

        bool IsInsideRenderPass() const;

        //TODO: current unused; previously used for 'Clear' function
        #if 0
        void BeginClearImage(
            VkImageMemoryBarrier&           clearToPresentBarrier,
            VkImage                         image,
            const VkImageAspectFlags        clearFlags,
            const VkClearColorValue*        clearColor,
            const VkClearDepthStencilValue* clearDepthStencil
        );
        void EndClearImage(VkImageMemoryBarrier& clearToPresentBarrier);
        #endif

        void BindResourceHeap(VKResourceHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t firstSet);

        #if 1//TODO: optimize
        void ResetQueryPoolsInFlight();
        void AppendQueryPoolInFlight(VkQueryPool queryPool);
        #endif

    private:

        const VKPtr<VkDevice>&          device_;
        VKPtr<VkCommandPool>            commandPool_;

        std::vector<VkCommandBuffer>    commandBufferList_;
        VkCommandBuffer                 commandBuffer_;
        std::size_t                     commandBufferIndex_         = 0;

        std::vector<VKPtr<VkFence>>     recordingFenceList_;
        VkFence                         recordingFence_;

        RecordState                     recordState_                = RecordState::Undefined;

        VkCommandBufferUsageFlags       usageFlags_                 = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCommandBufferLevel            bufferLevel_                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkClearColorValue               clearColor_                 = { 0.0f, 0.0f, 0.0f, 0.0f };
        VkClearDepthStencilValue        clearDepthStencil_          = { 1.0f, 0 };

        VkRenderPass                    renderPass_                 = VK_NULL_HANDLE; // primary render pass
        VkRenderPass                    secondaryRenderPass_        = VK_NULL_HANDLE; // to pause/resume render pass (load and store content)
        VkFramebuffer                   framebuffer_                = VK_NULL_HANDLE; // active framebuffer handle
        VkExtent2D                      framebufferExtent_          = { 0, 0 };
        std::uint32_t                   numColorAttachments_        = 0;
        bool                            hasDSVAttachment_           = false;

        std::uint32_t                   queuePresentFamily_         = 0;

        bool                            scissorEnabled_             = false;
        bool                            scissorRectInvalidated_     = true;

        std::uint32_t                   maxDrawIndirectCount_       = 0;

        #if 1//TODO: optimize usage of query pools
        std::vector<VkQueryPool>        queryPoolsInFlight_;
        std::size_t                     numQueryPoolsInFlight_      = 0;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
