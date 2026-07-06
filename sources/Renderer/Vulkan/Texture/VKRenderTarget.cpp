/*
 * VKRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKRenderTarget.h"
#include "VKTexture.h"
#include "../Command/VKCommandContext.h"
#include "../Ext/VKExtensionRegistry.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
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
        /* Get render pass from descriptor; multiview view count comes from that render pass's view mask */
        renderPass_ = LLGL_CAST(const VKRenderPass*, desc.renderPass);
        numViews_ = renderPass_->GetNumViews();
    }
    else
    {
        /*
        Create a default render pass. Its view count comes from RenderTargetDescriptor::views, so a multiview
        render target can be created without an explicit render pass; the default pass derives its per-attachment
        final layouts from each attachment texture's bind flags (e.g. color attachments end in
        COLOR_ATTACHMENT_OPTIMAL), which is what offscreen/XR images need -- unlike a swap-chain-style render pass.
        */
        numViews_ = (desc.views > 1 ? desc.views : 1);
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

void VKRenderTarget::OverrideImageLayoutsForRenderPass()
{
    for (const AttachmentView& attachmentView : attachmentViews_)
        attachmentView.texture->OverrideVkImageLayout(attachmentView.layout);
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
        /* Take color attachment format descriptors for multi-sampled attachments */
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
                /* Default initialize descriptor to disable this resolve attachment */
                InitVkAttachmentDesc(attachmentDescs[numTargetAttachments + i], VK_FORMAT_UNDEFINED, bindFlags, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
            }
        }
    }

    /* Create native Vulkan render pass with attachment descriptors (multiview view count must match the framebuffer image views) */
    renderPass.CreateVkRenderPassWithDescriptors(device, numTargetAttachments, numColorAttachments_, attachmentDescs, sampleCountBits_, numViews_);
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
    VKTexture*                  textureVK,
    Format                      format,
    const AttachmentDescriptor& attachmentDesc)
{
    /* Validate texture resolution to render target (to validate correlation between attachments) */
    ValidateMipResolution(*textureVK, attachmentDesc.mipLevel);

    /*
    For multiview rendering, the attachment view spans 'numViews_' consecutive array layers starting at the
    attachment's base layer; the render pass view mask then routes each view to one layer. This requires the
    attachment texture to be an array texture (so its image view type is 2D_ARRAY) with enough layers.
    For non-multiview rendering (numViews_ == 1) this is a single-layer view, identical to before.
    */
    const std::uint32_t numLayers = numViews_;
    if (numLayers > 1)
    {
        LLGL_ASSERT(
            attachmentDesc.arrayLayer + numLayers <= textureVK->GetNumArrayLayers(),
            "multiview render target requires an array texture with at least %u layers, but attachment provides %u",
            attachmentDesc.arrayLayer + numLayers, textureVK->GetNumArrayLayers()
        );
    }

    /* Create new image view for MIP-level and array layer range specified in attachment descriptor */
    const VkImageLayout renderPassImageLayout = (IsDepthOrStencilFormat(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VKPtr<VkImageView> imageView{ device, vkDestroyImageView };
    {
        textureVK->CreateImageView(device, TextureSubresource{ attachmentDesc.arrayLayer, numLayers, attachmentDesc.mipLevel, 1u }, format, imageView);
    }
    attachmentViews_.emplace_back(textureVK, renderPassImageLayout, std::move(imageView));

    return attachmentViews_.back().imageView.Get();
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

    attachmentViews_.reserve(numTargetAttachments + numResolveAttachments);

    VkImageView attachmentImageViews[LLGL_MAX_NUM_COLOR_ATTACHMENTS * 2 + 1];

    for_range(i, numColorAttachments_)
    {
        const AttachmentDescriptor& colorAttachment = desc.colorAttachments[i];
        if (Texture* texture = colorAttachment.texture)
        {
            /* Use attachment texture for color buffer view */
            auto* textureVK = LLGL_CAST(VKTexture*, texture);
            const Format colorFormat = GetAttachmentFormat(colorAttachment);
            attachmentImageViews[i] = CreateAttachmentImageView(device, textureVK, colorFormat, colorAttachment);
        }
        else
        {
            /* Internal (anonymous) color buffers are single-layer and cannot be used for multiview rendering */
            LLGL_ASSERT(numViews_ == 1, "multiview render target requires a texture for each color attachment");
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
            auto* textureVK = LLGL_CAST(VKTexture*, texture);
            attachmentImageViews[numColorAttachments_] = CreateAttachmentImageView(device, textureVK, depthStencilFormat_, depthStencilAttachment);
        }
        else
        {
            /* Internal (anonymous) depth-stencil buffers are single-layer and cannot be used for multiview rendering */
            LLGL_ASSERT(numViews_ == 1, "multiview render target requires a texture for the depth-stencil attachment");
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
                auto* textureVK = LLGL_CAST(VKTexture*, texture);
                const Format colorFormat = GetAttachmentFormat(resolveAttachment);
                attachmentImageViews[attachmentCount++] = CreateAttachmentImageView(device, textureVK, colorFormat, resolveAttachment);
            }
        }
    }

    #if VK_KHR_imageless_framebuffer

    /* Create meta-data for render-targets with no attachments */
    VkFramebufferAttachmentsCreateInfoKHR attachmentsCreateInfo = {};
    if (HasExtension(VKExt::KHR_imageless_framebuffer))
    {
        attachmentsCreateInfo.sType                     = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR;
        attachmentsCreateInfo.pNext                     = nullptr;
        attachmentsCreateInfo.attachmentImageInfoCount  = 0;
        attachmentsCreateInfo.pAttachmentImageInfos     = nullptr;
    }

    #endif // /VK_KHR_imageless_framebuffer

    /* Create framebuffer object */
    const Extent2D resolution = GetResolution();
    VkFramebufferCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        if (attachmentCount == 0)
        {
            #if VK_KHR_imageless_framebuffer
            if (HasExtension(VKExt::KHR_imageless_framebuffer))
            {
                createInfo.pNext = &attachmentsCreateInfo;
                createInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
            }
            else
            #endif // /VK_KHR_imageless_framebuffer
            {
                //TODO: create dummy image
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("VK_KHR_imageless_framebuffer");
            }
        }
        else
        {
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
        }

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
