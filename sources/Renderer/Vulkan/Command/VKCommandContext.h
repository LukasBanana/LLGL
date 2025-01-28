/*
 * VKCommandContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_CONTEXT_H
#define LLGL_VK_COMMAND_CONTEXT_H


#include "../VKPtr.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <cstdint>


namespace LLGL
{


struct TextureSubresource;
class VKBuffer;
class VKTexture;

class VKCommandContext
{

    public:

        VKCommandContext();

        VKCommandContext(VkCommandBuffer commandBuffer);

        void Reset(VkCommandBuffer commandBuffer);

        /* --- Memory barriers --- */

        void BufferMemoryBarrier(
            VkBuffer                    buffer,
            VkDeviceSize                offset,
            VkDeviceSize                size,
            VkAccessFlags               srcAccessMask       = 0,
            VkAccessFlags               dstAccessMask       = 0,
            bool                        flushImmediately    = false
        );

        void ImageMemoryBarrier(
            VkImage                     image,
            VkFormat                    format,
            VkImageLayout               oldLayout,
            VkImageLayout               newLayout,
            const TextureSubresource&   subresource,
            bool                        flushImmediately    = false
        );

        // Submits this pipeline barrier into the current command buffer.
        void FlushBarriers();

        /* --- Resource operations --- */

        void CopyBuffer(
            VkBuffer        srcBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    size,
            VkDeviceSize    srcOffset = 0,
            VkDeviceSize    dstOffset = 0
        );

        void CopyTexture(
            VKTexture&          srcTexture,
            VKTexture&          dstTexture,
            const VkImageCopy&  region
        );

        void CopyImage(
            VkImage             srcImage,
            VkImageLayout       srcImageLayout,
            VkImage             dstImage,
            VkImageLayout       dstImageLayout,
            const VkImageCopy&  region,
            VkFormat            format
        );

        void ResolveImage(
            VkImage                 srcImage,
            VkImageLayout           srcImageLayout,
            VkImage                 dstImage,
            VkImageLayout           dstImageLayout,
            const VkImageResolve&   region,
            VkFormat                format
        );

        // Copies the source buffer into the destination image (numMipLevels must be 1).
        void CopyBufferToImage(
            VkBuffer                    srcBuffer,
            VkImage                     dstImage,
            VkFormat                    format,
            const VkOffset3D&           offset,
            const VkExtent3D&           extent,
            const TextureSubresource&   subresource,
            std::uint32_t               rowLength       = 0,
            std::uint32_t               imageHeight     = 0
        );

        void CopyBufferToImage(
            VKBuffer&                   srcBuffer,
            VKTexture&                  dstTexture,
            const VkBufferImageCopy&    region
        );

        // Copies the source image into the destination buffer (numMipLevels must be 1).
        void CopyImageToBuffer(
            VkImage                     srcImage,
            VkBuffer                    dstBuffer,
            VkFormat                    format,
            const VkOffset3D&           offset,
            const VkExtent3D&           extent,
            const TextureSubresource&   subresource
        );

        void CopyImageToBuffer(
            VKTexture&                  srcTexture,
            VKBuffer&                   dstBuffer,
            const VkBufferImageCopy&    region
        );

        void GenerateMips(
            VkImage                     image,
            VkFormat                    format,
            const VkExtent3D&           extent,
            const TextureSubresource&   subresource
        );

    private:

        static constexpr std::uint32_t maxNumBarriers = 4;

    private:

        VkCommandBuffer         commandBuffer_                  = VK_NULL_HANDLE;

        VkPipelineStageFlags    srcStageMask_                   = 0;
        VkPipelineStageFlags    dstStageMask_                   = 0;

        std::uint32_t           numMemoryBarriers_ : 4;
        std::uint32_t           numBufferBarriers_ : 4;
        std::uint32_t           numImageBarriers_  : 4;

        VkMemoryBarrier         memoryBarriers_[maxNumBarriers];
        VkBufferMemoryBarrier   bufferBarriers_[maxNumBarriers];
        VkImageMemoryBarrier    imageBarriers_[maxNumBarriers];

};


} // /namespace LLGL


#endif



// ================================================================================
