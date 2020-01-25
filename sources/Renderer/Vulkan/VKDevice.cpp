/*
 * VKDevice.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDevice.h"
#include "RenderState/VKFence.h"
#include "Buffer/VKBuffer.h"
#include "Texture/VKTexture.h"
#include "Memory/VKDeviceMemoryRegion.h"
#include "Memory/VKDeviceMemory.h"
#include <set>
#include <algorithm>
#include <string.h>


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
    VkPhysicalDevice                physicalDevice,
    const VkPhysicalDeviceFeatures* features,
    const char* const*              extensions,
    std::uint32_t                   numExtensions)
{
    /* Initialize queue create description */
    queueFamilyIndices_ = VKFindQueueFamilies(physicalDevice, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = { queueFamilyIndices_.graphicsFamily, queueFamilyIndices_.presentFamily };

    float queuePriority = 1.0f;
    for (auto family : uniqueQueueFamilies)
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
    }

    /* Create logical device */
    VkDeviceCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.queueCreateInfoCount     = static_cast<std::uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.enabledLayerCount        = 0;        // deprecated and ignored
        createInfo.ppEnabledLayerNames      = nullptr;  // deprecated and ignored
        createInfo.enabledExtensionCount    = numExtensions;
        createInfo.ppEnabledExtensionNames  = extensions;
        createInfo.pEnabledFeatures         = features;
    }
    auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, device_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan logical device");

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
    auto result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool.ReleaseAndGetAddressOf());
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
    auto result = vkAllocateCommandBuffers(device_, &allocInfo, &cmdBuffer);
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
    auto result = vkEndCommandBuffer(cmdBuffer);
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
        fence.Wait(device_, std::numeric_limits<std::uint64_t>::max());
    }

    /* Release command buffer (if enabled) */
    if (release)
        vkFreeCommandBuffers(device_, commandPool_, 1, &cmdBuffer);
}

void VKDevice::TransitionImageLayout(
    VkCommandBuffer             commandBuffer,
    VkImage                     image,
    VkFormat                    /*format*/,
    VkImageLayout               oldLayout,
    VkImageLayout               newLayout,
    const TextureSubresource&   subresource)
{
    /* Initialize image memory barrier descriptor */
    VkImageMemoryBarrier barrier;
    {
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext                           = nullptr;
        barrier.srcAccessMask                   = 0;
        barrier.dstAccessMask                   = 0;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = subresource.baseMipLevel;
        barrier.subresourceRange.levelCount     = subresource.numMipLevels;
        barrier.subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        barrier.subresourceRange.layerCount     = subresource.numArrayLayers;
    }

    /* Initialize pipeline state flags */
    VkPipelineStageFlags srcStageMask = 0;
    VkPipelineStageFlags dstStageMask = 0;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    /* Record image barrier command */
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VKDevice::CopyBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size,
    VkDeviceSize    srcOffset,
    VkDeviceSize    dstOffset)
{
    VkBufferCopy region;
    {
        region.srcOffset    = srcOffset;
        region.dstOffset    = dstOffset;
        region.size         = size;
    }
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);
}

void VKDevice::CopyBuffer(
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size,
    VkDeviceSize    srcOffset,
    VkDeviceSize    dstOffset)
{
    auto cmdBuffer = AllocCommandBuffer();
    {
        CopyBuffer(cmdBuffer, srcBuffer, dstBuffer, size, srcOffset, dstOffset);
    }
    FlushCommandBuffer(cmdBuffer);
}

void VKDevice::CopyTexture(
    VkCommandBuffer     commandBuffer,
    VKTexture&          srcTexture,
    VKTexture&          dstTexture,
    const VkImageCopy&  region)
{
    vkCmdCopyImage(
        commandBuffer,
        srcTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void VKDevice::CopyBufferToImage(
    VkCommandBuffer             commandBuffer,
    VkBuffer                    srcBuffer,
    VkImage                     dstImage,
    const VkOffset3D&           offset,
    const VkExtent3D&           extent,
    const TextureSubresource&   subresource)
{
    VkBufferImageCopy region;
    {
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = subresource.numArrayLayers;
        region.imageOffset                      = offset;
        region.imageExtent                      = extent;
    }
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VKDevice::CopyBufferToImage(
    VkCommandBuffer             commandBuffer,
    VKBuffer&                   srcBuffer,
    VKTexture&                  dstTexture,
    const VkBufferImageCopy&    region)
{
    vkCmdCopyBufferToImage(
        commandBuffer,
        srcBuffer.GetVkBuffer(),
        dstTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void VKDevice::CopyImageToBuffer(
    VkCommandBuffer             commandBuffer,
    VkImage                     srcImage,
    VkBuffer                    dstBuffer,
    const VkOffset3D&           offset,
    const VkExtent3D&           extent,
    const TextureSubresource&   subresource)
{
    VkBufferImageCopy region;
    {
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = subresource.numArrayLayers;
        region.imageOffset                      = offset;
        region.imageExtent                      = extent;
    }
    vkCmdCopyImageToBuffer(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer, 1, &region);
}

void VKDevice::CopyImageToBuffer(
    VkCommandBuffer             commandBuffer,
    VKTexture&                  srcTexture,
    VKBuffer&                   dstBuffer,
    const VkBufferImageCopy&    region)
{
    vkCmdCopyImageToBuffer(
        commandBuffer,
        srcTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        dstBuffer.GetVkBuffer(),
        1,
        &region
    );
}

void VKDevice::GenerateMips(
    VkCommandBuffer             commandBuffer,
    VkImage                     image,
    const VkExtent3D&           imageExtent,
    const TextureSubresource&   subresource)
{
    TransitionImageLayout(
        commandBuffer,
        image,
        VK_FORMAT_UNDEFINED,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresource
    );

    /* Initialize image memory barrier */
    VkImageMemoryBarrier barrier;

    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = nullptr;
    barrier.srcAccessMask                   = 0;
    barrier.dstAccessMask                   = 0;
    barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = subresource.baseMipLevel;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
    barrier.subresourceRange.layerCount     = 1;

    /* Blit each MIP-map from previous (lower) MIP level */
    for (std::uint32_t arrayLayer = 0; arrayLayer < subresource.numArrayLayers; ++arrayLayer)
    {
        auto currExtent = imageExtent;

        for (std::uint32_t mipLevel = 1; mipLevel < subresource.numMipLevels; ++mipLevel)
        {
            /* Determine extent of next MIP level */
            auto nextExtent = currExtent;

            nextExtent.width    = std::max(1u, currExtent.width  / 2);
            nextExtent.height   = std::max(1u, currExtent.height / 2);
            nextExtent.depth    = std::max(1u, currExtent.depth  / 2);

            /* Transition previous MIP level to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL */
            barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.subresourceRange.baseMipLevel   = subresource.baseMipLevel + mipLevel - 1;
            barrier.subresourceRange.baseArrayLayer = subresource.baseArrayLayer + arrayLayer;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            /* Blit previous MIP level into next higher MIP level (with smaller extent) */
            VkImageBlit blit;

            blit.srcSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel        = subresource.baseMipLevel + mipLevel - 1;
            blit.srcSubresource.baseArrayLayer  = subresource.baseArrayLayer + arrayLayer;
            blit.srcSubresource.layerCount      = 1;
            blit.srcOffsets[0]                  = { 0, 0, 0 };
            blit.srcOffsets[1].x                = static_cast<std::int32_t>(currExtent.width);
            blit.srcOffsets[1].y                = static_cast<std::int32_t>(currExtent.height);
            blit.srcOffsets[1].z                = static_cast<std::int32_t>(currExtent.depth);
            blit.dstSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel        = subresource.baseMipLevel + mipLevel;
            blit.dstSubresource.baseArrayLayer  = subresource.baseArrayLayer + arrayLayer;
            blit.dstSubresource.layerCount      = 1;
            blit.dstOffsets[0]                  = { 0, 0, 0 };
            blit.dstOffsets[1].x                = static_cast<std::int32_t>(nextExtent.width);
            blit.dstOffsets[1].y                = static_cast<std::int32_t>(nextExtent.height);
            blit.dstOffsets[1].z                = static_cast<std::int32_t>(nextExtent.depth);

            vkCmdBlitImage(
                commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            /* Transition previous MIP level back to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */
            barrier.srcAccessMask   = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            /* Reduce image extent to next MIP level */
            currExtent = nextExtent;
        }

        /* Transition last MIP level back to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */
        barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.subresourceRange.baseMipLevel   = subresource.numMipLevels - 1;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }
}

void VKDevice::WriteBuffer(VKDeviceBuffer& buffer, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    if (auto region = buffer.GetMemoryRegion())
    {
        /* Map buffer memory to host memory */
        auto deviceMemory = region->GetParentChunk();
        if (auto memory = deviceMemory->Map(device_, region->GetOffset() + offset, size))
        {
            /* Copy data to buffer object */
            ::memcpy(memory, data, static_cast<std::size_t>(size));
            deviceMemory->Unmap(device_);
        }
    }
}

void VKDevice::FlushMappedBuffer(VKDeviceBuffer& buffer, VkDeviceSize size, VkDeviceSize offset)
{
    if (auto region = buffer.GetMemoryRegion())
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
        auto result = vkFlushMappedMemoryRanges(device_, 1, &memoryRange);
        VKThrowIfFailed(result, "failed to flush mapped memory range");
    }
}


} // /namespace LLGL



// ================================================================================
