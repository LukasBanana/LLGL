/*
 * VKCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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


class VKDevice;
class VKPhysicalDevice;
class VKResourceHeap;
class VKRenderPass;
class VKQueryHeap;

class VKCommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        VKCommandBuffer(
            const VKPhysicalDevice&         physicalDevice,
            VKDevice&                       device,
            VkQueue                         graphicsQueue,
            const QueueFamilyIndices&       queueFamilyIndices,
            const CommandBufferDescriptor&  desc
        );
        ~VKCommandBuffer();

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

        /* ----- Resources ----- */

        void SetResourceHeap(
            ResourceHeap&           resourceHeap,
            std::uint32_t           firstSet        = 0,
            const PipelineBindPoint bindPoint       = PipelineBindPoint::Undefined
        ) override;

        void SetResource(
            Resource&       resource,
            std::uint32_t   slot,
            long            bindFlags,
            long            stageFlags = StageFlags::AllStages
        ) override;

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

        void SetPipelineState(PipelineState& pipelineState) override;
        void SetBlendFactor(const ColorRGBAf& color) override;
        void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) override;

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

        /* ----- Internals ----- */

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

    private:

        void CreateCommandPool(std::uint32_t queueFamilyIndex);
        void CreateCommandBuffers(std::uint32_t bufferCount);
        void CreateRecordingFences(VkQueue graphicsQueue, std::uint32_t numFences);

        void ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments);

        void ConvertRenderPassClearValues(
            const VKRenderPass& renderPass,
            std::uint32_t&      dstClearValuesCount,
            VkClearValue*       dstClearValues,
            std::uint32_t       srcClearValuesCount,
            const ClearValue*   srcClearValues
        );

        void PauseRenderPass();
        void ResumeRenderPass();

        bool IsInsideRenderPass() const;

        void BindResourceHeap(VKResourceHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t firstSet);

        // Acquires the next native VkCommandBuffer object.
        void AcquireNextBuffer();

        #if 1//TODO: optimize
        void ResetQueryPoolsInFlight();
        void AppendQueryPoolInFlight(VKQueryHeap* queryHeap);
        #endif

    private:

        VKDevice&                       device_;
        VKPtr<VkCommandPool>            commandPool_;

        std::vector<VkCommandBuffer>    commandBufferList_;
        VkCommandBuffer                 commandBuffer_;
        std::size_t                     commandBufferIndex_         = 0;

        std::vector<VKPtr<VkFence>>     recordingFenceList_;
        VkFence                         recordingFence_;

        RecordState                     recordState_                = RecordState::Undefined;

        VkCommandBufferUsageFlags       usageFlags_                 = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCommandBufferLevel            bufferLevel_                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkClearColorValue               clearColor_                 = { { 0.0f, 0.0f, 0.0f, 0.0f } };
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
        std::vector<VKQueryHeap*>       queryHeapsInFlight_;
        std::size_t                     numQueryHeapsInFlight_      = 0;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
