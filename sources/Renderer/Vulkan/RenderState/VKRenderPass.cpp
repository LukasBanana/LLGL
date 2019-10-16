/*
 * VKRenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderPass.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include <LLGL/RenderPassFlags.h>
#include <limits>


namespace LLGL
{


VKRenderPass::VKRenderPass(const VKPtr<VkDevice>& device) :
    renderPass_ { device, vkDestroyRenderPass }
{
}

VKRenderPass::VKRenderPass(const VKPtr<VkDevice>& device, const RenderPassDescriptor& desc) :
    VKRenderPass { device }
{
    CreateVkRenderPass(device, desc);
}

static void Convert(
    VkAttachmentDescription&            dst,
    const AttachmentFormatDescriptor&   src,
    VkSampleCountFlagBits               sampleCountBits)
{
    dst.flags           = 0;
    dst.format          = VKTypes::Map(src.format);
    dst.samples         = sampleCountBits;
    dst.loadOp          = VKTypes::Map(src.loadOp);
    dst.storeOp         = VKTypes::Map(src.storeOp);
    dst.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    dst.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    dst.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    dst.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

static VkFormat GetDepthStencilFormat(const Format depthFormat, const Format& stencilFormat)
{
    if (depthFormat != Format::Undefined && stencilFormat != Format::Undefined)
    {
        /* Check whether depth and stencil attachments share the same format */
        if (depthFormat != stencilFormat)
            throw std::invalid_argument("format mismatch between depth and stencil render pass attachments");
        return VKTypes::Map(depthFormat);
    }

    if (depthFormat != Format::Undefined)
    {
        /* Get depth-stencil format from depth attachment only */
        return VKTypes::Map(depthFormat);
    }

    if (stencilFormat != Format::Undefined)
    {
        /* Get depth-stencil format from stencil attachment only */
        return VKTypes::Map(stencilFormat);
    }

    return VK_FORMAT_UNDEFINED;
}

static void Convert(
    VkAttachmentDescription&            dst,
    const AttachmentFormatDescriptor&   srcDepth,
    const AttachmentFormatDescriptor&   srcStencil,
    VkSampleCountFlagBits               sampleCountBits)
{
    dst.flags           = 0;
    dst.format          = GetDepthStencilFormat(srcDepth.format, srcStencil.format);
    dst.samples         = sampleCountBits;
    dst.loadOp          = VKTypes::Map(srcDepth.loadOp);
    dst.storeOp         = VKTypes::Map(srcDepth.storeOp);
    dst.stencilLoadOp   = VKTypes::Map(srcStencil.loadOp);
    dst.stencilStoreOp  = VKTypes::Map(srcStencil.storeOp);
    dst.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    dst.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void VKRenderPass::CreateVkRenderPass(VkDevice device, const RenderPassDescriptor& desc)
{
    /* Get number of attachments */
    std::uint32_t numColorAttachments   = static_cast<std::uint32_t>(desc.colorAttachments.size());
    std::uint32_t numAttachments        = numColorAttachments;

    if (numAttachments > static_cast<std::uint32_t>(std::numeric_limits<decltype(numClearValues_)>::max()))
        throw std::invalid_argument("too many attachments for Vulkan render pass");

    /* Check for depth-stencil attachment */
    bool hasDepthStencil = false;

    if (desc.depthAttachment.format != Format::Undefined || desc.stencilAttachment.format != Format::Undefined)
    {
        ++numAttachments;
        hasDepthStencil = true;
    }

    /* Initialize attachment descriptors */
    const auto sampleCountBits = VKTypes::ToVkSampleCountBits(desc.samples);
    std::vector<VkAttachmentDescription> attachmentDescs(desc.samples > 1 ? numAttachments + numColorAttachments : numAttachments);

    for (std::uint32_t i = 0; i < numColorAttachments; ++i)
        Convert(attachmentDescs[i], desc.colorAttachments[i], VK_SAMPLE_COUNT_1_BIT);

    if (hasDepthStencil)
        Convert(attachmentDescs[numColorAttachments], desc.depthAttachment, desc.stencilAttachment, sampleCountBits);

    if (sampleCountBits > VK_SAMPLE_COUNT_1_BIT)
    {
        /* Take color attachment format descriptors for multi-sampled attachemnts */
        for (std::uint32_t i = 0; i < numColorAttachments; ++i)
            Convert(attachmentDescs[numAttachments + i], desc.colorAttachments[i], sampleCountBits);

        /* Modify original attachment descriptors */
        for (std::uint32_t i = 0; i < numColorAttachments; ++i)
            attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    /* Create render pass with native attachment descriptors */
    CreateVkRenderPassWithDescriptors(
        device,
        numAttachments,
        numColorAttachments,
        attachmentDescs.data(),
        sampleCountBits
    );
}

void VKRenderPass::CreateVkRenderPassWithDescriptors(
    VkDevice                        device,
    std::uint32_t                   numAttachments,
    std::uint32_t                   numColorAttachments,
    const VkAttachmentDescription*  attachmentDescs,
    VkSampleCountFlagBits           sampleCountBits)
{
    /* Store sample count bits */
    sampleCountBits_ = sampleCountBits;
    const bool multiSampleEnabled = (sampleCountBits > VK_SAMPLE_COUNT_1_BIT);

    std::vector<VkAttachmentReference> rtvAttachmentsRefs(numAttachments);
    std::vector<VkAttachmentReference> rtvMsaaAttachmentsRefs;
    VkAttachmentReference dsvAttachmentRef = {};

    /* Store index for depth-stencil attachment */
    bool hasDepthStencil = (numColorAttachments < numAttachments);
    if (hasDepthStencil)
        depthStencilIndex_ = static_cast<std::uint8_t>(numColorAttachments);

    /* Store number of color attachments (required for default blend states in VKGraphicsPipeline) */
    numColorAttachments_ = static_cast<std::uint8_t>(numColorAttachments);

    /* Build bitmask for clear values: least significant bit (LSB) is used for the first attachment */
    clearValuesMask_ = 0;

    for (std::uint32_t i = 0; i < numAttachments; ++i)
    {
        if (attachmentDescs[multiSampleEnabled && i + 1 < numAttachments ? numAttachments + i : i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValuesMask_ |= (0x1ull << i);
            numClearValues_ = std::max(numClearValues_, static_cast<std::uint8_t>(i + 1));
        }
    }

    /* Initialize attachment reference */
    for (std::uint32_t i = 0; i < numColorAttachments; ++i)
    {
        rtvAttachmentsRefs[i].attachment    = i;
        rtvAttachmentsRefs[i].layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if (hasDepthStencil)
    {
        dsvAttachmentRef.attachment = depthStencilIndex_;
        dsvAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (multiSampleEnabled)
    {
        rtvMsaaAttachmentsRefs.resize(numColorAttachments);
        for (std::uint32_t i = 0; i < numColorAttachments; ++i)
        {
            rtvMsaaAttachmentsRefs[i].attachment    = numAttachments + i;
            rtvMsaaAttachmentsRefs[i].layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    /* Initialize sub-pass descriptor */
    VkSubpassDescription subpassDesc;
    {
        subpassDesc.flags                   = 0;
        subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.inputAttachmentCount    = 0;
        subpassDesc.pInputAttachments       = nullptr;
        subpassDesc.colorAttachmentCount    = numColorAttachments;
        if (rtvMsaaAttachmentsRefs.empty())
        {
            subpassDesc.pColorAttachments   = rtvAttachmentsRefs.data();
            subpassDesc.pResolveAttachments = nullptr;
        }
        else
        {
            /* Swap color and resolve attachments */
            subpassDesc.pColorAttachments   = rtvMsaaAttachmentsRefs.data();
            subpassDesc.pResolveAttachments = rtvAttachmentsRefs.data();
        }
        subpassDesc.pDepthStencilAttachment = (hasDepthStencil ? &dsvAttachmentRef : nullptr);
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments    = nullptr;
    }

    /* Initialize sub-pass dependency */
    VkSubpassDependency subpassDep;
    {
        subpassDep.srcSubpass               = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass               = 0;
        subpassDep.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
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
        createInfo.attachmentCount          = (multiSampleEnabled ? numAttachments + numColorAttachments : numAttachments);
        createInfo.pAttachments             = attachmentDescs;
        createInfo.subpassCount             = 1;
        createInfo.pSubpasses               = (&subpassDesc);
        createInfo.dependencyCount          = 1;
        createInfo.pDependencies            = (&subpassDep);
    }
    auto result = vkCreateRenderPass(device, &createInfo, nullptr, renderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan render pass");
}


} // /namespace LLGL



// ================================================================================
