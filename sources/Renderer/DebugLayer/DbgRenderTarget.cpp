/*
 * DbgRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderTarget.h"
#include "DbgTexture.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc) :
    instance { instance },
    desc_    { desc     }
{
}

Extent2D DbgRenderTarget::GetResolution() const
{
    return instance.GetResolution();
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


} // /namespace LLGL



// ================================================================================
