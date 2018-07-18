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

void VKBuffer::BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    bufferObj_.BindMemoryRegion(device, memoryRegion);
}

void VKBuffer::TakeStagingBuffer(VKDeviceBuffer&& deviceBuffer)
{
    bufferObjStaging_ = std::move(deviceBuffer);
}

void* VKBuffer::Map(VkDevice device, const CPUAccess access)
{
    if (auto region = bufferObjStaging_.GetMemoryRegion())
    {
        mappingCPUAccess_ = access;
        return region->GetParentChunk()->Map(device, region->GetOffset(), region->GetSize());
    }
    return nullptr;
}

void VKBuffer::Unmap(VkDevice device)
{
    if (auto region = bufferObjStaging_.GetMemoryRegion())
        region->GetParentChunk()->Unmap(device);
}

void* VKBuffer::MapStaging(VkDevice device, VkDeviceSize dataSize, VkDeviceSize offset)
{
    if (auto region = bufferObjStaging_.GetMemoryRegion())
        return region->GetParentChunk()->Map(device, region->GetOffset() + offset, dataSize);
    else
        return nullptr;
}

void VKBuffer::UnmapStaging(VkDevice device)
{
    if (auto region = bufferObjStaging_.GetMemoryRegion())
        region->GetParentChunk()->Unmap(device);
}


} // /namespace LLGL



// ================================================================================
