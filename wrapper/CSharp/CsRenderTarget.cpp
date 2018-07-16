/*
 * CsRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderTarget.h"


namespace LHermanns
{

namespace LLGL
{


RenderTarget::RenderTarget(::LLGL::RenderTarget* native) :
    native_ { native }
{
}

::LLGL::RenderTarget* RenderTarget::Native::get()
{
    return native_;
}

bool RenderTarget::IsRenderContext()
{
    return native_->IsRenderContext();
}

Extent2D^ RenderTarget::GetResolution()
{
    auto extent = native_->GetResolution();
    return gcnew Extent2D(extent.width, extent.height);
}

unsigned int RenderTarget::GetNumColorAttachments()
{
    return native_->GetNumColorAttachments();
}

bool RenderTarget::HasDepthAttachment()
{
    return native_->HasDepthAttachment();
}

bool RenderTarget::HasStencilAttachment()
{
    return native_->HasStencilAttachment();
}

RenderPass^ RenderTarget::RenderPass::get()
{
    return renderPass_;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
