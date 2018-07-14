/*
 * CsRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderContext.h"
#include "CsHelper.h"


namespace LHermanns
{

namespace LLGL
{


RenderContext::RenderContext(::LLGL::RenderContext* instance) :
    instance_ { instance }
{
}

void RenderContext::Present()
{
    instance_->Present();
}

Window^ RenderContext::Surface::get()
{
    if (!surface_)
    {
        auto& window = static_cast<::LLGL::Window&>(instance_->GetSurface());
        surface_ = gcnew Window(&window);
    }
    return surface_;
}

void* RenderContext::Native::get()
{
    return instance_;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
