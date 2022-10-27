/*
 * CsRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderContext.h"
#include "CsHelper.h"


namespace SharpLLGL
{


RenderContext::RenderContext(LLGL::RenderContext* instance) :
    RenderTarget { instance }
{
}

void RenderContext::Present()
{
    static_cast<LLGL::RenderContext*>(Native::get())->Present();
}

Window^ RenderContext::Surface::get()
{
    if (!surface_)
    {
        auto& window = static_cast<LLGL::Window&>(static_cast<LLGL::RenderContext*>(Native::get())->GetSurface());
        surface_ = gcnew Window(&window);
    }
    return surface_;
}

bool RenderContext::ResizeBuffers(Extent2D^ Resolution, ResizeBuffersFlags Flags)
{
    return static_cast<LLGL::RenderContext*>(Native)->ResizeBuffers({ Resolution->Width, Resolution->Height }, static_cast<long>(Flags));
}

bool RenderContext::SwitchFullscreen(bool Enable)
{
    return static_cast<LLGL::RenderContext*>(Native)->SwitchFullscreen(Enable);
}

/* ----- Configuration ----- */

static void Convert(SwapChainDescriptor^ dst, const LLGL::SwapChainDescriptor& src)
{
    dst->Resolution->Width  = src.resolution.width;
    dst->Resolution->Height = src.resolution.height;
    dst->Samples            = src.samples;
    dst->ColorBits          = src.colorBits;
    dst->DepthBits          = src.depthBits;
    dst->StencilBits        = src.stencilBits;
    dst->SwapBuffers        = src.swapBuffers;
    dst->Fullscreen         = src.fullscreen;
}

void RenderContext::VsyncInterval::set(unsigned int value)
{
    static_cast<LLGL::RenderContext*>(Native)->SetVsyncInterval(value);
}


} // /namespace SharpLLGL



// ================================================================================
