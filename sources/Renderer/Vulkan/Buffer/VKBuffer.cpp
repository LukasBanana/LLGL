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
 * VKBufferWithRequirements structure
 */

VKBufferWithRequirements::VKBufferWithRequirements(const VKPtr<VkDevice>& device) :
    buffer { device, vkDestroyBuffer }
{
}

VKBufferWithRequirements::VKBufferWithRequirements(VKBufferWithRequirements&& rhs) :
    buffer       { std::move(rhs.buffer) },
    requirements { rhs.requirements      }
{
}

VKBufferWithRequirements& VKBufferWithRequirements::operator = (VKBufferWithRequirements&& rhs)
{
    buffer = std::move(rhs.buffer);
    requirements = rhs.requirements;
    return *this;
}

void VKBufferWithRequirements::Create(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo)
{
    /* Create buffer object */
    auto result = vkCreateBuffer(device, &createInfo, nullptr, buffer.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");

    /* Query memory requirements */
    vkGetBufferMemoryRequirements(device, buffer, &requirements);
}

void VKBufferWithRequirements::Release()
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

void VKBuffer::BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    if (memoryRegion)
    {
        memoryRegion_ = memoryRegion;
        memoryRegion_->BindBuffer(device, GetVkBuffer());
    }
}

void VKBuffer::TakeStagingBuffer(VKBufferWithRequirements&& buffer, VKDeviceMemoryRegion* memoryRegionStaging)
{
    bufferObjStaging_ = std::move(buffer);
    memoryRegionStaging_ = memoryRegionStaging;
}


} // /namespace LLGL



// ================================================================================
