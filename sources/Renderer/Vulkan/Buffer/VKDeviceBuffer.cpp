/*
 * VKDeviceBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../VKCore.h"


namespace LLGL
{


/* ----- Common ----- */

VKDeviceBuffer::VKDeviceBuffer(const VKPtr<VkDevice>& device) :
    buffer_ { device, vkDestroyBuffer }
{
}

VKDeviceBuffer::VKDeviceBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo) :
    VKDeviceBuffer { device }
{
    CreateVkBuffer(device, createInfo);
}

VKDeviceBuffer::VKDeviceBuffer(VKDeviceBuffer&& rhs) :
    buffer_       { std::move(rhs.buffer_) },
    requirements_ { rhs.requirements_      },
    memoryRegion_ { rhs.memoryRegion_      }
{
    rhs.memoryRegion_ = nullptr;
}

VKDeviceBuffer& VKDeviceBuffer::operator = (VKDeviceBuffer&& rhs)
{
    buffer_             = std::move(rhs.buffer_);
    requirements_       = rhs.requirements_;
    memoryRegion_       = rhs.memoryRegion_;
    rhs.memoryRegion_   = nullptr;
    return *this;
}

/* ----- Native buffer ----- */

void VKDeviceBuffer::CreateVkBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo)
{
    /* Create buffer object */
    auto result = vkCreateBuffer(device, &createInfo, nullptr, buffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");

    /* Query memory requirements */
    vkGetBufferMemoryRequirements(device, buffer_, &requirements_);
}

void VKDeviceBuffer::ReleaseVkBuffer()
{
    buffer_.Release();
}

void VKDeviceBuffer::BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    if (memoryRegion)
    {
        memoryRegion_ = memoryRegion;
        memoryRegion_->BindBuffer(device, GetVkBuffer());
    }
}

void VKDeviceBuffer::ReleaseMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr)
{
    deviceMemoryMngr.Release(memoryRegion_);
    memoryRegion_ = nullptr;
}


} // /namespace LLGL



// ================================================================================
