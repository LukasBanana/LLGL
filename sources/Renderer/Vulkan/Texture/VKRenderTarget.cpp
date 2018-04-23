/*
 * VKRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderTarget.h"
#include "../../CheckedCast.h"
#include "../VKCore.h"


namespace LLGL
{


VKRenderTarget::VKRenderTarget(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc) :
    framebuffer_ { device, vkDestroyFramebuffer }
{
    CreateFramebuffer(device, desc);
}


/*
 * ======= Private: =======
 */

void VKRenderTarget::CreateFramebuffer(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc)
{
    #if 0
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = ;
        createInfo.attachmentCount  = ;
        createInfo.pAttachments     = ;
        createInfo.width            = ;
        createInfo.height           = ;
        createInfo.layers           = ;
    }
    VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan framebuffer");
    #endif
}


} // /namespace LLGL



// ================================================================================
