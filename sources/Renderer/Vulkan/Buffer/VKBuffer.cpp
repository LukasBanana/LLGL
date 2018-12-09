/*
 * VKBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKBuffer.h"
#include "../VKCore.h"
#include "../VKTypes.h"


namespace LLGL
{


static VkIndexType MapIndexType(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::UInt16:  return VK_INDEX_TYPE_UINT16;
        case DataType::UInt32:  return VK_INDEX_TYPE_UINT32;
        default:                break;
    }
    VKTypes::MapFailed("DataType", "VkIndexType");
}

static VkBufferUsageFlags GetVkBufferUsageFlags(const BufferDescriptor& desc)
{
    VkBufferUsageFlags flags = 0;

    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0)
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if ((desc.bindFlags & (BindFlags::SampleBuffer | BindFlags::RWStorageBuffer)) != 0)
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::IndirectBuffer) != 0)
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    #ifdef LLGL_VK_ENABLE_EXT
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    #else
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
        throw std::runtime_error("stream output buffer not supported by Vulkan renderer");
    #endif

    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0)
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    else
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    return flags;
}

VKBuffer::VKBuffer(const VKPtr<VkDevice>& device, const BufferDescriptor& desc) :
    Buffer            { desc.bindFlags                                      },
    bufferObj_        { device                                              },
    bufferObjStaging_ { device                                              },
    size_             { desc.size                                           },
    indexType_        { MapIndexType(desc.indexBuffer.format.GetDataType()) }
{
    VkBufferCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.size                     = desc.size;
        createInfo.usage                    = GetVkBufferUsageFlags(desc);
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
    }
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
    mappedCPUAccess_ = access;
    return bufferObjStaging_.Map(device);
}

void VKBuffer::Unmap(VkDevice device)
{
    bufferObjStaging_.Unmap(device);
}


} // /namespace LLGL



// ================================================================================
