/*
 * VKRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_TARGET_H
#define LLGL_VK_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include "../RenderState/VKRenderPass.h"
#include "VKDepthStencilBuffer.h"


namespace LLGL
{


class VKRenderTarget final : public RenderTarget
{

    public:

        VKRenderTarget(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc);

        Extent2D GetResolution() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

        /* ----- Extended functions ----- */

        void ReleaseDeviceMemoryResources(VKDeviceMemoryManager& deviceMemoryMngr);

        // Returns the Vulkan framebuffer object.
        inline VkFramebuffer GetVkFramebuffer() const
        {
            return framebuffer_;
        }

        // Returns the Vulkan render pass object.
        inline VkRenderPass GetVkRenderPass() const
        {
            return renderPass_->GetVkRenderPass();
        }

        // Returns the secondary Vulkan render pass object.
        inline VkRenderPass GetSecondaryVkRenderPass() const
        {
            return secondaryRenderPass_.GetVkRenderPass();
        }

        // Returns the render target resolution as VkExtent2D.
        inline VkExtent2D GetVkExtent() const
        {
            return { resolution_.width, resolution_.height };
        }

    private:

        void CreateDepthStencilForAttachment(VKDeviceMemoryManager& deviceMemoryMngr, const AttachmentDescriptor& attachmentDesc);
        void CreateDefaultRenderPass(VkDevice device, const RenderTargetDescriptor& desc);
        void CreateSecondaryRenderPass(VkDevice device, const RenderTargetDescriptor& desc);
        void CreateFramebuffer(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc);

        VkSampleCountFlagBits GetSampleCountFlags() const;

    private:

        Extent2D                        resolution_;

        VKPtr<VkFramebuffer>            framebuffer_;
        const VKRenderPass*             renderPass_             = nullptr;
        VKRenderPass                    defaultRenderPass_;
        VKRenderPass                    secondaryRenderPass_;

        std::vector<VKPtr<VkImageView>> imageViews_;

        VKDepthStencilBuffer            depthStencilBuffer_;
        VkFormat                        depthStencilFormat_     = VK_FORMAT_UNDEFINED;  // Format either from internal depth-stencil buffer or attachmed texture.

        std::uint32_t                   numColorAttachments_    = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
