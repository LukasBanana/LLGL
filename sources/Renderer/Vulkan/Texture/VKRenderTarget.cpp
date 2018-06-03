/*
 * VKRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderTarget.h"
#include "VKTexture.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../../CheckedCast.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <vector>
#include <algorithm>


namespace LLGL
{


VKRenderTarget::VKRenderTarget(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc) :
    framebuffer_        { device, vkDestroyFramebuffer },
    renderPass_         { device, vkDestroyRenderPass  },
    depthStencilBuffer_ { device                       }
{
    CreateRenderPass(device, deviceMemoryMngr, desc);
    CreateFramebuffer(device, desc);
}

void VKRenderTarget::ReleaseDeviceMemoryResources(VKDeviceMemoryManager& deviceMemoryMngr)
{
    depthStencilBuffer_.ReleaseDepthStencil(deviceMemoryMngr);
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void ErrDepthAttachmentFailed()
{
    throw std::runtime_error("attachment to render target failed, because render target already has a depth-stencil buffer");
}

static VkFormat GetDepthAttachmentVkFormat(const AttachmentType type)
{
    switch (type)
    {
        case AttachmentType::Color:         throw std::invalid_argument("invalid color attachment to render target that has no texture");
        case AttachmentType::Depth:         return VK_FORMAT_D32_SFLOAT;
        case AttachmentType::DepthStencil:  return VK_FORMAT_D24_UNORM_S8_UINT;
        case AttachmentType::Stencil:       return VK_FORMAT_S8_UINT;
    }
    throw std::invalid_argument("unknown attachment type to render target that has no texture");
}

static bool HasStencilComponent(const AttachmentType type)
{
    return (type == AttachmentType::Stencil || type == AttachmentType::DepthStencil);
}

void VKRenderTarget::CreateRenderPass(const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const RenderTargetDescriptor& desc)
{
    /* Initialize attachment descriptors */
    std::vector<VkAttachmentDescription> attachmentDescs(desc.attachments.size());

    for (std::size_t i = 0; i < desc.attachments.size(); ++i)
    {
        const auto& attachmentSrc = desc.attachments[i];
        auto&       attachmentDst = attachmentDescs[i];

        /* Initialize format and sample count flags */
        VkFormat                format          = VK_FORMAT_UNDEFINED;
        VkSampleCountFlagBits   samplesFlags    = VK_SAMPLE_COUNT_1_BIT; //TODO: multi-sampling

        if (auto texture = attachmentSrc.texture)
        {
            /* Get format from texture */
            auto textureVK = LLGL_CAST(VKTexture*, texture);
            format = textureVK->GetVkFormat();
        }
        else
        {
            /* Validate attachment attributes */
            if (attachmentSrc.resolution.width == 0 || attachmentSrc.resolution.height == 0)
                throw std::invalid_argument("invalid attachment to render target that has no texture and no valid size specified");

            format = GetDepthAttachmentVkFormat(attachmentSrc.type);

            /* Create depth-stencil buffer */
            if (depthStencilBuffer_.GetVkFormat() == VK_FORMAT_UNDEFINED)
                depthStencilBuffer_.CreateDepthStencil(deviceMemoryMngr, attachmentSrc.resolution, format, samplesFlags);
            else
                ErrDepthAttachmentFailed();
        }

        /* Write attachment descriptor */
        attachmentDst.flags            = 0;
        attachmentDst.format           = format;
        attachmentDst.samples          = samplesFlags;
        attachmentDst.loadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDst.storeOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDst.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDst.stencilStoreOp    = (HasStencilComponent(attachmentSrc.type) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE);
        attachmentDst.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDst.finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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
                attachment.mipLevel,
                1,
                attachment.layer,
                1,
                imageViews_[numAttachments].ReleaseAndGetAddressOf()
            );

            /* Add image view to attachments */
            imageViewRefs[numAttachments] = imageViews_[numAttachments].Get();

            /* Apply texture resolution to render target (to validate correlation between attachments) */
            ApplyResolution({ textureVK->GetVkExtent().width, textureVK->GetVkExtent().height });

            /* Next attachment index */
            ++numAttachments;
        }
        else
        {
            /* Add depth-stencil image view to attachments */
            imageViewRefs[numAttachments] = depthStencilBuffer_.GetVkImageView();

            /* Apply texture resolution to render target (to validate correlation between attachments) */
            ApplyResolution(attachment.resolution);

            /* Next attachment index */
            ++numAttachments;
        }
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
        createInfo.width            = GetResolution().width;
        createInfo.height           = GetResolution().height;
        createInfo.layers           = 1;
    }
    VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan framebuffer");
}


} // /namespace LLGL



// ================================================================================
