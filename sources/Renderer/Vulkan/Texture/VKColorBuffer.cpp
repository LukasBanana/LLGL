/*
 * VKColorBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKColorBuffer.h"


namespace LLGL
{


VKColorBuffer::VKColorBuffer(VkDevice device) :
    VKRenderBuffer { device }
{
}

void VKColorBuffer::Create(
    VKDeviceMemoryManager&  deviceMemoryMngr,
    const Extent2D&         extent,
    VkFormat                format,
    VkSampleCountFlagBits   sampleCountBits)
{
    VKRenderBuffer::Create(
        deviceMemoryMngr,
        extent,
        format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        sampleCountBits,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    );
}

void VKColorBuffer::Release()
{
    VKRenderBuffer::Release();
}


} // /namespace LLGL



// ================================================================================
