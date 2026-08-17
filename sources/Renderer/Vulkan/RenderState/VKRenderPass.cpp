/*
 * VKRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKRenderPass.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../Ext/VKExtensionRegistry.h"
#include "../Ext/VKExtensions.h"
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

static void InitColorVkAttachmentDesc(
    VkAttachmentDescription&    dst,
    Format                      format,
    AttachmentLoadOp            loadOp,
    AttachmentStoreOp           storeOp,
    VkImageLayout               finalLayout,
    VkSampleCountFlagBits       sampleCountBits)
{
    dst.flags           = 0;
    dst.format          = VKTypes::Map(format);
    dst.samples         = sampleCountBits;
    dst.loadOp          = VKTypes::Map(loadOp);
    dst.storeOp         = VKTypes::Map(storeOp);
    dst.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    dst.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    dst.initialLayout   = (loadOp == AttachmentLoadOp::Load ? finalLayout : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = finalLayout;
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

static void InitDepthStencilVkAttachmentDesc(
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
    // Depth/stencil attachments never transition to PRESENT_SRC_KHR — that layout is for color
    // images being presented via VK_KHR_swapchain. Using it here also requires VK_KHR_swapchain to
    // be enabled on the device, and is incompatible with the depth-stencil-attachment usage flag
    // even when the extension is enabled.
    const bool loadingExistingContent = (srcDepth.loadOp == AttachmentLoadOp::Load || srcStencil.loadOp == AttachmentLoadOp::Load);
    dst.initialLayout   = (loadingExistingContent ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED);
    dst.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void VKRenderPass::CreateVkRenderPass(VkDevice device, const RenderPassDescriptor& desc)
{
    /* Get number of attachments */
    const std::uint32_t numColorAttachments = NumEnabledColorAttachments(desc);
    std::uint32_t       numAttachments      = numColorAttachments;

    constexpr std::uint32_t maxNumClearValues = static_cast<std::uint32_t>(std::numeric_limits<decltype(numClearValues_)>::max());
    LLGL_ASSERT(numAttachments <= maxNumClearValues, "too many attachments for Vulkan render pass");

    /* Check for depth-stencil attachment */
    const bool hasDepthStencil = (desc.depthAttachment.format != Format::Undefined || desc.stencilAttachment.format != Format::Undefined);
    if (hasDepthStencil)
        ++numAttachments;

    /* Initialize attachment descriptors */
    const VkSampleCountFlagBits sampleCountBits = VKTypes::ToVkSampleCountBits(desc.samples);
    VkAttachmentDescription attachmentDescs[LLGL_MAX_NUM_ATTACHMENTS + LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    if (sampleCountBits > VK_SAMPLE_COUNT_1_BIT)
    {
        /* Initialize color attachment Vulkan descriptors */
        for_range(i, numColorAttachments)
        {
            InitColorVkAttachmentDesc(
                attachmentDescs[i],
                desc.colorAttachments[i].format,
                desc.colorAttachments[i].loadOp,
                desc.colorAttachments[i].storeOp,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                sampleCountBits
            );
        }

        /* Initialize resolve attachment Vulkan descriptors */
        for_range(i, numColorAttachments)
        {
            InitColorVkAttachmentDesc(
                attachmentDescs[numAttachments + i],
                desc.colorAttachments[i].format,
                AttachmentLoadOp::Undefined, // Don't load since resolve will be overridden everytime
                desc.colorAttachments[i].storeOp,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_SAMPLE_COUNT_1_BIT
            );
        }
    }
    else
    {
        /* Initialize color attachment Vulkan descriptors */
        for_range(i, numColorAttachments)
        {
            InitColorVkAttachmentDesc(
                attachmentDescs[i],
                desc.colorAttachments[i].format,
                desc.colorAttachments[i].loadOp,
                desc.colorAttachments[i].storeOp,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                sampleCountBits
            );
        }
    }

    if (hasDepthStencil)
    {
        /* Initialize depth-stencil attachment Vulkan descriptor */
        InitDepthStencilVkAttachmentDesc(
            attachmentDescs[numColorAttachments],
            desc.depthAttachment,
            desc.stencilAttachment,
            sampleCountBits
        );
    }

    /* Create render pass with native attachment descriptors */
    CreateVkRenderPassWithDescriptors(device, numAttachments, numColorAttachments, attachmentDescs, sampleCountBits, desc.views);
}

#if VK_KHR_create_renderpass2 && VK_KHR_depth_stencil_resolve

static VkAttachmentDescription2KHR ToVkAttachmentDescription2(const VkAttachmentDescription& src)
{
    VkAttachmentDescription2KHR dst = {};
    {
        dst.sType           = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        dst.pNext           = nullptr;
        dst.flags           = src.flags;
        dst.format          = src.format;
        dst.samples         = src.samples;
        dst.loadOp          = src.loadOp;
        dst.storeOp         = src.storeOp;
        dst.stencilLoadOp   = src.stencilLoadOp;
        dst.stencilStoreOp  = src.stencilStoreOp;
        dst.initialLayout   = src.initialLayout;
        dst.finalLayout     = src.finalLayout;
    }
    return dst;
}

static VkAttachmentReference2KHR ToVkAttachmentReference2(const VkAttachmentReference& src)
{
    VkAttachmentReference2KHR dst = {};
    {
        dst.sType       = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        dst.pNext       = nullptr;
        dst.attachment  = src.attachment;
        dst.layout      = src.layout;
        dst.aspectMask  = 0; // Only read for input attachments, of which there are none
    }
    return dst;
}

/*
Creates the render pass through VK_KHR_create_renderpass2, which is the only entry point that accepts a
depth-stencil resolve description (it chains into VkSubpassDescription2, not VkSubpassDescription).
Mirrors the version-1 path in CreateVkRenderPassWithDescriptors one-for-one, except that multiview moves from
a chained VkRenderPassMultiviewCreateInfo to VkSubpassDescription2::viewMask.
*/
static VkResult CreateVkRenderPass2WithDepthStencilResolve(
    VkDevice                        device,
    const VkAttachmentDescription*  attachmentDescs,
    std::uint32_t                   numAttachments,
    const VkAttachmentReference*    colorAttachmentsRefs,
    std::uint32_t                   numColorAttachments,
    const VkAttachmentReference*    resolveAttachmentsRefs,
    const VkAttachmentReference&    depthStencilAttachmentRef,
    const VkAttachmentReference&    depthStencilResolveAttachmentRef,
    const VkSubpassDependency&      subpassDep,
    std::uint32_t                   viewMask,
    VkRenderPass*                   outRenderPass)
{
    /* Convert the version-1 descriptors and references this class builds into their version-2 counterparts */
    VkAttachmentDescription2KHR attachmentDescs2[LLGL_MAX_NUM_ATTACHMENTS + LLGL_MAX_NUM_COLOR_ATTACHMENTS + 1];
    for_range(i, numAttachments)
        attachmentDescs2[i] = ToVkAttachmentDescription2(attachmentDescs[i]);

    VkAttachmentReference2KHR colorAttachmentsRefs2[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkAttachmentReference2KHR resolveAttachmentsRefs2[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    for_range(i, numColorAttachments)
    {
        colorAttachmentsRefs2[i] = ToVkAttachmentReference2(colorAttachmentsRefs[i]);
        if (resolveAttachmentsRefs != nullptr)
            resolveAttachmentsRefs2[i] = ToVkAttachmentReference2(resolveAttachmentsRefs[i]);
    }

    const VkAttachmentReference2KHR depthStencilAttachmentRef2          = ToVkAttachmentReference2(depthStencilAttachmentRef);
    const VkAttachmentReference2KHR depthStencilResolveAttachmentRef2   = ToVkAttachmentReference2(depthStencilResolveAttachmentRef);

    /*
    Resolve by taking sample 0 rather than averaging: an averaged depth does not correspond to any surface that
    was rasterized, and a compositor reprojecting from it would warp against geometry that never existed.
    SAMPLE_ZERO is also the only mode the specification requires every implementation to support.

    Both aspects deliberately use the same mode. Resolving one aspect but not the other requires
    VkPhysicalDeviceDepthStencilResolveProperties::independentResolveNone, whereas identical modes are always
    permitted; the stencil mode is simply ignored when the format carries no stencil aspect.
    */
    VkSubpassDescriptionDepthStencilResolveKHR depthStencilResolve = {};
    {
        depthStencilResolve.sType                           = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
        depthStencilResolve.pNext                           = nullptr;
        depthStencilResolve.depthResolveMode                = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
        depthStencilResolve.stencilResolveMode              = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
        depthStencilResolve.pDepthStencilResolveAttachment  = (&depthStencilResolveAttachmentRef2);
    }

    VkSubpassDescription2KHR subpassDesc2 = {};
    {
        subpassDesc2.sType                      = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
        subpassDesc2.pNext                      = (&depthStencilResolve);
        subpassDesc2.flags                      = 0;
        subpassDesc2.pipelineBindPoint          = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc2.viewMask                   = viewMask;
        subpassDesc2.inputAttachmentCount       = 0;
        subpassDesc2.pInputAttachments          = nullptr;
        subpassDesc2.colorAttachmentCount       = numColorAttachments;
        subpassDesc2.pColorAttachments          = colorAttachmentsRefs2;
        subpassDesc2.pResolveAttachments        = (resolveAttachmentsRefs != nullptr && numColorAttachments > 0 ? resolveAttachmentsRefs2 : nullptr);
        subpassDesc2.pDepthStencilAttachment    = (&depthStencilAttachmentRef2);
        subpassDesc2.preserveAttachmentCount    = 0;
        subpassDesc2.pPreserveAttachments       = nullptr;
    }

    VkSubpassDependency2KHR subpassDep2 = {};
    {
        subpassDep2.sType           = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        subpassDep2.pNext           = nullptr;
        subpassDep2.srcSubpass      = subpassDep.srcSubpass;
        subpassDep2.dstSubpass      = subpassDep.dstSubpass;
        subpassDep2.srcStageMask    = subpassDep.srcStageMask;
        subpassDep2.dstStageMask    = subpassDep.dstStageMask;
        subpassDep2.srcAccessMask   = subpassDep.srcAccessMask;
        subpassDep2.dstAccessMask   = subpassDep.dstAccessMask;
        subpassDep2.dependencyFlags = subpassDep.dependencyFlags;
        subpassDep2.viewOffset      = 0; // All views depend on the same source view; matches the version-1 path
    }

    VkRenderPassCreateInfo2KHR createInfo2 = {};
    {
        createInfo2.sType                   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
        createInfo2.pNext                   = nullptr;
        createInfo2.flags                   = 0;
        createInfo2.attachmentCount         = numAttachments;
        createInfo2.pAttachments            = attachmentDescs2;
        createInfo2.subpassCount            = 1;
        createInfo2.pSubpasses              = (&subpassDesc2);
        createInfo2.dependencyCount         = 1;
        createInfo2.pDependencies           = (&subpassDep2);
        createInfo2.correlatedViewMaskCount = (viewMask != 0 ? 1u : 0u);
        createInfo2.pCorrelatedViewMasks    = (viewMask != 0 ? (&viewMask) : nullptr);
    }
    return vkCreateRenderPass2KHR(device, &createInfo2, nullptr, outRenderPass);
}

#endif // /VK_KHR_create_renderpass2 && VK_KHR_depth_stencil_resolve

void VKRenderPass::CreateVkRenderPassWithDescriptors(
    VkDevice                        device,
    std::uint32_t                   numAttachments,
    std::uint32_t                   numColorAttachments,
    const VkAttachmentDescription*  attachmentDescs,
    VkSampleCountFlagBits           sampleCountBits,
    std::uint32_t                   numViews,
    bool                            hasDepthStencilResolve)
{
    LLGL_ASSERT(numAttachments <= LLGL_MAX_NUM_ATTACHMENTS);
    LLGL_ASSERT(numColorAttachments <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);

    /* Uninitialized stack memory for descriptor containers */
    VkAttachmentReference colorAttachmentsRefs[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkAttachmentReference resolveAttachmentsRefs[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    VkAttachmentReference depthStencilAttachmentRef;
    VkAttachmentReference depthStencilResolveAttachmentRef;

    SmallVector<VkAttachmentDescription, LLGL_MAX_NUM_ATTACHMENTS * 2> sanitizedAttachmentDescs;

    /* Store sample count bits and number of color attachments (required for default blend states in VKGraphicsPipeline) */
    sampleCountBits_        = sampleCountBits;
    numColorAttachments_    = static_cast<std::uint8_t>(numColorAttachments);
    numViews_               = (numViews > 1 ? numViews : 1);

    /* Build bitmask for clear values: least significant bit (LSB) is used for the first attachment */
    clearValuesMask_ = 0;

    for_range(i, numAttachments)
    {
        if (attachmentDescs[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValuesMask_ |= (0x1ull << i);
            numClearValues_ = std::max(numClearValues_, static_cast<std::uint8_t>(i + 1));
        }
        sanitizedAttachmentDescs.push_back(attachmentDescs[i]);
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
    std::uint32_t resolveAttachmentIndex = numAttachments;

    if (hasMultiSampling)
    {
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
                sanitizedAttachmentDescs.push_back(attachmentDescs[numAttachments + i]);
            }
        }
    }

    /*
    A multi-sampled depth-stencil attachment cannot be resolved through the sub-pass' resolve list, which is
    color-only. It needs VkSubpassDescriptionDepthStencilResolve, and that only chains into a version-2 sub-pass
    description -- hence the separate creation path below.
    */
    const bool resolvesDepthStencil = (hasDepthStencilResolve && hasMultiSampling && hasDepthStencil);
    if (resolvesDepthStencil)
    {
        depthStencilResolveAttachmentRef.attachment = resolveAttachmentIndex++;
        depthStencilResolveAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        sanitizedAttachmentDescs.push_back(attachmentDescs[numAttachments + numColorAttachments]);
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

    /*
    For multiview (single-pass layered) rendering, broadcast the single subpass to 'numViews_' views via a
    view mask. The correlation mask hints that all views are rendered from correlated geometry, which lets
    drivers optimize. This requires VK_KHR_multiview (core in Vulkan 1.1).
    */
    const std::uint32_t viewMask = (numViews_ > 1 ? ((1u << numViews_) - 1u) : 0u);
    if (numViews_ > 1 && !HasExtension(VKExt::KHR_multiview))
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("A Vulkan render pass with multiple views was requested but the multiview extension is not supported");

    /*
    Take the version-2 entry point when the depth-stencil attachment is resolved; it is the only one that accepts
    the resolve description. Everything else keeps using the version-1 entry point, so the widely-travelled path
    is untouched. Note the two versions express multiview differently: version 1 chains
    VkRenderPassMultiviewCreateInfo, version 2 carries the view mask on the sub-pass description itself.
    */
    if (resolvesDepthStencil)
    {
        #if VK_KHR_create_renderpass2 && VK_KHR_depth_stencil_resolve
        if (HasExtension(VKExt::KHR_create_renderpass2) && HasExtension(VKExt::KHR_depth_stencil_resolve))
        {
            VkResult result = CreateVkRenderPass2WithDepthStencilResolve(
                device,
                sanitizedAttachmentDescs.data(),
                static_cast<std::uint32_t>(sanitizedAttachmentDescs.size()),
                colorAttachmentsRefs,
                numColorAttachments,
                (numColorAttachments > 0 ? resolveAttachmentsRefs : nullptr),
                depthStencilAttachmentRef,
                depthStencilResolveAttachmentRef,
                subpassDep,
                viewMask,
                renderPass_.ReleaseAndGetAddressOf()
            );
            VKThrowIfFailed(result, "failed to create Vulkan render pass with depth-stencil resolve");
            return;
        }
        #endif // /VK_KHR_create_renderpass2 && VK_KHR_depth_stencil_resolve
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("VK_KHR_depth_stencil_resolve");
    }

    const void* createInfoNext = nullptr;
    #if VK_KHR_multiview
    VkRenderPassMultiviewCreateInfoKHR multiviewCreateInfo = {};
    if (numViews_ > 1)
    {
        multiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
        multiviewCreateInfo.pNext = nullptr;
        multiviewCreateInfo.subpassCount = 1;
        multiviewCreateInfo.pViewMasks = (&viewMask);
        multiviewCreateInfo.dependencyCount = 0;
        multiviewCreateInfo.pViewOffsets = nullptr;
        multiviewCreateInfo.correlationMaskCount = 1;
        multiviewCreateInfo.pCorrelationMasks = (&viewMask);
        createInfoNext = (&multiviewCreateInfo);
    }
    #endif // /VK_KHR_multiview

    /* Create swap-chain render pass */
    VkRenderPassCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext            = createInfoNext;
        createInfo.flags            = 0;
        createInfo.attachmentCount  = static_cast<std::uint32_t>(sanitizedAttachmentDescs.size());
        createInfo.pAttachments     = sanitizedAttachmentDescs.data();
        createInfo.subpassCount     = 1;
        createInfo.pSubpasses       = (&subpassDesc);
        createInfo.dependencyCount  = 1;
        createInfo.pDependencies    = (&subpassDep);
    }
    VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, renderPass_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan render pass");
}


} // /namespace LLGL



// ================================================================================
