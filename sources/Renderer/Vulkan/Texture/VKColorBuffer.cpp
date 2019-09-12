/*
 * VKColorBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKColorBuffer.h"


namespace LLGL
{


VKColorBuffer::VKColorBuffer(const VKPtr<VkDevice>& device) :
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
