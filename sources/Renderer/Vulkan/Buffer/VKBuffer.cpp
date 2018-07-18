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


VKBuffer::VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo) :
    Buffer            { type            },
    bufferObj_        { device          },
    bufferObjStaging_ { device          },
    size_             { createInfo.size }
{
    bufferObj_.CreateVkBuffer(device, createInfo);
}

void VKBuffer::BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    bufferObj_.BindToMemory(device, memoryRegion);
}

void VKBuffer::TakeStagingBuffer(VKDeviceBuffer&& deviceBuffer)
{
    bufferObjStaging_ = std::move(deviceBuffer);
}

void* VKBuffer::Map(VkDevice device, const CPUAccess access)
{
    if (auto region = GetMemoryRegionStaging())
    {
        mappingCPUAccess_ = access;
        return region->GetParentChunk()->Map(device, region->GetOffset(), region->GetSize());
    }
    return nullptr;
}

void VKBuffer::Unmap(VkDevice device)
{
    if (auto region = GetMemoryRegionStaging())
        region->GetParentChunk()->Unmap(device);
}

void* VKBuffer::MapStaging(VkDevice device, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (auto region = GetMemoryRegionStaging())
        return region->GetParentChunk()->Map(device, region->GetOffset() + offset, dataSize);
    else
        return nullptr;
}

void VKBuffer::UnmapStaging(VkDevice device)
{
    if (auto region = GetMemoryRegionStaging())
        region->GetParentChunk()->Unmap(device);
}

void VKBuffer::UpdateStagingBuffer(VkDevice device, const void* data, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (auto region = GetMemoryRegionStaging())
    {
        auto deviceMemory = region->GetParentChunk();
        if (auto memory = deviceMemory->Map(device, region->GetOffset() + offset, dataSize))
        {
            /* Copy data to staging buffer and unmap */
            ::memcpy(memory, data, static_cast<std::size_t>(dataSize));
            deviceMemory->Unmap(device);
        }
    }
}

void VKBuffer::FlushStagingBuffer(VkDevice device, VkDeviceSize size, VkDeviceSize offset)
{
    if (auto region = GetMemoryRegionStaging())
    {
        /* Flush mapped memory to make it visible on the device */
        VkMappedMemoryRange memoryRange;
        {
            memoryRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.pNext   = nullptr;
            memoryRange.memory  = region->GetParentChunk()->GetVkDeviceMemory();
            memoryRange.offset  = region->GetOffset() + offset;
            memoryRange.size    = size;
        }
        auto result = vkFlushMappedMemoryRanges(device, 1, &memoryRange);
        VKThrowIfFailed(result, "failed to flush mapped memory range");
    }
}


} // /namespace LLGL



// ================================================================================
