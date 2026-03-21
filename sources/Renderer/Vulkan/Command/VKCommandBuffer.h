/*
 * VKCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_BUFFER_H
#define LLGL_VK_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "VKCommandBufferRing.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../VKCore.h"
#include "VKCommandContext.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../Buffer/VKStagingBufferPool.h"
#include "../RenderState/VKStagingDescriptorSetPool.h"
#include "../RenderState/VKDescriptorCache.h"
#include "../RenderState/VKPipelineLayout.h"
#include <vector>


namespace LLGL
{


class VKPhysicalDevice;
class VKResourceHeap;
class VKRenderPass;
class VKQueryHeap;
class VKSwapChain;
class VKPipelineState;
class VKPipelineBarrier;

class VKCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        VKCommandBuffer(
            const VKPhysicalDevice&         physicalDevice,
            VkDevice                        device,
            VkQueue                         commandQueue,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const VKQueueFamilyIndices&     queueFamilyIndices,
            const CommandBufferDescriptor&  desc
        );

    public:

        // Submits this command buffer to the specified queue. This might include another command buffer that is submitted alongside to reset query pools.
        VkResult SubmitToQueue(VkQueue queue);

        // Returns the native VkCommandBuffer object.
        inline VkCommandBuffer GetVkCommandBuffer() const
        {
            return commandBuffer_;
        }

        // Returns true if this is an immediate command buffer, otherwise it is a deferred command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return immediateSubmit_;
        }

        // Returns true if this is a secondary command buffer (VK_COMMAND_BUFFER_LEVEL_SECONDARY).
        inline bool IsSecondaryCmdBuffer() const
        {
            return (bufferLevel_ == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
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

        void CreateStagingBufferPools(VKDeviceMemoryManager& deviceMemoryMngr, VkDeviceSize minStagingPoolSize);

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
        void SubmitAutoPipelineBarrier();

        // Acquires the next native VkCommandBuffer object.
        void AcquireNextBuffer();

        void ResetBindingStates();

        void BindVertexBuffer(VKBuffer& bufferVK);

        inline VKStagingBufferPool& GetStagingBufferPool()
        {
            return stagingBufferPools_[commandBufferRing_.GetIndex()];
        }

    private:

        struct InputAssemblyState
        {
            // Input-assembly state for slot 0 only (IA0)
            VkBuffer        ia0XfbCounterBuffer         = VK_NULL_HANDLE;
            VkDeviceSize    ia0XfbCounterBufferOffset   = 0;
            std::uint32_t   ia0VertexStride             = 0;
        };

        struct TransformFeedbackState
        {
            VkBuffer        xfbBuffers[LLGL_MAX_NUM_SO_BUFFERS]         = {};
            VkDeviceSize    xfbCounterOffsets[LLGL_MAX_NUM_SO_BUFFERS]  = {};
            std::uint32_t   numXfbBuffers                               = 0;
        };

    private:

        VkDevice                        device_                                         = VK_NULL_HANDLE;

        VkQueue                         commandQueue_                                   = VK_NULL_HANDLE;

        VKCommandBufferRing             commandBufferRing_;
        VkCommandBuffer                 commandBuffer_                                  = VK_NULL_HANDLE;

        VKCommandContext                context_;

        VKStagingBufferPool             stagingBufferPools_[VKCommandBufferRing::maxCount];

        RecordState                     recordState_                                    = RecordState::Undefined;

        VkCommandBufferLevel            bufferLevel_                                    = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        VkCommandBufferUsageFlags       usageFlags_                                     = 0;
        bool                            immediateSubmit_                                = false;

        VKSwapChain*                    boundSwapChain_                                 = nullptr;
        std::uint32_t                   currentColorBuffer_                             = 0;

        VkRenderPass                    renderPass_                                     = VK_NULL_HANDLE; // primary render pass
        VkRenderPass                    secondaryRenderPass_                            = VK_NULL_HANDLE; // to pause/resume render pass (load and store content)
        VkFramebuffer                   framebuffer_                                    = VK_NULL_HANDLE; // active framebuffer handle
        VkRect2D                        framebufferRenderArea_                          = { { 0, 0 }, { 0, 0 } };
        std::uint32_t                   numColorAttachments_                            = 0;
        bool                            hasDepthStencilAttachment_                      = false;
        VkSubpassContents               subpassContents_                                = VK_SUBPASS_CONTENTS_INLINE;

        std::uint32_t                   queuePresentFamily_                             = 0;

        bool                            scissorEnabled_                                 = false;
        bool                            hasDynamicScissorRect_                          = false;
        VkPipelineBindPoint             pipelineBindPoint_                              = VK_PIPELINE_BIND_POINT_MAX_ENUM;
        const VKLayoutBindingTable*     boundBindingTable_                              = nullptr;
        VKPipelineState*                boundPipelineState_                             = nullptr;
        VKPipelineBarrier*              boundPipelineBarrier_                           = nullptr;

        std::uint32_t                   maxDrawIndirectCount_                           = 0;

        VKStagingDescriptorSetPool      descriptorSetPoolArray_[VKCommandBufferRing::maxCount];
        VKStagingDescriptorSetPool*     descriptorSetPool_                              = nullptr;
        VKDescriptorCache*              descriptorCache_                                = nullptr;
        VKDescriptorSetWriter           descriptorSetWriter_;

        InputAssemblyState              iaState_;
        TransformFeedbackState          xfbState_;

        #if 0//TODO: optimize usage of query pools
        std::vector<VKQueryHeap*>       queryHeapsInFlight_;
        std::size_t                     numQueryHeapsInFlight_                          = 0;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
