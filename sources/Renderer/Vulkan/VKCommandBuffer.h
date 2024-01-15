/*
 * VKCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_BUFFER_H
#define LLGL_VK_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"
#include "RenderState/VKStagingDescriptorSetPool.h"
#include "RenderState/VKDescriptorCache.h"
#include <vector>


namespace LLGL
{


class VKDevice;
class VKPhysicalDevice;
class VKResourceHeap;
class VKRenderPass;
class VKQueryHeap;
class VKSwapChain;
class VKPipelineState;

class VKCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        VKCommandBuffer(
            const VKPhysicalDevice&         physicalDevice,
            VKDevice&                       device,
            VkQueue                         commandQueue,
            const QueueFamilyIndices&       queueFamilyIndices,
            const CommandBufferDescriptor&  desc
        );

        ~VKCommandBuffer();

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

        // Returns true if this is an immediate command buffer, otherwise it is a deferred command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return immediateSubmit_;
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

        void CreateVkCommandPool(std::uint32_t queueFamilyIndex);
        void CreateVkCommandBuffers();
        void CreateVkRecordingFences();

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

        void BufferPipelineBarrier(
            VkBuffer                buffer,
            VkDeviceSize            offset,
            VkDeviceSize            size,
            VkAccessFlags           srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT,
            VkAccessFlags           dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
            VkPipelineStageFlags    srcStageMask    = VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkPipelineStageFlags    dstStageMask    = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
        );

        void FlushDescriptorCache();

        // Acquires the next native VkCommandBuffer object.
        void AcquireNextBuffer();

        void ResetBindingStates();

        #if 1//TODO: optimize
        void ResetQueryPoolsInFlight();
        void AppendQueryPoolInFlight(VKQueryHeap* queryHeap);
        #endif

    private:

        // Returns the number of native Vulkan command buffers used for the specified descriptor.
        static std::uint32_t GetNumVkCommandBuffers(const CommandBufferDescriptor& desc);

    private:

        static constexpr std::uint32_t maxNumCommandBuffers = 3;

        VKDevice&                       device_;

        VkQueue                         commandQueue_               = VK_NULL_HANDLE;

        VKPtr<VkCommandPool>            commandPool_;

        VKPtr<VkFence>                  recordingFenceArray_[maxNumCommandBuffers];
        VkFence                         recordingFence_             = VK_NULL_HANDLE;
        VkCommandBuffer                 commandBufferArray_[maxNumCommandBuffers];
        VkCommandBuffer                 commandBuffer_              = VK_NULL_HANDLE;
        std::uint32_t                   commandBufferIndex_         = 0;
        std::uint32_t                   numCommandBuffers_          = 2;

        RecordState                     recordState_                = RecordState::Undefined;

        VkCommandBufferLevel            bufferLevel_                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        VkCommandBufferUsageFlags       usageFlags_                 = 0;
        bool                            immediateSubmit_            = false;

        VKSwapChain*                    boundSwapChain_             = nullptr;
        std::uint32_t                   currentColorBuffer_         = 0;

        VkRenderPass                    renderPass_                 = VK_NULL_HANDLE; // primary render pass
        VkRenderPass                    secondaryRenderPass_        = VK_NULL_HANDLE; // to pause/resume render pass (load and store content)
        VkFramebuffer                   framebuffer_                = VK_NULL_HANDLE; // active framebuffer handle
        VkRect2D                        framebufferRenderArea_      = { { 0, 0 }, { 0, 0 } };
        std::uint32_t                   numColorAttachments_        = 0;
        bool                            hasDepthStencilAttachment_  = false;

        std::uint32_t                   queuePresentFamily_         = 0;

        bool                            scissorEnabled_             = false;
        bool                            scissorRectInvalidated_     = true;
        VkPipelineBindPoint             pipelineBindPoint_          = VK_PIPELINE_BIND_POINT_MAX_ENUM;
        const VKPipelineLayout*         boundPipelineLayout_        = nullptr;
        VKPipelineState*                boundPipelineState_         = nullptr;

        std::uint32_t                   maxDrawIndirectCount_       = 0;

        VKStagingDescriptorSetPool      descriptorSetPoolArray_[maxNumCommandBuffers];
        VKStagingDescriptorSetPool*     descriptorSetPool_          = nullptr;
        VKDescriptorCache*              descriptorCache_            = nullptr;
        VKDescriptorSetWriter           descriptorSetWriter_;

        #if 1//TODO: optimize usage of query pools
        std::vector<VKQueryHeap*>       queryHeapsInFlight_;
        std::size_t                     numQueryHeapsInFlight_      = 0;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
