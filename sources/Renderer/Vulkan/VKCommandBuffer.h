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


class VKResourceHeap;

class VKCommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        VKCommandBuffer(
            const VKPtr<VkDevice>&          device,
            VkQueue                         graphicsQueue,
            const QueueFamilyIndices&       queueFamilyIndices,
            const CommandBufferDescriptor&  desc
        );
        ~VKCommandBuffer();

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

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void SetRenderTarget(RenderContext& renderContext) override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;
        bool QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
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

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) override;

        /* ----- Extended functions ----- */

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

        void CreateCommandPool(std::uint32_t queueFamilyIndex);
        void CreateCommandBuffers(std::size_t bufferCount);
        void CreateRecordingFences(VkQueue graphicsQueue, std::size_t numFences);

        void ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments);

        //TODO: current unused; previously used for 'Clear' function
        #if 0
        void BeginClearImage(
            VkImageMemoryBarrier& clearToPresentBarrier, VkImage image, const VkImageAspectFlags clearFlags,
            const VkClearColorValue* clearColor, const VkClearDepthStencilValue* clearDepthStencil
        );
        void EndClearImage(VkImageMemoryBarrier& clearToPresentBarrier);
        #endif

        void BindResourceHeap(VKResourceHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t firstSet);

        const VKPtr<VkDevice>&          device_;
        VKPtr<VkCommandPool>            commandPool_;

        std::vector<VkCommandBuffer>    commandBufferList_;
        VkCommandBuffer                 commandBuffer_;
        std::size_t                     commandBufferIndex_         = 0;

        std::vector<VKPtr<VkFence>>     recordingFenceList_;
        VkFence                         recordingFence_;

        VkClearColorValue               clearColor_                 = { 0.0f, 0.0f, 0.0f, 0.0f };
        VkClearDepthStencilValue        clearDepthStencil_          = { 1.0f, 0 };

        VkRenderPass                    renderPass_                 = VK_NULL_HANDLE;
        bool                            renderPassActive_           = false;
        VkFramebuffer                   framebuffer_                = VK_NULL_HANDLE;
        VkExtent2D                      framebufferExtent_          = { 0, 0 };
        std::uint32_t                   numColorAttachments_        = 0;
        bool                            hasDSVAttachment_           = false;

        #if 1//TODO: remove (use numColorAttachments_ instead)
        VkImage                         imageColor_                 = VK_NULL_HANDLE;
        VkImage                         imageDepthStencil_          = VK_NULL_HANDLE;
        #endif

        std::uint32_t                   queuePresentFamily_         = 0;

        bool                            scissorEnabled_             = false;
        bool                            scissorRectInvalidated_     = true;

};


} // /namespace LLGL


#endif



// ================================================================================
