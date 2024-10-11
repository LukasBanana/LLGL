/*
 * VKDevice.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDevice.h"
#include "VKTypes.h"
#include "RenderState/VKFence.h"
#include "Buffer/VKBuffer.h"
#include "Texture/VKTexture.h"
#include "Memory/VKDeviceMemoryRegion.h"
#include "Memory/VKDeviceMemory.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>
#include <limits.h>


namespace LLGL
{


/* ----- Common ----- */

VKDevice::VKDevice() :
    device_      { vkDestroyDevice               },
    commandPool_ { device_, vkDestroyCommandPool }
{
}

VKDevice::VKDevice(VKDevice&& device) :
    device_             { std::move(device.device_)      },
    queueFamilyIndices_ { device.queueFamilyIndices_     },
    graphicsQueue_      { device.graphicsQueue_          },
    commandPool_        { std::move(device.commandPool_) }
{
}

VKDevice& VKDevice::operator = (VKDevice&& device)
{
    device_             = std::move(device.device_);
    queueFamilyIndices_ = device.queueFamilyIndices_;
    graphicsQueue_      = device.graphicsQueue_;
    commandPool_        = std::move(device.commandPool_);
    return *this;
}

void VKDevice::WaitIdle()
{
    vkDeviceWaitIdle(device_);
}

// Device-only layers are deprecated -> set 'enabledLayerCount' and 'ppEnabledLayerNames' members to zero during device creation.
// see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#extended-functionality-device-layer-deprecation
void VKDevice::CreateLogicalDevice(
    VkPhysicalDevice                    physicalDevice,
    const VkPhysicalDeviceFeatures2*    features,
    const char* const*                  extensions,
    std::uint32_t                       numExtensions)
{
    /* Initialize queue create description */
    queueFamilyIndices_ = VKFindQueueFamilies(physicalDevice, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    SmallVector<VkDeviceQueueCreateInfo, 2> queueCreateInfos;

    auto AddQueueFamily = [&queueCreateInfos](std::uint32_t family, float queuePriority)
    {
        VkDeviceQueueCreateInfo info;
        {
            info.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.pNext              = nullptr;
            info.flags              = 0;
            info.queueFamilyIndex   = family;
            info.queueCount         = 1;
            info.pQueuePriorities   = &queuePriority;
        }
        queueCreateInfos.push_back(info);
    };

    constexpr float queuePriority = 1.0f;

    AddQueueFamily(queueFamilyIndices_.graphicsFamily, queuePriority);

    if (queueFamilyIndices_.graphicsFamily != queueFamilyIndices_.presentFamily)
        AddQueueFamily(queueFamilyIndices_.presentFamily, queuePriority);

    /* Create logical device */
    VkDeviceCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.flags                    = 0;
        createInfo.queueCreateInfoCount     = static_cast<std::uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.enabledLayerCount        = 0;        // deprecated and ignored
        createInfo.ppEnabledLayerNames      = nullptr;  // deprecated and ignored
        createInfo.enabledExtensionCount    = numExtensions;
        createInfo.ppEnabledExtensionNames  = extensions;

        /* Must pass the feature flags either through the chain of pNext (Vulkan 1.1+), or only through pEnabledFeatures (Vulkan 1.0) */
        if (features->pNext != nullptr)
        {
            createInfo.pNext                = features;
            createInfo.pEnabledFeatures     = nullptr;
        }
        else
        {
            createInfo.pNext                = nullptr;
            createInfo.pEnabledFeatures     = &(features->features);
        }
    }
    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, device_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan logical device");

    /* Query device graphics queue */
    vkGetDeviceQueue(device_, queueFamilyIndices_.graphicsFamily, 0, &graphicsQueue_);

    /* Create default command pool */
    commandPool_ = CreateCommandPool();
}

void VKDevice::LoadLogicalDeviceWeakRef(VkPhysicalDevice physicalDevice, VkDevice device)
{
    /* Initialize queue create description */
    queueFamilyIndices_ = VKFindQueueFamilies(physicalDevice, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    /* Store weak reference to logical Vulkan device */
    device_ = VKPtr<VkDevice>{ device };

    /* Query device graphics queue */
    vkGetDeviceQueue(device_, queueFamilyIndices_.graphicsFamily, 0, &graphicsQueue_);

    /* Create default command pool */
    commandPool_ = CreateCommandPool();
}

VKPtr<VkCommandPool> VKDevice::CreateCommandPool()
{
    VKPtr<VkCommandPool> commandPool{ device_, vkDestroyCommandPool };

    /* Create staging command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndices_.graphicsFamily;
    }
    VkResult result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");

    return commandPool;
}

VkCommandBuffer VKDevice::AllocCommandBuffer(bool begin)
{
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    /* Allocate new primary level command buffer via staging command pool */
    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                = nullptr;
        allocInfo.commandPool          = commandPool_;
        allocInfo.level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount   = 1;
    }
    VkResult result = vkAllocateCommandBuffers(device_, &allocInfo, &cmdBuffer);
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffer");

    /* Begin command buffer recording (if enabled) */
    if (begin)
    {
        VkCommandBufferBeginInfo beginInfo;
        {
            beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext             = nullptr;
            beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo  = nullptr;
        }
        result = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        VKThrowIfFailed(result, "failed to begin recording Vulkan command buffer");
    }

    return cmdBuffer;
}

void VKDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer, bool release)
{
    /* End command buffer record */
    VkResult result = vkEndCommandBuffer(cmdBuffer);
    VKThrowIfFailed(result, "failed to end recording Vulkan command buffer");

    /* Create fence to ensure the command buffer has finished execution */
    {
        VKFence fence{ device_ };

        /* Submit command buffer to queue */
        VkSubmitInfo submitInfo = {};
        {
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = (&cmdBuffer);
        }
        vkQueueSubmit(graphicsQueue_, 1, &submitInfo, fence.GetVkFence());

        /* Wait for fence to be signaled */
        fence.Wait(device_, ULLONG_MAX);
    }

    /* Release command buffer (if enabled) */
    if (release)
        vkFreeCommandBuffers(device_, commandPool_, 1, &cmdBuffer);
}

void VKDevice::CopyBuffer(
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size,
    VkDeviceSize    srcOffset,
    VkDeviceSize    dstOffset)
{
    VkCommandBuffer cmdBuffer = AllocCommandBuffer();
    {
        VkBufferCopy region;
        {
            region.srcOffset    = srcOffset;
            region.dstOffset    = dstOffset;
            region.size         = size;
        }
        vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &region);
    }
    FlushCommandBuffer(cmdBuffer);
}

void VKDevice::WriteBuffer(VKDeviceBuffer& buffer, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    if (VKDeviceMemoryRegion* region = buffer.GetMemoryRegion())
    {
        /* Map buffer memory to host memory */
        VKDeviceMemory* deviceMemory = region->GetParentChunk();
        if (void* memory = deviceMemory->Map(device_, region->GetOffset() + offset, size))
        {
            /* Copy input data to buffer memory */
            ::memcpy(memory, data, static_cast<std::size_t>(size));
            deviceMemory->Unmap(device_);
        }
    }
}

void VKDevice::ReadBuffer(VKDeviceBuffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset)
{
    if (VKDeviceMemoryRegion* region = buffer.GetMemoryRegion())
    {
        /* Map buffer memory to host memory */
        VKDeviceMemory* deviceMemory = region->GetParentChunk();
        if (const void* memory = deviceMemory->Map(device_, region->GetOffset() + offset, size))
        {
            /* Copy buffer memory to output data */
            ::memcpy(data, memory, static_cast<std::size_t>(size));
            deviceMemory->Unmap(device_);
        }
    }
}

void VKDevice::FlushMappedBuffer(VKDeviceBuffer& buffer, VkDeviceSize size, VkDeviceSize offset)
{
    if (VKDeviceMemoryRegion* region = buffer.GetMemoryRegion())
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
        VkResult result = vkFlushMappedMemoryRanges(device_, 1, &memoryRange);
        VKThrowIfFailed(result, "failed to flush mapped memory range");
    }
}


} // /namespace LLGL



// ================================================================================
