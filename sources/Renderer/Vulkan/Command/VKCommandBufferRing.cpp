/*
 * VKCommandBufferRing.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCommandBufferRing.h"
#include "../VKPhysicalDevice.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


constexpr std::uint32_t VKCommandBufferRing::maxCount;
constexpr std::uint32_t VKCommandBufferRing::maxNumCommandBuffers;

VKCommandBufferRing::VKCommandBufferRing(VkDevice device) :
    device_              { device                                   },
    commandPool_         { device, vkDestroyCommandPool             },
    recordingFenceArray_ { VKPtr<VkFence>{ device, vkDestroyFence },
                           VKPtr<VkFence>{ device, vkDestroyFence },
                           VKPtr<VkFence>{ device, vkDestroyFence } }
{
}

VKCommandBufferRing::~VKCommandBufferRing()
{
    vkFreeCommandBuffers(device_, commandPool_, count_, commandBuffers_);
}

void VKCommandBufferRing::Create(
    VkCommandBufferLevel    cmdBufferLevel,
    std::uint32_t           graphicsFamily,
    std::uint32_t           numNativeBuffers)
{
    count_ = VKCommandBufferRing::GetIterationCount(numNativeBuffers);

    /* Create native command buffer objects */
    CreateVkCommandPool(graphicsFamily);
    CreateVkCommandBuffers(cmdBufferLevel);
    CreateVkRecordingFences();
}

VkFence VKCommandBufferRing::GetQueueSubmitFenceAndFlush()
{
    /*
    Flush recoring fence since we don't have to signal it more than once,
    until the same native command buffer is recorded again.
    */
    VkFence fence = recordingFence_;
    recordingFence_ = VK_NULL_HANDLE;
    recordingFenceDirty_[index_] = true;
    return fence;
}

VkCommandBuffer VKCommandBufferRing::AcquireNextBuffer()
{
    /* Move to next command buffer index */
    index_ = (index_ + 1) % count_;

    /* Wait for fence before using next command buffer */
    recordingFence_ = recordingFenceArray_[index_].Get();
    if (recordingFenceDirty_[index_])
        vkWaitForFences(device_, 1, &recordingFence_, VK_TRUE, UINT64_MAX);

    /* Reset fence state after it has been signaled by the command queue */
    vkResetFences(device_, 1, &recordingFence_);
    recordingFenceDirty_[index_] = false;

    /* Return new active command buffer */
    return commandBuffers_[index_];
}


/*
 * ======= Private: =======
 */

void VKCommandBufferRing::CreateVkCommandPool(std::uint32_t queueFamilyIndex)
{
    /* Create command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
    }
    VkResult result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");
}

void VKCommandBufferRing::CreateVkCommandBuffers(VkCommandBufferLevel cmdBufferLevel)
{
    /* Allocate command buffers */
    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.commandPool           = commandPool_;
        allocInfo.level                 = cmdBufferLevel;
        allocInfo.commandBufferCount    = maxNumCommandBuffers;
    }
    VkResult result = vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_);
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffers");
}

void VKCommandBufferRing::CreateVkRecordingFences()
{
    /* Create all recording fences with their initial state being signaled */
    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    for_range(i, count_)
    {
        /* Create fence for command buffer recording */
        VkResult result = vkCreateFence(device_, &createInfo, nullptr, recordingFenceArray_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan fence");
    }
}

std::uint32_t VKCommandBufferRing::GetIterationCount(std::uint32_t numNativeBuffers)
{
    constexpr std::uint32_t defaultIterationCount = 2;
    if (numNativeBuffers == 0)
        return defaultIterationCount;
    else
        return Clamp<std::uint32_t>(numNativeBuffers, 1u, VKCommandBufferRing::maxCount);
}


} // /namespace LLGL



// ================================================================================
