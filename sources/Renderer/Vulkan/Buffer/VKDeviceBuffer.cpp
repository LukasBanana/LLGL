/*
 * VKDeviceBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

VKDeviceBuffer::VKDeviceBuffer(
    const VKPtr<VkDevice>&      device,
    const VkBufferCreateInfo&   createInfo,
    VKDeviceMemoryManager&      deviceMemoryMngr,
    VkMemoryPropertyFlags       memoryProperties)
:
    VKDeviceBuffer { device }
{
    CreateVkBufferAndMemoryRegion(device, createInfo, deviceMemoryMngr, memoryProperties);
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
    /* Create Vulkan buffer object and query memory requirements */
    auto result = vkCreateBuffer(device, &createInfo, nullptr, buffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan buffer");
    vkGetBufferMemoryRequirements(device, buffer_, &requirements_);
}

void VKDeviceBuffer::CreateVkBufferAndMemoryRegion(
    const VKPtr<VkDevice>&      device,
    const VkBufferCreateInfo&   createInfo,
    VKDeviceMemoryManager&      deviceMemoryMngr,
    VkMemoryPropertyFlags       memoryProperties)
{
    /* Create Vulkan bnuffer object */
    CreateVkBuffer(device, createInfo);

    if (auto memoryRegion = deviceMemoryMngr.Allocate(requirements_, memoryProperties))
    {
        /* Bind allocated memory region to buffer */
        BindMemoryRegion(device, memoryRegion);
    }
    else
    {
        /* Failed to allocate device memory */
        throw std::runtime_error(
            "failed to allocate " + std::to_string(requirements_.size) +
            " byte(s) of device memory for Vulkan buffer"
        );
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

void* VKDeviceBuffer::Map(VkDevice device)
{
    if (memoryRegion_)
        return memoryRegion_->GetParentChunk()->Map(device, memoryRegion_->GetOffset(), memoryRegion_->GetSize());
    else
        return nullptr;
}

void VKDeviceBuffer::Unmap(VkDevice device)
{
    if (memoryRegion_)
        memoryRegion_->GetParentChunk()->Unmap(device);
}


} // /namespace LLGL



// ================================================================================
