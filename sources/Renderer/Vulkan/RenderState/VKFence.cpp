/*
 * VKFence.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKFence.h"
#include "../VKCore.h"


namespace LLGL
{


VKFence::VKFence(VkDevice device) :
    fence_ { device, vkDestroyFence }
{
    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
    }
    auto result = vkCreateFence(device, &createInfo, nullptr, fence_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan fence");
}

void VKFence::Reset(VkDevice device)
{
    vkResetFences(device, 1, fence_.GetAddressOf());
}

bool VKFence::Wait(VkDevice device, std::uint64_t timeout)
{
    return (vkWaitForFences(device, 1, fence_.GetAddressOf(), VK_TRUE, timeout) == VK_SUCCESS);
}


} // /namespace LLGL



// ================================================================================
