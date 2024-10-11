/*
 * VKDeviceBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDeviceBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../VKCore.h"
#include "../../../Core/PrintfUtils.h"
#include "../../../Core/Assertion.h"
#include <algorithm>


namespace LLGL
{


/* ----- Common ----- */

VKDeviceBuffer::VKDeviceBuffer(VkDevice device) :
    buffer_ { device, vkDestroyBuffer }
{
}

VKDeviceBuffer::VKDeviceBuffer(VkDevice device, const VkBufferCreateInfo& createInfo) :
    VKDeviceBuffer { device }
{
    CreateVkBuffer(device, createInfo);
}

VKDeviceBuffer::VKDeviceBuffer(
    VkDevice                    device,
    const VkBufferCreateInfo&   createInfo,
    VKDeviceMemoryManager&      deviceMemoryMngr,
    VkMemoryPropertyFlags       memoryProperties)
:
    VKDeviceBuffer { device }
{
    CreateVkBufferAndMemoryRegion(device, createInfo, deviceMemoryMngr, memoryProperties);
}

VKDeviceBuffer::VKDeviceBuffer(VKDeviceBuffer&& rhs) noexcept :
    buffer_       { std::move(rhs.buffer_) },
    requirements_ { rhs.requirements_      },
    memoryRegion_ { rhs.memoryRegion_      }
{
    rhs.memoryRegion_ = nullptr;
}

VKDeviceBuffer& VKDeviceBuffer::operator = (VKDeviceBuffer&& rhs) noexcept
{
    buffer_             = std::move(rhs.buffer_);
    requirements_       = rhs.requirements_;
    memoryRegion_       = rhs.memoryRegion_;
    rhs.memoryRegion_   = nullptr;
    return *this;
}

/* ----- Native buffer ----- */

void VKDeviceBuffer::CreateVkBuffer(VkDevice device, const VkBufferCreateInfo& createInfo)
{
    /* Create Vulkan buffer object and query memory requirements */
    VkResult result = vkCreateBuffer(device, &createInfo, nullptr, buffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");
    vkGetBufferMemoryRequirements(device, buffer_, &requirements_);
}

void VKDeviceBuffer::CreateVkBufferAndMemoryRegion(
    VkDevice                    device,
    const VkBufferCreateInfo&   createInfo,
    VKDeviceMemoryManager&      deviceMemoryMngr,
    VkMemoryPropertyFlags       memoryProperties)
{
    /* Create Vulkan bnuffer object */
    CreateVkBuffer(device, createInfo);

    if (VKDeviceMemoryRegion* memoryRegion = deviceMemoryMngr.Allocate(requirements_, memoryProperties))
    {
        /* Bind allocated memory region to buffer */
        BindMemoryRegion(device, memoryRegion);
    }
    else
    {
        /* Failed to allocate device memory */
        LLGL_TRAP("failed to allocate %" PRIu64 " byte(s) of device memory for Vulkan buffer", requirements_.size);
    }
}

void VKDeviceBuffer::ReleaseVkBuffer()
{
    buffer_.Release();
}

void VKDeviceBuffer::BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
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

void* VKDeviceBuffer::Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size)
{
    if (memoryRegion_)
    {
        offset  = (std::min)(offset, memoryRegion_->GetSize());
        size    = (std::min)(size, memoryRegion_->GetSize() - offset);
        return memoryRegion_->GetParentChunk()->Map(device, memoryRegion_->GetOffset() + offset, size);
    }
    return nullptr;
}

void VKDeviceBuffer::Unmap(VkDevice device)
{
    if (memoryRegion_)
        memoryRegion_->GetParentChunk()->Unmap(device);
}


} // /namespace LLGL



// ================================================================================
