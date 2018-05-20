/*
 * VKRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_TARGET_H
#define LLGL_VK_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include "VKDepthStencilBuffer.h"


namespace LLGL
{


class VKRenderTarget : public RenderTarget
{

    public:

        VKRenderTarget(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc);

        void ReleaseDeviceMemoryResources(VKDeviceMemoryManager& deviceMemoryMngr);

        inline VkFramebuffer GetVkFramebuffer() const
        {
            return framebuffer_;
        }

        inline VkRenderPass GetVkRenderPass() const
        {
            return renderPass_;
        }

        inline VkExtent2D GetVkExtent() const
        {
            return { GetResolution().width, GetResolution().height };
        }

        inline std::uint32_t GetNumColorAttachments() const
        {
            return numColorAttachments_;
        }

        inline bool HasDepthStencilAttachment() const
        {
            return hasDepthStencilAttachment_;
        }

    private:

        void CreateRenderPass(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc);
        void CreateFramebuffer(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc);

        VKPtr<VkFramebuffer>            framebuffer_;
        VKPtr<VkRenderPass>             renderPass_;

        std::vector<VKPtr<VkImageView>> imageViews_;

        VKDepthStencilBuffer            depthStencilBuffer_;

        std::uint32_t                   numColorAttachments_        = 0;
        bool                            hasDepthStencilAttachment_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
