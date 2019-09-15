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
#include "VKColorBuffer.h"
#include <memory>


namespace LLGL
{


class VKRenderTarget final : public RenderTarget
{

    public:

        VKRenderTarget(
            const VKPtr<VkDevice>&          device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const RenderTargetDescriptor&   desc
        );

        Extent2D GetResolution() const override;
        std::uint32_t GetSamples() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        // Returns true if this render context has multi-sampling enabled.
        bool HasMultiSampling() const;

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

        void CreateRenderPass(
            VkDevice                        device,
            const RenderTargetDescriptor&   desc,
            VKRenderPass&                   renderPass,
            bool                            loadContent
        );

        void CreateDefaultRenderPass(VkDevice device, const RenderTargetDescriptor& desc);
        void CreateSecondaryRenderPass(VkDevice device, const RenderTargetDescriptor& desc);

        void CreateFramebuffer(
            const VKPtr<VkDevice>&          device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const RenderTargetDescriptor&   desc
        );

    private:

        using VKColorBufferPtr = std::unique_ptr<VKColorBuffer>;

        Extent2D                        resolution_;

        VKPtr<VkFramebuffer>            framebuffer_;
        const VKRenderPass*             renderPass_             = nullptr;
        VKRenderPass                    defaultRenderPass_;
        VKRenderPass                    secondaryRenderPass_;

        std::vector<VKPtr<VkImageView>> imageViews_;

        VKDepthStencilBuffer            depthStencilBuffer_;
        VkFormat                        depthStencilFormat_     = VK_FORMAT_UNDEFINED;  // Format either from internal depth-stencil buffer or attachmed texture.
        std::vector<VKColorBufferPtr>   colorBuffers_;                                  // Internal color buffers for multi-sampling

        std::uint32_t                   numColorAttachments_    = 0;
        VkSampleCountFlagBits           sampleCountBits_        = VK_SAMPLE_COUNT_1_BIT;

};


} // /namespace LLGL


#endif



// ================================================================================
