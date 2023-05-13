/*
 * VKRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKRenderPass.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../../RenderPassUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <limits>


namespace LLGL
{


VKRenderPass::VKRenderPass(VkDevice device) :
    renderPass_ { device, vkDestroyRenderPass }
{
}

VKRenderPass::VKRenderPass(VkDevice device, const RenderPassDescriptor& desc) :
    VKRenderPass { device }
{
    CreateVkRenderPass(device, desc);
}

static void ConvertColorVkAttachmentDesc(
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
    dst.initialLayout   = (src.loadOp == AttachmentLoadOp::Load ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

static VkFormat GetDepthStencilFormat(const Format depthFormat, const Format& stencilFormat)
{
    if (depthFormat != Format::Undefined && stencilFormat != Format::Undefined)
    {
        /* Check whether depth and stencil attachments share the same format */
        if (depthFormat != stencilFormat)
            LLGL_TRAP("format mismatch between depth and stencil render pass attachments");
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

static void ConvertDepthStencilVkAttachmentDesc(
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
    dst.initialLayout   = (srcDepth.loadOp == AttachmentLoadOp::Load || srcStencil.loadOp == AttachmentLoadOp::Load ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void VKRenderPass::CreateVkRenderPass(VkDevice device, const RenderPassDescriptor& desc)
{
    /* Get number of attachments */
    const auto  numColorAttachments = NumEnabledColorAttachments(desc);
    auto        numAttachments      = numColorAttachments;

    constexpr auto maxNumClearValues = static_cast<std::uint32_t>(std::numeric_limits<decltype(numClearValues_)>::max());
    LLGL_ASSERT(numAttachments <= maxNumClearValues, "too many attachments for Vulkan render pass");

    /* Check for depth-stencil attachment */
    const bool hasDepthStencil = (desc.depthAttachment.format != Format::Undefined || desc.stencilAttachment.format != Format::Undefined);
    if (hasDepthStencil)
        ++numAttachments;

    /* Initialize attachment descriptors */
    const VkSampleCountFlagBits sampleCountBits = VKTypes::ToVkSampleCountBits(desc.samples);
    VkAttachmentDescription attachmentDescs[LLGL_MAX_NUM_ATTACHMENTS + LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    for_range(i, numColorAttachments)
        ConvertColorVkAttachmentDesc(attachmentDescs[i], desc.colorAttachments[i], sampleCountBits);

    if (hasDepthStencil)
        ConvertDepthStencilVkAttachmentDesc(attachmentDescs[numColorAttachments], desc.depthAttachment, desc.stencilAttachment, sampleCountBits);

    if (sampleCountBits > VK_SAMPLE_COUNT_1_BIT)
    {
        /* Take color attachment format descriptors for multi-sampled attachemnts */
        for_range(i, numColorAttachments)
            ConvertColorVkAttachmentDesc(attachmentDescs[numAttachments + i], desc.colorAttachments[i], VK_SAMPLE_COUNT_1_BIT);
    }

    /* Create render pass with native attachment descriptors */
    CreateVkRenderPassWithDescriptors(device, numAttachments, numColorAttachments, attachmentDescs, sampleCountBits);
}

void VKRenderPass::CreateVkRenderPassWithDescriptors(
    VkDevice                        device,
    std::uint32_t                   numAttachments,
    std::uint32_t                   numColorAttachments,
    const VkAttachmentDescription*  attachmentDescs,
    VkSampleCountFlagBits           sampleCountBits)
{
    LLGL_ASSERT(numAttachments <= LLGL_MAX_NUM_ATTACHMENTS);
    LLGL_ASSERT(numColorAttachments <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);

    /* Uninitialized stack memory for descriptor containers */
    VkAttachmentReference colorAttachmentsRefs[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkAttachmentReference resolveAttachmentsRefs[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkAttachmentReference depthStencilAttachmentRef;

    /* Store sample count bits and number of color attachments (required for default blend states in VKGraphicsPipeline) */
    sampleCountBits_        = sampleCountBits;
    numColorAttachments_    = static_cast<std::uint8_t>(numColorAttachments);

    /* Build bitmask for clear values: least significant bit (LSB) is used for the first attachment */
    clearValuesMask_ = 0;

    for_range(i, numAttachments)
    {
        if (attachmentDescs[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValuesMask_ |= (0x1ull << i);
            numClearValues_ = std::max(numClearValues_, static_cast<std::uint8_t>(i + 1));
        }
    }

    /* Initialize attachment reference */
    for_range(i, numColorAttachments)
    {
        colorAttachmentsRefs[i].attachment  = i;
        colorAttachmentsRefs[i].layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    const bool hasDepthStencil = (numColorAttachments < numAttachments);
    if (hasDepthStencil)
    {
        depthStencilIndex_ = static_cast<std::uint8_t>(numColorAttachments);
        depthStencilAttachmentRef.attachment    = depthStencilIndex_;
        depthStencilAttachmentRef.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    const bool hasMultiSampling = (sampleCountBits > VK_SAMPLE_COUNT_1_BIT);
    if (hasMultiSampling)
    {
        std::uint32_t resolveAttachmentIndex = numAttachments;
        for_range(i, numColorAttachments)
        {
            if (attachmentDescs[numAttachments + i].format == VK_FORMAT_UNDEFINED)
            {
                resolveAttachmentsRefs[i].attachment    = VK_ATTACHMENT_UNUSED;
                resolveAttachmentsRefs[i].layout        = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            else
            {
                resolveAttachmentsRefs[i].attachment    = resolveAttachmentIndex++;
                resolveAttachmentsRefs[i].layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
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
        subpassDesc.pColorAttachments       = colorAttachmentsRefs;
        subpassDesc.pResolveAttachments     = (hasMultiSampling && numColorAttachments > 0 ? resolveAttachmentsRefs : nullptr);
        subpassDesc.pDepthStencilAttachment = (hasDepthStencil ? &depthStencilAttachmentRef : nullptr);
        subpassDesc.preserveAttachmentCount = 0;
        subpassDesc.pPreserveAttachments    = nullptr;
    }

    /* Initialize sub-pass dependency */
    VkSubpassDependency subpassDep;
    {
        subpassDep.srcSubpass       = VK_SUBPASS_EXTERNAL;
        subpassDep.dstSubpass       = 0;
        subpassDep.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
        subpassDep.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDep.srcAccessMask    = 0;
        subpassDep.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDep.dependencyFlags  = 0;
    }

    /* Create swap-chain render pass */
    VkRenderPassCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.attachmentCount  = (hasMultiSampling ? numAttachments + numColorAttachments : numAttachments);
        createInfo.pAttachments     = attachmentDescs;
        createInfo.subpassCount     = 1;
        createInfo.pSubpasses       = (&subpassDesc);
        createInfo.dependencyCount  = 1;
        createInfo.pDependencies    = (&subpassDep);
    }
    auto result = vkCreateRenderPass(device, &createInfo, nullptr, renderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan render pass");
}


} // /namespace LLGL



// ================================================================================
