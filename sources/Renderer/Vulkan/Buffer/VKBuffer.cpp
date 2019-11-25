/*
 * VKBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKBuffer.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../Ext/VKExtensions.h"
#include "../Ext/VKExtensionRegistry.h"


namespace LLGL
{


static VkBufferUsageFlags GetVkBufferUsageFlags(const BufferDescriptor& desc)
{
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0)
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if ((desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if ((desc.bindFlags & BindFlags::IndirectBuffer) != 0)
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
    {
        if (HasExtension(VKExt::EXT_transform_feedback))
        {
            /* Enable transform feedback with extension VK_EXT_transform_feedback */
            flags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        }
        else
        {
            /* Error: feature not supported due to missing extension */
            throw std::runtime_error("stream output buffer not supported by Vulkan renderer");
        }
    }

    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0 || (desc.bindFlags & BindFlags::CopySrc) != 0)
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    return flags;
}

VKBuffer::VKBuffer(const VKPtr<VkDevice>& device, const BufferDescriptor& desc) :
    Buffer            { desc.bindFlags },
    bufferObj_        { device         },
    bufferObjStaging_ { device         },
    size_             { desc.size      }
{
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        indexType_ = VKTypes::ToVkIndexType(desc.format);

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

BufferDescriptor VKBuffer::GetDesc() const
{
    BufferDescriptor bufferDesc;

    bufferDesc.size             = GetSize();
    bufferDesc.bindFlags        = GetBindFlags();
    #if 0//TODO
    bufferDesc.cpuAccessFlags   = 0;
    bufferDesc.miscFlags        = 0;
    #endif

    return bufferDesc;
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
