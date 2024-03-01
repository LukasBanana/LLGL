/*
 * DbgRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgRenderTarget.h"
#include "DbgTexture.h"
#include "../DbgCore.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static void ConvertAttachmentFormatDesc(AttachmentFormatDescriptor& dst, const AttachmentDescriptor& src)
{
    dst.format  = GetAttachmentFormat(src);
    dst.loadOp  = AttachmentLoadOp::Load;
    dst.storeOp = AttachmentStoreOp::Store;
}

static void ConvertRenderPassAttachmentDesc(RenderPassDescriptor& dst, const AttachmentDescriptor& src, std::uint32_t colorAttachmentIndex = 0)
{
    const Format format = GetAttachmentFormat(src);
    if (IsColorFormat(format))
    {
        LLGL_ASSERT(colorAttachmentIndex < LLGL_MAX_NUM_COLOR_ATTACHMENTS);
        ConvertAttachmentFormatDesc(dst.colorAttachments[colorAttachmentIndex], src);
    }
    else if (IsDepthAndStencilFormat(format))
    {
        ConvertAttachmentFormatDesc(dst.depthAttachment, src);
        ConvertAttachmentFormatDesc(dst.stencilAttachment, src);
    }
    else if (IsDepthFormat(format))
        ConvertAttachmentFormatDesc(dst.depthAttachment, src);
    else if (IsStencilFormat(format))
        ConvertAttachmentFormatDesc(dst.stencilAttachment, src);
}

static void ConvertRenderPassDesc(RenderPassDescriptor& dst, const RenderTargetDescriptor& src)
{
    for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        ConvertRenderPassAttachmentDesc(dst, src.colorAttachments[i], i);
    ConvertRenderPassAttachmentDesc(dst, src.depthStencilAttachment);
    dst.samples = src.samples;
}

static RenderPassDescriptor MakeRenderPassDesc(const RenderTargetDescriptor& renderTargetDesc)
{
    RenderPassDescriptor renderPassDesc;
    ConvertRenderPassDesc(renderPassDesc, renderTargetDesc);
    return renderPassDesc;
}

DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, const RenderTargetDescriptor& desc) :
    instance { instance             },
    desc     { desc                 },
    label    { LLGL_DBG_LABEL(desc) }
{
    if (const RenderPass* renderPass = instance.GetRenderPass())
        renderPass_ = MakeUnique<DbgRenderPass>(*renderPass, MakeRenderPassDesc(desc));
}

void DbgRenderTarget::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

Extent2D DbgRenderTarget::GetResolution() const
{
    return instance.GetResolution();
}

std::uint32_t DbgRenderTarget::GetSamples() const
{
    return instance.GetSamples();
}

std::uint32_t DbgRenderTarget::GetNumColorAttachments() const
{
    return instance.GetNumColorAttachments();
}

bool DbgRenderTarget::HasDepthAttachment() const
{
    return instance.HasDepthAttachment();
}

bool DbgRenderTarget::HasStencilAttachment() const
{
    return instance.HasStencilAttachment();
}

const RenderPass* DbgRenderTarget::GetRenderPass() const
{
    return renderPass_.get();
}


} // /namespace LLGL



// ================================================================================
