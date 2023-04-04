/*
 * VKRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKRenderTarget.h"
#include "VKTexture.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <vector>
#include <algorithm>
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


VKRenderTarget::VKRenderTarget(
    VkDevice                        device,
    VKDeviceMemoryManager&          deviceMemoryMngr,
    const RenderTargetDescriptor&   desc)
:
    resolution_          { desc.resolution                            },
    framebuffer_         { device, vkDestroyFramebuffer               },
    defaultRenderPass_   { device                                     },
    secondaryRenderPass_ { device                                     },
    depthStencilBuffer_  { device                                     },
    sampleCountBits_     { VKTypes::ToVkSampleCountBits(desc.samples) }
{
    if (desc.renderPass)
    {
        /* Get render pass from descriptor */
        renderPass_ = LLGL_CAST(const VKRenderPass*, desc.renderPass);
    }
    else
    {
        /* Create default render pass */
        CreateDefaultRenderPass(device, desc);
        renderPass_ = (&defaultRenderPass_);
    }
    CreateSecondaryRenderPass(device, desc);
    CreateFramebuffer(device, deviceMemoryMngr, desc);
}

Extent2D VKRenderTarget::GetResolution() const
{
    return resolution_;
}

std::uint32_t VKRenderTarget::GetSamples() const
{
    return static_cast<std::uint32_t>(sampleCountBits_);
}

std::uint32_t VKRenderTarget::GetNumColorAttachments() const
{
    return numColorAttachments_;
}

bool VKRenderTarget::HasDepthAttachment() const
{
    return (depthStencilFormat_ == VK_FORMAT_D32_SFLOAT || depthStencilFormat_ == VK_FORMAT_D24_UNORM_S8_UINT);
}

bool VKRenderTarget::HasStencilAttachment() const
{
    return (depthStencilFormat_ == VK_FORMAT_S8_UINT || depthStencilFormat_ == VK_FORMAT_D24_UNORM_S8_UINT);
}

const RenderPass* VKRenderTarget::GetRenderPass() const
{
    return renderPass_;
}

bool VKRenderTarget::HasMultiSampling() const
{
    return (sampleCountBits_ > VK_SAMPLE_COUNT_1_BIT);
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void ErrDepthAttachmentFailed()
{
    throw std::runtime_error("depth-stencil already allocated in render target: more than one attachment does not have a texture reference");
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

void VKRenderTarget::CreateDepthStencilForAttachment(VKDeviceMemoryManager& deviceMemoryMngr, const AttachmentDescriptor& attachmentDesc)
{
    /* Create depth-stencil buffer */
    if (depthStencilBuffer_.GetVkFormat() == VK_FORMAT_UNDEFINED)
    {
        depthStencilBuffer_.Create(
            deviceMemoryMngr,
            GetResolution(),
            GetDepthAttachmentVkFormat(attachmentDesc.type),
            sampleCountBits_
        );
    }
    else
        ErrDepthAttachmentFailed();
}

static bool IsVkFormatDepthStencil(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

static VkImageLayout GetFinalLayoutForAttachment(VkFormat format, long bindFlags)
{
    if ((bindFlags & BindFlags::Sampled) != 0)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else
        return (IsVkFormatDepthStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

static void SetVkAttachmentDesc(
    VkAttachmentDescription&    dst,
    const AttachmentDescriptor& src,
    VkFormat                    format,
    long                        bindFlags,
    VkSampleCountFlagBits       sampleCountBits,
    VkAttachmentLoadOp          loadOp)
{
    dst.flags           = 0;
    dst.format          = format;
    dst.samples         = sampleCountBits;
    dst.loadOp          = loadOp;
    dst.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    dst.stencilLoadOp   = (HasStencilComponent(src.type) ? loadOp : VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    dst.stencilStoreOp  = (HasStencilComponent(src.type) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE);
    dst.initialLayout   = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? GetFinalLayoutForAttachment(format, bindFlags) : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = GetFinalLayoutForAttachment(format, bindFlags);
}

static void SetVkAttachmentDescForMsaaColorOnly(
    VkAttachmentDescription&    dst,
    VkFormat                    format,
    VkSampleCountFlagBits       sampleCountBits,
    VkAttachmentLoadOp          loadOp)
{
    dst.flags           = 0;
    dst.format          = format;
    dst.samples         = sampleCountBits;
    dst.loadOp          = loadOp;
    dst.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    dst.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    dst.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    dst.initialLayout   = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void VKRenderTarget::CreateRenderPass(
    VkDevice                        device,
    const RenderTargetDescriptor&   desc,
    VKRenderPass&                   renderPass,
    VkAttachmentLoadOp              attachmentsLoadOp)
{
    /* Initialize attachment descriptors */
    std::uint32_t numAttachments        = static_cast<std::uint32_t>(desc.attachments.size());
    std::uint32_t numColorAttachments   = 0;

    VkAttachmentDescription attachmentDescs[LLGL_MAX_NUM_ATTACHMENTS + LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkFormat colorFormats[LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    for (const auto& attachment : desc.attachments)
    {
        if (auto texture = attachment.texture)
        {
            /* Get format from texture */
            auto textureVK = LLGL_CAST(VKTexture*, texture);

            /* Write attachment descriptor */
            SetVkAttachmentDesc(
                attachmentDescs[numColorAttachments],
                attachment,
                textureVK->GetVkFormat(),
                textureVK->GetBindFlags(),
                VK_SAMPLE_COUNT_1_BIT, // target texture always has 1 sample only
                attachmentsLoadOp
            );

            if (attachment.type == AttachmentType::Color)
                colorFormats[numColorAttachments++] = textureVK->GetVkFormat();
        }
        else
        {
            /* Write attachment descriptor */
            SetVkAttachmentDesc(
                attachmentDescs[numAttachments - 1],
                attachment,
                GetDepthAttachmentVkFormat(attachment.type),
                BindFlags::DepthStencilAttachment,
                sampleCountBits_,
                attachmentsLoadOp
            );
        }
    }

    /* Initialize attachment descriptors for multi-sampled color attachments */
    if (HasMultiSampling())
    {
        /* Take color attachment format descriptors for multi-sampled attachemnts */
        for_range(i, numColorAttachments)
        {
            SetVkAttachmentDescForMsaaColorOnly(
                attachmentDescs[numAttachments + i],
                colorFormats[i],
                sampleCountBits_,
                attachmentsLoadOp
            );
        }

        /* Modify original attachment descriptors */
        for_range(i, numColorAttachments)
            attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    /* Create native Vulkan render pass with attachment descriptors */
    renderPass.CreateVkRenderPassWithDescriptors(device, numAttachments, numColorAttachments, attachmentDescs, sampleCountBits_);
}

void VKRenderTarget::CreateDefaultRenderPass(VkDevice device, const RenderTargetDescriptor& desc)
{
    CreateRenderPass(device, desc, defaultRenderPass_, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
}

void VKRenderTarget::CreateSecondaryRenderPass(VkDevice device, const RenderTargetDescriptor& desc)
{
    CreateRenderPass(device, desc, secondaryRenderPass_, VK_ATTACHMENT_LOAD_OP_LOAD);
}

void VKRenderTarget::CreateFramebuffer(
    VkDevice                        device,
    VKDeviceMemoryManager&          deviceMemoryMngr,
    const RenderTargetDescriptor&   desc)
{
    depthStencilFormat_     = VK_FORMAT_UNDEFINED;
    numColorAttachments_    = 0;

    /* Create image view for each attachment */
    std::uint32_t numAttachments = static_cast<std::uint32_t>(desc.attachments.size());

    if (numAttachments == 0)
        throw std::runtime_error("cannot create render target without attachments");

    imageViews_.reserve(numAttachments);
    std::vector<VkImageView> imageViewRefs(numAttachments);
    std::vector<VkFormat> colorFormats(numAttachments);

    for (const auto& attachment : desc.attachments)
    {
        if (auto texture = attachment.texture)
        {
            auto textureVK = LLGL_CAST(VKTexture*, texture);

            /* Create new image view for MIP-level and array layer specified in attachment descriptor */
            VKPtr<VkImageView> imageView{ device, vkDestroyImageView };
            {
                textureVK->CreateImageView(device, attachment.mipLevel, /*numMips:*/ 1, attachment.arrayLayer, /*numLayers:*/ 1, imageView);

                /* Add image view to attachments */
                if (attachment.type == AttachmentType::Color)
                {
                    /* Next color attachment index */
                    imageViewRefs[numColorAttachments_] = imageView;
                    colorFormats[numColorAttachments_] = textureVK->GetVkFormat();
                    ++numColorAttachments_;
                }
                else
                {
                    /* Store depth-stencil format */
                    imageViewRefs[numAttachments - 1] = imageView;
                    depthStencilFormat_ = textureVK->GetVkFormat();
                }
            }
            imageViews_.emplace_back(std::move(imageView));

            /* Validate texture resolution to render target (to validate correlation between attachments) */
            ValidateMipResolution(*textureVK, attachment.mipLevel);
        }
        else
        {
            /* Create depth-stencil buffer */
            CreateDepthStencilForAttachment(deviceMemoryMngr, attachment);

            /* Add depth-stencil image view to attachments */
            imageViewRefs[numAttachments - 1] = depthStencilBuffer_.GetVkImageView();

            /* Store depth-stencil format */
            depthStencilFormat_ = depthStencilBuffer_.GetVkFormat();
        }
    }

    /* Create multi-sample color buffers */
    if (HasMultiSampling())
    {
        colorBuffers_.reserve(numColorAttachments_);
        imageViewRefs.reserve(numAttachments + numColorAttachments_);

        for_range(i, numColorAttachments_)
        {
            /* Create new multi-sampled color buffer and store reference to image view in primary attachment container */
            auto colorBuffer = MakeUnique<VKColorBuffer>(device);
            {
                colorBuffer->Create(deviceMemoryMngr, GetResolution(), colorFormats[i], sampleCountBits_);
                imageViewRefs.push_back(colorBuffer->GetVkImageView());
            }
            colorBuffers_.push_back(std::move(colorBuffer));
        }
    }

    /* Create framebuffer object */
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.renderPass       = renderPass_->GetVkRenderPass();
        createInfo.attachmentCount  = (HasMultiSampling() ? numAttachments + numColorAttachments_ : numAttachments);
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
