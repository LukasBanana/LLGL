/*
 * VKCommandBufferRing.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_BUFFER_RING_H
#define LLGL_VK_COMMAND_BUFFER_RING_H


#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../VKCore.h"
#include <vector>


namespace LLGL
{


class VKCommandBufferRing
{

    public:

        static constexpr std::uint32_t maxCount = 3;

        enum CommandType
        {
            CommandType_Default,
            CommandType_ResetQuery,

            CommandType_Num,
        };

    public:

        VKCommandBufferRing(VkDevice device);
        ~VKCommandBufferRing();

        void Create(VkCommandBufferLevel cmdBufferLevel, std::uint32_t graphicsFamily, std::uint32_t numNativeBuffers);

    public:

        // Returns the fence used to submit the command buffer and resets it if this is a multi-submit command buffer,
        // i.e. it won't need another signal for the next submission.
        VkFence GetQueueSubmitFenceAndFlush();

        // Returns the native command buffer from this ring.
        inline VkCommandBuffer GetVkCommandBuffer(CommandType type) const
        {
            return commandBuffers_[index_ + maxCount*type];
        }

        // Returns the current iteration index.
        inline std::uint32_t GetIndex() const
        {
            return index_;
        }

        // Returns the number of command buffers this ring buffer can iterate through.
        inline std::uint32_t GetCount() const
        {
            return count_;
        }

        // Acquires the next native VkCommandBuffer object.
        VkCommandBuffer AcquireNextBuffer();

    private:

        void CreateVkCommandPool(std::uint32_t queueFamilyIndex);
        void CreateVkCommandBuffers(VkCommandBufferLevel cmdBufferLevel);
        void CreateVkRecordingFences();

    private:

        // Returns the number of native Vulkan command buffers used for the specified descriptor.
        static std::uint32_t GetIterationCount(std::uint32_t numNativeBuffers);

    private:

        static constexpr std::uint32_t maxNumCommandBuffers = maxCount * CommandType_Num;

        VkDevice                device_                         = VK_NULL_HANDLE;

        VKPtr<VkCommandPool>    commandPool_;

        std::uint32_t           index_                          = 0;
        std::uint32_t           count_                          = 2;

        VKPtr<VkFence>          recordingFenceArray_[maxCount];
        VkFence                 recordingFence_                 = VK_NULL_HANDLE;
        bool                    recordingFenceDirty_[maxCount]  = {};

        VkCommandBuffer         commandBuffers_[maxNumCommandBuffers];

};


} // /namespace LLGL


#endif



// ================================================================================
