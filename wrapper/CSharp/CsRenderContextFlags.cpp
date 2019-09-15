/*
 * CsRenderContextFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderContextFlags.h"


namespace SharpLLGL
{


VsyncDescriptor::VsyncDescriptor()
{
    Enabled     = true;
    RefreshRate = 60;
    Interval    = 1;
}

VideoModeDescriptor::VideoModeDescriptor()
{
    Resolution      = gcnew Extent2D();
    ColorBits       = 32;
    DepthBits       = 24;
    StencilBits     = 8;
    Fullscreen      = false;
    SwapChainSize   = 2;
}

RendererConfigurationOpenGL::RendererConfigurationOpenGL()
{
    ContextProfile  = OpenGLContextProfile::CompatibilityProfile;
    MajorVersion    = -1;
    MinorVersion    = -1;
}

RenderContextDescriptor::RenderContextDescriptor()
{
    Vsync       = gcnew VsyncDescriptor();
    Samples     = 1;
    VideoMode   = gcnew VideoModeDescriptor();
}


} // /namespace SharpLLGL



// ================================================================================
