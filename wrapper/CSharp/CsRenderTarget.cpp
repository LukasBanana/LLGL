/*
 * CsRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderTarget.h"


namespace SharpLLGL
{


RenderTarget::RenderTarget(LLGL::RenderTarget* native) :
    native_ { native }
{
}

bool RenderTarget::IsRenderContext::get()
{
    return native_->IsRenderContext();
}

Extent2D^ RenderTarget::Resolution::get()
{
    auto extent = native_->GetResolution();
    return gcnew Extent2D(extent.width, extent.height);
}

unsigned int RenderTarget::NumColorAttachments::get()
{
    return native_->GetNumColorAttachments();
}

bool RenderTarget::HasDepthAttachment::get()
{
    return native_->HasDepthAttachment();
}

bool RenderTarget::HasStencilAttachment::get()
{
    return native_->HasStencilAttachment();
}

RenderPass^ RenderTarget::RenderPass::get()
{
    return renderPass_;
}


/*
 * ======= Internal: =======
 */

LLGL::RenderTarget* RenderTarget::Native::get()
{
    return native_;
}


} // /namespace SharpLLGL



// ================================================================================
