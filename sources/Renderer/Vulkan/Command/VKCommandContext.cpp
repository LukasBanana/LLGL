/*
 * VKCommandContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCommandContext.h"
#include "../VKTypes.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKTexture.h"
#include "../Texture/VKImageUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{

VKCommandContext::VKCommandContext() :
    VKCommandContext { VK_NULL_HANDLE }
{
}

VKCommandContext::VKCommandContext(VkCommandBuffer commandBuffer) :
    commandBuffer_     { commandBuffer },
    numMemoryBarriers_ { 0             },
    numBufferBarriers_ { 0             },
    numImageBarriers_  { 0             }
{
    /* Initialize default structure members of all barriers */
    for (VkMemoryBarrier& barrier : memoryBarriers_)
    {
        barrier.sType               = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.pNext               = nullptr;
    }
    for (VkBufferMemoryBarrier& barrier : bufferBarriers_)
    {
        barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext               = nullptr;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
    for (VkImageMemoryBarrier& barrier : imageBarriers_)
    {
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext               = nullptr;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
}


void VKCommandContext::Reset(VkCommandBuffer commandBuffer)
{
    LLGL_ASSERT(numMemoryBarriers_ == 0, "memory barriers have not be flushed before end of previous command buffer");
    LLGL_ASSERT(numBufferBarriers_ == 0, "buffer memory barriers have not be flushed before end of previous command buffer");
    LLGL_ASSERT(numImageBarriers_ == 0, "image memory barriers have not be flushed before end of previous command buffer");
    commandBuffer_ = commandBuffer;
}

void VKCommandContext::BufferMemoryBarrier(
    VkBuffer        buffer,
    VkDeviceSize    offset,
    VkDeviceSize    size,
    VkAccessFlags   srcAccessMask,
    VkAccessFlags   dstAccessMask,
    bool            flushImmediately)
{
    if (numBufferBarriers_ == maxNumBarriers)
        FlushBarriers();

    /* Initialize buffer memory barrier descriptor */
    VkBufferMemoryBarrier& barrier = bufferBarriers_[numBufferBarriers_++];
    {
        barrier.srcAccessMask   = srcAccessMask;
        barrier.dstAccessMask   = dstAccessMask;
        barrier.buffer          = buffer;
        barrier.offset          = offset;
        barrier.size            = size;
    }

    /* Initialize pipeline state flags */
    srcStageMask_ |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStageMask_ |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    if (flushImmediately)
        FlushBarriers();
}

void VKCommandContext::ImageMemoryBarrier(
    VkImage                     image,
    VkFormat                    format,
    VkImageLayout               oldLayout,
    VkImageLayout               newLayout,
    const TextureSubresource&   subresource,
    bool                        flushImmediately)
{
    if (numImageBarriers_ == maxNumBarriers)
        FlushBarriers();

    /* Initialize image memory barrier descriptor */
    VkImageMemoryBarrier& barrier = imageBarriers_[numImageBarriers_++];
    {
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VKImageUtils::GetInclusiveVkImageAspect(format);
        barrier.subresourceRange.baseMipLevel   = subresource.baseMipLevel;
        barrier.subresourceRange.levelCount     = subresource.numMipLevels;
        barrier.subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        barrier.subresourceRange.layerCount     = subresource.numArrayLayers;
    }

    /* Initialize pipeline state flags */
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

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
    else
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
    }

    srcStageMask_ |= srcStageMask;
    dstStageMask_ |= dstStageMask;

    if (flushImmediately)
        FlushBarriers();
}

void VKCommandContext::FlushBarriers()
{
    if (numMemoryBarriers_ > 0 || numBufferBarriers_ > 0 || numImageBarriers_ > 0)
    {
        vkCmdPipelineBarrier(
            commandBuffer_,
            srcStageMask_,
            dstStageMask_,
            0, // VkDependencyFlags
            numMemoryBarriers_,
            memoryBarriers_,
            numBufferBarriers_,
            bufferBarriers_,
            numImageBarriers_,
            imageBarriers_
        );
        numMemoryBarriers_  = 0;
        numBufferBarriers_  = 0;
        numImageBarriers_   = 0;
        srcStageMask_       = 0;
        dstStageMask_       = 0;
    }
}

void VKCommandContext::CopyBuffer(
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
    vkCmdCopyBuffer(commandBuffer_, srcBuffer, dstBuffer, 1, &region);
}

void VKCommandContext::CopyTexture(
    VKTexture&          srcTexture,
    VKTexture&          dstTexture,
    const VkImageCopy&  region)
{
    vkCmdCopyImage(
        commandBuffer_,
        srcTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void VKCommandContext::CopyImage(
    VkImage             srcImage,
    VkImageLayout       srcImageLayout,
    VkImage             dstImage,
    VkImageLayout       dstImageLayout,
    const VkImageCopy&  region,
    VkFormat            format)
{
    const TextureSubresource srcImageSubresource{ region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount, region.srcSubresource.mipLevel, 1u };
    const TextureSubresource dstImageSubresource{ region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount, region.dstSubresource.mipLevel, 1u };

    ImageMemoryBarrier(srcImage, format, srcImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageSubresource);
    ImageMemoryBarrier(dstImage, format, dstImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageSubresource, true);

    vkCmdCopyImage(commandBuffer_, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    ImageMemoryBarrier(srcImage, format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageLayout, srcImageSubresource);
    ImageMemoryBarrier(dstImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageLayout, dstImageSubresource, true);
}

void VKCommandContext::ResolveImage(
    VkImage                 srcImage,
    VkImageLayout           srcImageLayout,
    VkImage                 dstImage,
    VkImageLayout           dstImageLayout,
    const VkImageResolve&   region,
    VkFormat                format)
{
    const TextureSubresource srcImageSubresource{ region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount, region.srcSubresource.mipLevel, 1u };
    const TextureSubresource dstImageSubresource{ region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount, region.dstSubresource.mipLevel, 1u };

    ImageMemoryBarrier(srcImage, format, srcImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageSubresource);
    ImageMemoryBarrier(dstImage, format, dstImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageSubresource, true);

    vkCmdResolveImage(commandBuffer_, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    ImageMemoryBarrier(srcImage, format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageLayout, srcImageSubresource);
    ImageMemoryBarrier(dstImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageLayout, dstImageSubresource, true);
}

void VKCommandContext::CopyBufferToImage(
    VkBuffer                    srcBuffer,
    VkImage                     dstImage,
    VkFormat                    format,
    const VkOffset3D&           offset,
    const VkExtent3D&           extent,
    const TextureSubresource&   subresource,
    std::uint32_t               rowLength,
    std::uint32_t               imageHeight)
{
    VkBufferImageCopy region;
    {
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = rowLength;
        region.bufferImageHeight                = imageHeight;
        region.imageSubresource.aspectMask      = VKImageUtils::GetInclusiveVkImageAspect(format);
        region.imageSubresource.mipLevel        = subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = subresource.numArrayLayers;
        region.imageOffset                      = offset;
        region.imageExtent                      = extent;
    }
    vkCmdCopyBufferToImage(commandBuffer_, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VKCommandContext::CopyBufferToImage(
    VKBuffer&                   srcBuffer,
    VKTexture&                  dstTexture,
    const VkBufferImageCopy&    region)
{
    vkCmdCopyBufferToImage(
        commandBuffer_,
        srcBuffer.GetVkBuffer(),
        dstTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

void VKCommandContext::CopyImageToBuffer(
    VkImage                     srcImage,
    VkBuffer                    dstBuffer,
    VkFormat                    format,
    const VkOffset3D&           offset,
    const VkExtent3D&           extent,
    const TextureSubresource&   subresource)
{
    VkBufferImageCopy region;
    {
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VKImageUtils::GetInclusiveVkImageAspect(format);
        region.imageSubresource.mipLevel        = subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = subresource.numArrayLayers;
        region.imageOffset                      = offset;
        region.imageExtent                      = extent;
    }
    vkCmdCopyImageToBuffer(commandBuffer_, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer, 1, &region);
}

void VKCommandContext::CopyImageToBuffer(
    VKTexture&                  srcTexture,
    VKBuffer&                   dstBuffer,
    const VkBufferImageCopy&    region)
{
    vkCmdCopyImageToBuffer(
        commandBuffer_,
        srcTexture.GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstBuffer.GetVkBuffer(),
        1,
        &region
    );
}

void VKCommandContext::GenerateMips(
    VkImage                     image,
    VkFormat                    format,
    const VkExtent3D&           extent,
    const TextureSubresource&   subresource)
{
    ImageMemoryBarrier(
        image,
        VK_FORMAT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresource,
        true
    );

    /* Initialize image memory barrier */
    VkImageMemoryBarrier barrier;

    const VkImageAspectFlags aspectMask = VKImageUtils::GetInclusiveVkImageAspect(format);

    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = nullptr;
    barrier.srcAccessMask                   = 0;
    barrier.dstAccessMask                   = 0;
    barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = aspectMask;
    barrier.subresourceRange.baseMipLevel   = subresource.baseMipLevel;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
    barrier.subresourceRange.layerCount     = 1;

    /* Blit each MIP-map from previous (lower) MIP level */
    for_range(arrayLayer, subresource.numArrayLayers)
    {
        VkExtent3D currExtent = extent;

        for_subrange(mipLevel, 1, subresource.numMipLevels)
        {
            /* Determine extent of next MIP level */
            VkExtent3D nextExtent = currExtent;

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
                commandBuffer_,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            /* Blit previous MIP level into next higher MIP level (with smaller extent) */
            VkImageBlit blit;

            blit.srcSubresource.aspectMask      = aspectMask;
            blit.srcSubresource.mipLevel        = subresource.baseMipLevel + mipLevel - 1;
            blit.srcSubresource.baseArrayLayer  = subresource.baseArrayLayer + arrayLayer;
            blit.srcSubresource.layerCount      = 1;
            blit.srcOffsets[0]                  = { 0, 0, 0 };
            blit.srcOffsets[1].x                = static_cast<std::int32_t>(currExtent.width);
            blit.srcOffsets[1].y                = static_cast<std::int32_t>(currExtent.height);
            blit.srcOffsets[1].z                = static_cast<std::int32_t>(currExtent.depth);
            blit.dstSubresource.aspectMask      = aspectMask;
            blit.dstSubresource.mipLevel        = subresource.baseMipLevel + mipLevel;
            blit.dstSubresource.baseArrayLayer  = subresource.baseArrayLayer + arrayLayer;
            blit.dstSubresource.layerCount      = 1;
            blit.dstOffsets[0]                  = { 0, 0, 0 };
            blit.dstOffsets[1].x                = static_cast<std::int32_t>(nextExtent.width);
            blit.dstOffsets[1].y                = static_cast<std::int32_t>(nextExtent.height);
            blit.dstOffsets[1].z                = static_cast<std::int32_t>(nextExtent.depth);

            vkCmdBlitImage(
                commandBuffer_,
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
                commandBuffer_,
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
            commandBuffer_,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }
}


} // /namespace LLGL



// ================================================================================
