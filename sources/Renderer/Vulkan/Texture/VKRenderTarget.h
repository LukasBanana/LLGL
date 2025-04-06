/*
 * VKRenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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


class VKTexture;
class VKCommandContext;

class VKRenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        VKRenderTarget(
            VkDevice                        device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const RenderTargetDescriptor&   desc
        );

    public:

        // Returns true if this render target has multi-sampling enabled.
        bool HasMultiSampling() const;

        // Transitions the image layouts for all texture attachments that are specified in this render-target's render-pass.
        void OverrideImageLayoutsForRenderPass();

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

        void CreateRenderPass(
            VkDevice                        device,
            const RenderTargetDescriptor&   desc,
            VKRenderPass&                   renderPass,
            VkAttachmentLoadOp              attachmentsLoadOp
        );

        void CreateDefaultRenderPass(VkDevice device, const RenderTargetDescriptor& desc);
        void CreateSecondaryRenderPass(VkDevice device, const RenderTargetDescriptor& desc);

        VkImageView CreateAttachmentImageView(
            VkDevice                    device,
            VKTexture*                  textureVK,
            Format                      format,
            const AttachmentDescriptor& attachmentDesc
        );

        VkImageView CreateColorBuffer(VKDeviceMemoryManager& deviceMemoryMngr, Format format);
        VkImageView CreateDepthStencilBuffer(VKDeviceMemoryManager& deviceMemoryMngr, Format format);

        void CreateFramebuffer(
            VkDevice                        device,
            VKDeviceMemoryManager&          deviceMemoryMngr,
            const RenderTargetDescriptor&   desc
        );

    private:

        struct AttachmentView
        {
            inline AttachmentView(VkDevice device) :
                imageView { device, vkDestroyImageView }
            {
            }

            inline AttachmentView(AttachmentView&& rhs) noexcept :
                texture   { rhs.texture              },
                layout    { rhs.layout               },
                imageView { std::move(rhs.imageView) }
            {
            }

            inline AttachmentView(VKTexture* texture, VkImageLayout layout, VKPtr<VkImageView>&& imageView) :
                texture   { texture              },
                layout    { layout               },
                imageView { std::move(imageView) }
            {
            }

            AttachmentView(const AttachmentView&) = delete;
            AttachmentView& operator = (const AttachmentView&) = delete;

            VKTexture*          texture     = nullptr;
            VkImageLayout       layout      = VK_IMAGE_LAYOUT_UNDEFINED;
            VKPtr<VkImageView>  imageView;
        };

    private:

        using VKColorBufferPtr = std::unique_ptr<VKColorBuffer>;

        Extent2D                        resolution_;

        VKPtr<VkFramebuffer>            framebuffer_;
        const VKRenderPass*             renderPass_             = nullptr;
        VKRenderPass                    defaultRenderPass_;
        VKRenderPass                    secondaryRenderPass_;

        vector<AttachmentView>          attachmentViews_;

        VKDepthStencilBuffer            depthStencilBuffer_;
        Format                          depthStencilFormat_     = Format::Undefined;    // Format either from internal depth-stencil buffer or attachmed texture.
        vector<VKColorBufferPtr>        colorBuffers_;                                  // Internal color buffers for multi-sampling

        std::uint32_t                   numColorAttachments_    = 0;
        VkSampleCountFlagBits           sampleCountBits_        = VK_SAMPLE_COUNT_1_BIT;

};


} // /namespace LLGL


#endif



// ================================================================================
