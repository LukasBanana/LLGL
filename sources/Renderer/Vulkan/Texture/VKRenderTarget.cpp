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
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <vector>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>


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
    numColorAttachments_ { NumActiveColorAttachments(desc)            },
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
    return IsDepthFormat(depthStencilFormat_);
}

bool VKRenderTarget::HasStencilAttachment() const
{
    return IsStencilFormat(depthStencilFormat_);
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

static VkImageLayout GetFinalLayoutForAttachment(VkFormat format, long bindFlags)
{
    if ((bindFlags & BindFlags::Sampled) != 0)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else
        return (VKTypes::IsVkFormatDepthStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

static void InitVkAttachmentDesc(
    VkAttachmentDescription&    outDesc,
    VkFormat                    format,
    long                        bindFlags,
    VkSampleCountFlagBits       sampleCountBits,
    VkAttachmentLoadOp          loadOp)
{
    outDesc.flags               = 0;
    outDesc.format              = format;
    outDesc.samples             = sampleCountBits;
    outDesc.loadOp              = loadOp;
    outDesc.storeOp             = VK_ATTACHMENT_STORE_OP_STORE;
    if (VKTypes::IsVkFormatStencil(format))
    {
        outDesc.stencilLoadOp   = loadOp;
        outDesc.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_STORE;
    }
    else
    {
        outDesc.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        outDesc.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    outDesc.initialLayout       = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? GetFinalLayoutForAttachment(format, bindFlags) : VK_IMAGE_LAYOUT_UNDEFINED);
    outDesc.finalLayout         = GetFinalLayoutForAttachment(format, bindFlags);
}

static VkFormat GetDepthStencilVkFormat(const Format format)
{
    if (IsDepthOrStencilFormat(format))
        return VKTypes::Map(format);
    else if (IsColorFormat(format))
        LLGL_TRAP("invalid color attachment to render target that has no texture");
    else
        LLGL_TRAP("unknown attachment type to render target that has no texture");
}

void VKRenderTarget::CreateRenderPass(
    VkDevice                        device,
    const RenderTargetDescriptor&   desc,
    VKRenderPass&                   renderPass,
    VkAttachmentLoadOp              attachmentsLoadOp)
{
    /* Uninitialized stack memory for attachment descriptors */
    VkAttachmentDescription attachmentDescs[LLGL_MAX_NUM_ATTACHMENTS + LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    /* Initialize attachment descriptors */
    const bool          hasDepthStencil         = IsAttachmentEnabled(desc.depthStencilAttachment);
    const std::uint32_t numTargetAttachments    = (hasDepthStencil ? numColorAttachments_ + 1 : numColorAttachments_);

    for_range(i, numColorAttachments_)
    {
        /* Write Vulkan descriptor for color attachment */
        const AttachmentDescriptor& colorAttachment = desc.colorAttachments[i];
        VkFormat format = VKTypes::Map(GetAttachmentFormat(colorAttachment));
        long bindFlags = (colorAttachment.texture != nullptr ? colorAttachment.texture->GetBindFlags() : 0);
        InitVkAttachmentDesc(attachmentDescs[i], format, bindFlags, sampleCountBits_, attachmentsLoadOp);
    }

    if (hasDepthStencil)
    {
        /* Write Vulkan descriptor for depth-stencil attachment */
        const AttachmentDescriptor& depthStencilAttachment = desc.depthStencilAttachment;
        VkFormat format = GetDepthStencilVkFormat(GetAttachmentFormat(depthStencilAttachment));
        long bindFlags = (depthStencilAttachment.texture != nullptr ? depthStencilAttachment.texture->GetBindFlags() : 0);
        InitVkAttachmentDesc(attachmentDescs[numColorAttachments_], format, bindFlags, sampleCountBits_, attachmentsLoadOp);
    }

    /* Initialize attachment descriptors for multi-sampled color attachments */
    if (HasMultiSampling())
    {
        /* Take color attachment format descriptors for multi-sampled attachemnts */
        for_range(i, numColorAttachments_)
        {
            const AttachmentDescriptor& resolveAttachment = desc.resolveAttachments[i];
            constexpr long bindFlags = 0;
            if (resolveAttachment.texture != nullptr)
            {
                VkFormat format = VKTypes::Map(GetAttachmentFormat(resolveAttachment));
                InitVkAttachmentDesc(attachmentDescs[numTargetAttachments + i], format, bindFlags, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
            }
            else
            {
                /* Defualt initialize descriptor to disable this resolve attachment */
                InitVkAttachmentDesc(attachmentDescs[numTargetAttachments + i], VK_FORMAT_UNDEFINED, bindFlags, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
            }
        }
    }

    /* Create native Vulkan render pass with attachment descriptors */
    renderPass.CreateVkRenderPassWithDescriptors(device, numTargetAttachments, numColorAttachments_, attachmentDescs, sampleCountBits_);
}

void VKRenderTarget::CreateDefaultRenderPass(VkDevice device, const RenderTargetDescriptor& desc)
{
    CreateRenderPass(device, desc, defaultRenderPass_, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
}

void VKRenderTarget::CreateSecondaryRenderPass(VkDevice device, const RenderTargetDescriptor& desc)
{
    CreateRenderPass(device, desc, secondaryRenderPass_, VK_ATTACHMENT_LOAD_OP_LOAD);
}

VkImageView VKRenderTarget::CreateAttachmentImageView(
    VkDevice                    device,
    VKTexture&                  textureVK,
    Format                      format,
    const AttachmentDescriptor& attachmentDesc)
{
    /* Validate texture resolution to render target (to validate correlation between attachments) */
    ValidateMipResolution(textureVK, attachmentDesc.mipLevel);

    /* Create new image view for MIP-level and array layer specified in attachment descriptor */
    VKPtr<VkImageView> imageView{ device, vkDestroyImageView };
    {
        textureVK.CreateImageView(device, TextureSubresource{ attachmentDesc.arrayLayer, attachmentDesc.mipLevel }, format, imageView);
    }
    imageViews_.emplace_back(std::move(imageView));

    return imageViews_.back().Get();
}

VkImageView VKRenderTarget::CreateColorBuffer(VKDeviceMemoryManager& deviceMemoryMngr, Format format)
{
    /* Create new color buffer with sampling information */
    auto colorBuffer = MakeUnique<VKColorBuffer>(deviceMemoryMngr.GetVkDevice());
    {
        colorBuffer->Create(deviceMemoryMngr, GetResolution(), VKTypes::Map(format), sampleCountBits_);
    }
    colorBuffers_.push_back(std::move(colorBuffer));

    return colorBuffers_.back()->GetVkImageView();
}

VkImageView VKRenderTarget::CreateDepthStencilBuffer(VKDeviceMemoryManager& deviceMemoryMngr, Format format)
{
    /* Create depth-stencil buffer */
    depthStencilBuffer_.Create(deviceMemoryMngr, GetResolution(), GetDepthStencilVkFormat(format), sampleCountBits_);

    /* Add depth-stencil image view to attachments */
    return depthStencilBuffer_.GetVkImageView();
}

void VKRenderTarget::CreateFramebuffer(
    VkDevice                        device,
    VKDeviceMemoryManager&          deviceMemoryMngr,
    const RenderTargetDescriptor&   desc)
{
    /* Create image view for each attachment */
    const bool          hasDepthStencil         = IsAttachmentEnabled(desc.depthStencilAttachment);
    const std::uint32_t numTargetAttachments    = (hasDepthStencil ? numColorAttachments_ + 1 : numColorAttachments_);
    const std::uint32_t numResolveAttachments   = NumActiveResolveAttachments(desc);

    imageViews_.reserve(numTargetAttachments + numResolveAttachments);

    VkImageView attachmentImageViews[LLGL_MAX_NUM_COLOR_ATTACHMENTS * 2 + 1];

    for_range(i, numColorAttachments_)
    {
        const AttachmentDescriptor& colorAttachment = desc.colorAttachments[i];
        if (Texture* texture = colorAttachment.texture)
        {
            /* Use attachment texture for color buffer view */
            auto& textureVK = LLGL_CAST(VKTexture&, *texture);
            const Format colorFormat = GetAttachmentFormat(colorAttachment);
            attachmentImageViews[i] = CreateAttachmentImageView(device, textureVK, colorFormat, colorAttachment);
        }
        else
        {
            /* Create internal color buffer */
            attachmentImageViews[i] = CreateColorBuffer(deviceMemoryMngr, colorAttachment.format);
        }
    }

    /* Create depth-stencil attachment */
    if (hasDepthStencil)
    {
        const AttachmentDescriptor& depthStencilAttachment = desc.depthStencilAttachment;
        depthStencilFormat_ = GetAttachmentFormat(depthStencilAttachment);
        if (Texture* texture = depthStencilAttachment.texture)
        {
            /* Use attachment texture for depth-stencil view */
            auto& textureVK = LLGL_CAST(VKTexture&, *texture);
            attachmentImageViews[numColorAttachments_] = CreateAttachmentImageView(device, textureVK, depthStencilFormat_, depthStencilAttachment);
        }
        else
        {
            /* Create internal depth-stencil buffer */
            attachmentImageViews[numColorAttachments_] = CreateDepthStencilBuffer(deviceMemoryMngr, depthStencilFormat_);
        }
    }

    /* Create resolve color buffer views */
    std::uint32_t attachmentCount = numTargetAttachments;

    if (HasMultiSampling())
    {
        for_range(i, numColorAttachments_)
        {
            const AttachmentDescriptor& resolveAttachment = desc.resolveAttachments[i];
            if (Texture* texture = resolveAttachment.texture)
            {
                /* Use attachment texture for color buffer view */
                auto& textureVK = LLGL_CAST(VKTexture&, *texture);
                const Format colorFormat = GetAttachmentFormat(resolveAttachment);
                attachmentImageViews[attachmentCount++] = CreateAttachmentImageView(device, textureVK, colorFormat, resolveAttachment);
            }
        }
    }

    /* Create framebuffer object */
    const Extent2D resolution = GetResolution();
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = (attachmentCount == 0 ? VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT : 0);
        createInfo.renderPass       = renderPass_->GetVkRenderPass();
        createInfo.attachmentCount  = attachmentCount;
        createInfo.pAttachments     = attachmentImageViews;
        createInfo.width            = resolution.width;
        createInfo.height           = resolution.height;
        createInfo.layers           = 1;
    }
    VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan framebuffer");
}


} // /namespace LLGL



// ================================================================================
