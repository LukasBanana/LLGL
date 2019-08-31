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

/* ----- Configuration ----- */

static void Convert(VideoModeDescriptor^ dst, const LLGL::VideoModeDescriptor& src)
{
    dst->Resolution->Width  = src.resolution.width;
    dst->Resolution->Height = src.resolution.height;
    dst->ColorBits          = src.colorBits;
    dst->DepthBits          = src.depthBits;
    dst->StencilBits        = src.stencilBits;
    dst->Fullscreen         = src.fullscreen;
    dst->SwapChainSize      = src.swapChainSize;
}

VideoModeDescriptor^ RenderContext::VideoMode::get()
{
    auto managedDesc = gcnew VideoModeDescriptor();
    Convert(managedDesc, static_cast<LLGL::RenderContext*>(Native)->GetVideoMode());
    return managedDesc;
}

static void Convert(LLGL::VideoModeDescriptor& dst, VideoModeDescriptor^ src)
{
    dst.resolution.width    = src->Resolution->Width;
    dst.resolution.height   = src->Resolution->Height;
    dst.colorBits           = src->ColorBits;
    dst.depthBits           = src->DepthBits;
    dst.stencilBits         = src->StencilBits;
    dst.fullscreen          = src->Fullscreen;
    dst.swapChainSize       = src->SwapChainSize;
}

void RenderContext::VideoMode::set(VideoModeDescriptor^ value)
{
    LLGL::VideoModeDescriptor nativeDesc;
    Convert(nativeDesc, value);
    static_cast<LLGL::RenderContext*>(Native)->SetVideoMode(nativeDesc);
}

static void Convert(VsyncDescriptor^ dst, const LLGL::VsyncDescriptor& src)
{
    dst->Enabled        = src.enabled;
    dst->RefreshRate    = src.refreshRate;
    dst->Interval       = src.interval;
}

VsyncDescriptor^ RenderContext::Vsync::get()
{
    auto managedDesc = gcnew VsyncDescriptor();
    Convert(managedDesc, static_cast<LLGL::RenderContext*>(Native)->GetVsync());
    return managedDesc;
}

static void Convert(LLGL::VsyncDescriptor& dst, VsyncDescriptor^ src)
{
    dst.enabled     = src->Enabled;
    dst.refreshRate = src->RefreshRate;
    dst.interval    = src->Interval;
}

void RenderContext::Vsync::set(VsyncDescriptor^ value)
{
    LLGL::VsyncDescriptor nativeDesc;
    Convert(nativeDesc, value);
    static_cast<LLGL::RenderContext*>(Native)->SetVsync(nativeDesc);
}


} // /namespace SharpLLGL



// ================================================================================
