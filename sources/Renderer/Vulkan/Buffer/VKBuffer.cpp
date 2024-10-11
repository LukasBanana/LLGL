/*
 * VKBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKBuffer.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../VKDevice.h"
#include "../Ext/VKExtensions.h"
#include "../Ext/VKExtensionRegistry.h"
#include "../../ResourceUtils.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


static constexpr VkDeviceSize k_xfbCounterSize = sizeof(std::uint32_t);

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
            flags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
            flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
        else
        {
            /* Error: feature not supported due to missing extension */
            LLGL_TRAP("stream output buffer not supported by Vulkan renderer");
        }
    }

    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0 || (desc.bindFlags & BindFlags::CopySrc) != 0)
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    return flags;
}

static VkAccessFlags GetBufferVkAccessFlags(long bindFlags)
{
    VkAccessFlags accessFlags = 0;

    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        accessFlags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        accessFlags |= VK_ACCESS_INDEX_READ_BIT;
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        accessFlags |= VK_ACCESS_UNIFORM_READ_BIT;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        accessFlags |= VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        accessFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if ((bindFlags & BindFlags::Sampled) != 0)
        accessFlags |= VK_ACCESS_SHADER_READ_BIT;
    if ((bindFlags & BindFlags::Storage) != 0)
        accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;

    return accessFlags;
}

static std::uint32_t GetVKBufferStride(const BufferDescriptor& desc)
{
    /* Just return first vertex attribute stride, since all attributes must have equal strides within the same buffer */
    return (desc.vertexAttribs.empty() ? 1 : std::max<std::uint32_t>(1u, desc.vertexAttribs[0].stride));
}

VKBuffer::VKBuffer(VkDevice device, const BufferDescriptor& desc) :
    Buffer            { desc.bindFlags                         },
    bufferObj_        { device                                 },
    bufferObjStaging_ { device                                 },
    size_             { desc.size                              },
    accessFlags_      { GetBufferVkAccessFlags(desc.bindFlags) },
    stride_           { GetVKBufferStride(desc)                }
{
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        indexType_ = VKTypes::ToVkIndexType(desc.format);

    VkBufferCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.size                     = GetInternalSize();
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

void* VKBuffer::Map(VKDevice& device, const CPUAccess access, VkDeviceSize offset, VkDeviceSize length)
{
    if (VkBuffer stagingBuffer = GetStagingVkBuffer())
    {
        /* Copy GPU local buffer into staging buffer for read accces */
        if (HasReadAccess(access))
            device.CopyBuffer(GetVkBuffer(), stagingBuffer, GetSize());

        if (HasWriteAccess(access))
        {
            mappedWriteRange_[0] = offset;
            mappedWriteRange_[1] = offset + length;
        }

        /* Map staging buffer */
        return bufferObjStaging_.Map(device);
    }
    return nullptr;
}

void VKBuffer::Unmap(VKDevice& device)
{
    if (VkBuffer stagingBuffer = GetStagingVkBuffer())
    {
        /* Unmap staging buffer */
        bufferObjStaging_.Unmap(device);

        /* Copy staging buffer into GPU local buffer for write access */
        if (mappedWriteRange_[0] < mappedWriteRange_[1])
        {
            const VkDeviceSize offset = mappedWriteRange_[0];
            const VkDeviceSize length = (mappedWriteRange_[1] - mappedWriteRange_[0]);
            device.CopyBuffer(stagingBuffer, GetVkBuffer(), length, offset, offset);
            mappedWriteRange_[0] = 0;
            mappedWriteRange_[1] = 0;
        }
    }
}

VkDeviceSize VKBuffer::GetInternalSize() const
{
    return ((GetBindFlags() & BindFlags::StreamOutputBuffer) != 0 ? GetSize() + k_xfbCounterSize : GetSize());
}

VkDeviceSize VKBuffer::GetXfbCounterOffset() const
{
    return ((GetBindFlags() & BindFlags::StreamOutputBuffer) != 0 ? GetSize() : 0);
}


} // /namespace LLGL



// ================================================================================
