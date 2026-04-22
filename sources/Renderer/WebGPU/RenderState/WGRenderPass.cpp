/*
 * WGRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderPass.h"
#include "../WGTypes.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


static Format PickDepthStencilFormat(const RenderPassDescriptor& desc)
{
    if (desc.depthAttachment.format != Format::Undefined)
        return desc.depthAttachment.format;
    if (desc.stencilAttachment.format != Format::Undefined)
        return desc.stencilAttachment.format;
    return Format::Undefined;
}

WGRenderPass::WGRenderPass(const RenderPassDescriptor& desc) :
    depthStencilFormat_ { WGTypes::ToWGTextureFormat(PickDepthStencilFormat(desc)) }
{
    //TODO
}


} // /namespace LLGL



// ================================================================================
