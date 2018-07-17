/*
 * VKBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKBuffer.h"
#include "../VKCore.h"
#include <string.h>


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

void* VKBuffer::Map(VkDevice device, const CPUAccess access)
{
    if (memoryRegionStaging_)
    {
        mappingCPUAccess_ = access;
        return memoryRegionStaging_->GetParentChunk()->Map(
            device, memoryRegionStaging_->GetOffset(), memoryRegionStaging_->GetSize()
        );
    }
    return nullptr;
}

void VKBuffer::Unmap(VkDevice device)
{
    if (memoryRegionStaging_)
        memoryRegionStaging_->GetParentChunk()->Unmap(device);
}

void* VKBuffer::MapStaging(VkDevice device, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (memoryRegionStaging_)
        return memoryRegionStaging_->GetParentChunk()->Map(device, memoryRegionStaging_->GetOffset() + offset, dataSize);
    else
        return nullptr;
}

void VKBuffer::UnmapStaging(VkDevice device)
{
    if (memoryRegionStaging_)
        memoryRegionStaging_->GetParentChunk()->Unmap(device);
}

void VKBuffer::UpdateStagingBuffer(VkDevice device, const void* data, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (memoryRegionStaging_)
    {
        auto deviceMemory = memoryRegionStaging_->GetParentChunk();
        if (auto memory = deviceMemory->Map(device, memoryRegionStaging_->GetOffset() + offset, dataSize))
        {
            /* Copy data to staging buffer and unmap */
            ::memcpy(memory, data, static_cast<std::size_t>(dataSize));
            deviceMemory->Unmap(device);
        }
    }
}

void VKBuffer::FlushStagingBuffer(VkDevice device, VkDeviceSize size, VkDeviceSize offset)
{
    if (memoryRegionStaging_)
    {
        /* Flush mapped memory to make it visible on the device */
        VkMappedMemoryRange memoryRange;
        {
            memoryRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.pNext   = nullptr;
            memoryRange.memory  = memoryRegionStaging_->GetParentChunk()->GetVkDeviceMemory();
            memoryRange.offset  = memoryRegionStaging_->GetOffset() + offset;
            memoryRange.size    = size;
        }
        auto result = vkFlushMappedMemoryRanges(device, 1, &memoryRange);
        VKThrowIfFailed(result, "failed to flush mapped memory range");
    }
}


} // /namespace LLGL



// ================================================================================
