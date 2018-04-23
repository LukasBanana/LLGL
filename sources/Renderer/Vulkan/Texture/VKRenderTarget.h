/*
 * VKRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_TARGET_H
#define LLGL_VK_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class VKRenderTarget : public RenderTarget
{

    public:

        VKRenderTarget(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc);

    private:

        void CreateFramebuffer(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc);

        VKPtr<VkFramebuffer> framebuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
