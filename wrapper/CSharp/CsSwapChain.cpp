/*
 * CsSwapChain.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsSwapChain.h"
#include "CsHelper.h"


namespace SharpLLGL
{


SwapChain::SwapChain(LLGL::SwapChain* instance) :
    RenderTarget { instance }
{
}

void SwapChain::Present()
{
    static_cast<LLGL::SwapChain*>(Native::get())->Present();
}

Window^ SwapChain::Surface::get()
{
    if (!surface_)
    {
        auto& window = static_cast<LLGL::Window&>(static_cast<LLGL::SwapChain*>(Native::get())->GetSurface());
        surface_ = gcnew Window(&window);
    }
    return surface_;
}

bool SwapChain::ResizeBuffers(Extent2D^ Resolution, ResizeBuffersFlags Flags)
{
    return static_cast<LLGL::SwapChain*>(Native)->ResizeBuffers({ Resolution->Width, Resolution->Height }, static_cast<long>(Flags));
}

bool SwapChain::SwitchFullscreen(bool Enable)
{
    return static_cast<LLGL::SwapChain*>(Native)->SwitchFullscreen(Enable);
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

void SwapChain::VsyncInterval::set(unsigned int value)
{
    static_cast<LLGL::SwapChain*>(Native)->SetVsyncInterval(value);
}


} // /namespace SharpLLGL



// ================================================================================
