/*
 * VKBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKBuffer.h"
#include "../VKCore.h"


namespace LLGL
{


VKBuffer::VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo) :
    Buffer  { type                    },
    buffer_ { device, vkDestroyBuffer },
    size_   { createInfo.size         }
{
    /* Create buffer object */
    auto result = vkCreateBuffer(device, &createInfo, nullptr, buffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");

    /* Query memory requirements */
    vkGetBufferMemoryRequirements(device, Get(), &requirements_);
}

void VKBuffer::BindToMemory(VkDevice device, const std::shared_ptr<VKDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset)
{
    /* Bind buffer to device memory */
    deviceMemory_ = deviceMemory;
    if (deviceMemory_ != nullptr)
        vkBindBufferMemory(device, buffer_, deviceMemory_->Get(), memoryOffset);
}

void* VKBuffer::Map(VkDevice device)
{
    void* memory = nullptr;

    auto result = vkMapMemory(device, deviceMemory_->Get(), 0, size_, 0, &memory);
    VKThrowIfFailed(result, "failed to map Vulkan buffer into CPU memory space");

    return memory;
}

void VKBuffer::Unmap(VkDevice device)
{
    vkUnmapMemory(device, deviceMemory_->Get());
}


} // /namespace LLGL



// ================================================================================
