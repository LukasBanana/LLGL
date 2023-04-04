/*
 * VKDepthStencilBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DEPTH_STENCIL_BUFFER_H
#define LLGL_VK_DEPTH_STENCIL_BUFFER_H


#include "VKRenderBuffer.h"


namespace LLGL
{


class VKDepthStencilBuffer final : private VKRenderBuffer
{

    public:

        VKDepthStencilBuffer(VkDevice device);

        void Create(
            VKDeviceMemoryManager&  deviceMemoryMngr,
            const Extent2D&         extent,
            VkFormat                format,
            VkSampleCountFlagBits   sampleCountBits
        );

        void Release();

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return VKRenderBuffer::GetVkImage();
        }

        // Returns the Vulkan image object.
        inline VkImageView GetVkImageView() const
        {
            return VKRenderBuffer::GetVkImageView();
        }

        // Returns the VkFormat with whereby the VkImage object was created.
        inline VkFormat GetVkFormat() const
        {
            return VKRenderBuffer::GetVkFormat();
        }

};


} // /namespace LLGL


#endif



// ================================================================================
