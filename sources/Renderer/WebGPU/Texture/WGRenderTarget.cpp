/*
 * WGRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderTarget.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


/* WGRenderTarget::WGRenderTarget(const RenderTargetDescriptor& desc) { ... } */

Extent2D WGRenderTarget::GetResolution() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGRenderTarget::GetSamples() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGRenderTarget::GetNumColorAttachments() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGRenderTarget::HasDepthAttachment() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGRenderTarget::HasStencilAttachment() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

const RenderPass* WGRenderTarget::GetRenderPass() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
