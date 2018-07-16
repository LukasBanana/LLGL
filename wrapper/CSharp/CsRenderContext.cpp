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
    RenderTarget { instance }
{
}

void RenderContext::Present()
{
    static_cast<::LLGL::RenderContext*>(Native::get())->Present();
}

Window^ RenderContext::Surface::get()
{
    if (!surface_)
    {
        auto& window = static_cast<::LLGL::Window&>(static_cast<::LLGL::RenderContext*>(Native::get())->GetSurface());
        surface_ = gcnew Window(&window);
    }
    return surface_;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
