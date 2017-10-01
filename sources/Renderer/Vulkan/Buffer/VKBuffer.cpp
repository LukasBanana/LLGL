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


/*
 * VKBufferObject structure
 */

VKBufferObject::VKBufferObject(const VKPtr<VkDevice>& device) :
    buffer { device, vkDestroyBuffer }
{
}

VKBufferObject::VKBufferObject(VKBufferObject&& rhs) :
    buffer       { std::move(rhs.buffer) },
    requirements { rhs.requirements      }
{
}

VKBufferObject& VKBufferObject::operator = (VKBufferObject&& rhs)
{
    buffer = std::move(rhs.buffer);
    requirements = rhs.requirements;
    return *this;
}

void VKBufferObject::Create(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo)
{
    /* Create buffer object */
    auto result = vkCreateBuffer(device, &createInfo, nullptr, buffer.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");

    /* Query memory requirements */
    vkGetBufferMemoryRequirements(device, buffer, &requirements);
}

void VKBufferObject::Release()
{
    buffer.Release();
}


/*
 * VKBuffer class
 */

VKBuffer::VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo) :
    Buffer            { type            },
    bufferObj_        { device          },
    bufferObjStaging_ { device          },
    size_             { createInfo.size }
{
    bufferObj_.Create(device, createInfo);
}

void VKBuffer::BindToMemory(VkDevice device, const std::shared_ptr<VKDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset)
{
    /* Bind buffer to device memory */
    deviceMemory_ = deviceMemory;
    if (deviceMemory_ != nullptr)
        vkBindBufferMemory(device, Get(), deviceMemory_->Get(), memoryOffset);
}

void VKBuffer::TakeStagingBuffer(VKBufferObject&& buffer, std::shared_ptr<VKDeviceMemory>&& deviceMemory)
{
    bufferObjStaging_ = std::move(buffer);

}


} // /namespace LLGL



// ================================================================================
