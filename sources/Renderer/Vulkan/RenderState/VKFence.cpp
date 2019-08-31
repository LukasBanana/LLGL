/*
 * VKFence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKFence.h"
#include "../VKCore.h"


namespace LLGL
{


VKFence::VKFence(const VKPtr<VkDevice>& device) :
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
    vkResetFences(device, 1, &fence_);
}

bool VKFence::Wait(VkDevice device, std::uint64_t timeout)
{
    return (vkWaitForFences(device, 1, &fence_, VK_TRUE, timeout) == VK_SUCCESS);
}


} // /namespace LLGL



// ================================================================================
