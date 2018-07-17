/*
 * VKDevice.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDevice.h"
#include "VKCore.h"
#include "RenderState/VKFence.h"
#include <set>


namespace LLGL
{


/* ----- Common ----- */

VKDevice::VKDevice() :
    device_ { vkDestroyDevice }
{
}

void VKDevice::WaitIdle()
{
    vkDeviceWaitIdle(device_);
}

// Device-only layers are deprecated -> set 'enabledLayerCount' and 'ppEnabledLayerNames' members to zero during device creation.
// see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#extended-functionality-device-layer-deprecation
void VKDevice::CreateLogicalDevice(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures* features)
{
    const char* g_deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    };

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
        createInfo.enabledLayerCount        = 0;
        createInfo.ppEnabledLayerNames      = nullptr;
        createInfo.enabledExtensionCount    = (sizeof(g_deviceExtensions) / sizeof(g_deviceExtensions[0]));
        createInfo.ppEnabledExtensionNames  = g_deviceExtensions;
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
    VKPtr<VkCommandPool> commandPool;

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
        VKFence fence { device_ };

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
    VkImage         image,
    VkFormat        /*format*/,
    VkImageLayout   oldLayout,
    VkImageLayout   newLayout,
    std::uint32_t   numMipLevels,
    std::uint32_t   numArrayLayers)
{
    auto cmdBuffer = AllocCommandBuffer();

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
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = numMipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = numArrayLayers;
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
    vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    FlushCommandBuffer(cmdBuffer);
}

void VKDevice::CopyBuffer(
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size,
    VkDeviceSize    srcOffset,
    VkDeviceSize    dstOffset)
{
    //BeginStagingCommands();
    auto cmdBuffer = AllocCommandBuffer();

    /* Record copy command */
    VkBufferCopy region;
    {
        region.srcOffset    = srcOffset;
        region.dstOffset    = dstOffset;
        region.size         = size;
    }
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &region);

    FlushCommandBuffer(cmdBuffer);
}

void VKDevice::CopyBufferToImage(
    VkBuffer            srcBuffer,
    VkImage             dstImage,
    const VkExtent3D&   extent,
    std::uint32_t       numLayers)
{
    auto cmdBuffer = AllocCommandBuffer();

    /* Record copy command */
    VkBufferImageCopy region;
    {
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = 0;
        region.imageSubresource.baseArrayLayer  = 0;
        region.imageSubresource.layerCount      = numLayers;
        region.imageOffset                      = { 0, 0, 0 };
        region.imageExtent                      = extent;
    }
    vkCmdCopyBufferToImage(cmdBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    FlushCommandBuffer(cmdBuffer);
}


} // /namespace LLGL



// ================================================================================
