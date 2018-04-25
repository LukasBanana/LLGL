/*
 * VKRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderTarget.h"
#include "VKTexture.h"
#include "../../CheckedCast.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <vector>
#include <algorithm>


namespace LLGL
{


VKRenderTarget::VKRenderTarget(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc) :
    framebuffer_ { device, vkDestroyFramebuffer },
    renderPass_  { device, vkDestroyRenderPass  }
{
    CreateRenderPass(device, desc);
    CreateFramebuffer(device, desc);
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void ErrMissingTextureRef()
{
    throw std::runtime_error("render target attachment without texture not supported yet for Vulkan renderer");
}

void VKRenderTarget::CreateRenderPass(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc)
{
    /* Initialize attachment descriptors */
    std::vector<VkAttachmentDescription> attachmentDescs(desc.attachments.size());

    for (std::size_t i = 0; i < desc.attachments.size(); ++i)
    {
        /* Get VkFormat from attachment texture */
        if (auto texture = desc.attachments[i].texture)
        {
            auto textureVK = LLGL_CAST(VKTexture*, texture);

            /* Write attachment descriptor */
            auto& attachmentDesc = attachmentDescs[i];
            {
                attachmentDesc.flags                = 0;
                attachmentDesc.format               = textureVK->GetFormat();
                attachmentDesc.samples              = VK_SAMPLE_COUNT_1_BIT; //!!!
                attachmentDesc.loadOp               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDesc.storeOp              = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDesc.stencilLoadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDesc.stencilStoreOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDesc.initialLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDesc.finalLayout          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
        }
        else
            ErrMissingTextureRef();
    }

    /* Initialize attachment reference */
    std::vector<VkAttachmentReference> attachmentsRefs(desc.attachments.size());

    numColorAttachments_ = 0;
    std::size_t depthAttachmentIndex = ~0;

    for (std::uint32_t i = 0, n = static_cast<std::uint32_t>(desc.attachments.size()); i < n; ++i)
    {
        if (desc.attachments[i].type == AttachmentType::Color)
        {
            auto& attachmentRef = attachmentsRefs[numColorAttachments_++];
            {
                attachmentRef.attachment    = i;
                attachmentRef.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
        else
        {
            if (!hasDepthStencilAttachment_)
            {
                auto& attachmentRef = attachmentsRefs.back();
                {
                    attachmentRef.attachment    = i;
                    attachmentRef.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                depthAttachmentIndex = (attachmentsRefs.size() - 1);
                hasDepthStencilAttachment_ = true;
            }
            else
                throw std::invalid_argument("cannot have more than one depth-stencil attachment for render target");
        }
    }

    /* Initialize sub-pass descriptor */
    VkSubpassDescription subpassDesc;
    {
        subpassDesc.flags                   = 0;
        subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount    = 0;
        subpassDesc.pInputAttachments       = nullptr;
        subpassDesc.colorAttachmentCount    = numColorAttachments_;
        subpassDesc.pColorAttachments       = attachmentsRefs.data();
        subpassDesc.pResolveAttachments     = nullptr;
        subpassDesc.pDepthStencilAttachment = (depthAttachmentIndex < attachmentsRefs.size() ? &(attachmentsRefs[depthAttachmentIndex]) : nullptr);
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments    = nullptr;
    }

    /* Initialize sub-pass dependency */
    VkSubpassDependency subpassDep;
    {
        subpassDep.srcSubpass               = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass               = 0;
        subpassDep.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.dstStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.srcAccessMask            = 0;
        subpassDep.dstAccessMask            = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDep.dependencyFlags          = 0;
    }

    /* Create swap-chain render pass */
    VkRenderPassCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.attachmentCount          = static_cast<std::uint32_t>(attachmentDescs.size());
        createInfo.pAttachments             = attachmentDescs.data();
        createInfo.subpassCount             = 1;
        createInfo.pSubpasses               = (&subpassDesc);
        createInfo.dependencyCount          = 1;
        createInfo.pDependencies            = (&subpassDep);
    }
    auto result = vkCreateRenderPass(device, &createInfo, nullptr, renderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan render pass");
}

void VKRenderTarget::CreateFramebuffer(const VKPtr<VkDevice>& device, const RenderTargetDescriptor& desc)
{
    /* Create image view for each attachment */
    imageViews_.resize(desc.attachments.size(), VKPtr<VkImageView> { device, vkDestroyImageView });

    std::vector<VkImageView> imageViewRefs(desc.attachments.size());

    std::uint32_t numAttachments = 0;

    for (const auto& attachment : desc.attachments)
    {
        if (attachment.texture)
        {
            auto textureVK = LLGL_CAST(VKTexture*, attachment.texture);

            /* Create new image view for MIP-level and array layer specified in attachment descriptor */
            textureVK->CreateImageView(
                device,
                attachment.layer,
                attachment.mipLevel,
                1,
                imageViews_[numAttachments].ReleaseAndGetAddressOf()
            );

            /* Add image view to attachments */
            imageViewRefs[numAttachments] = imageViews_[numAttachments].Get();

            /* Apply texture resolution to render target (to validate correlation between attachments) */
            ApplyResolution({ textureVK->GetExtent().width, textureVK->GetExtent().height});

            ++numAttachments;
        }
        #if 0//TODO: not supported yet
        else
        {
            if (attachment.width == 0 || attachment.height == 0)
                throw std::invalid_argument("invalid attachment to render target that has no texture and no valid size specified");
            if (attachment.type == AttachmentType::Color)
                throw std::invalid_argument("invalid color attachment to render target that has no texture");

            //TODO: create texture just for this render target ...
        }
        #else
        else
            ErrMissingTextureRef();
        #endif
    }

    if (numAttachments == 0)
        throw std::runtime_error("failed to create render target without attachments");

    /* Create framebuffer object */
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = renderPass_;
        createInfo.attachmentCount  = numAttachments;
        createInfo.pAttachments     = imageViewRefs.data();
        createInfo.width            = GetResolution().x;
        createInfo.height           = GetResolution().y;
        createInfo.layers           = 1;
    }
    VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan framebuffer");
}


} // /namespace LLGL



// ================================================================================
