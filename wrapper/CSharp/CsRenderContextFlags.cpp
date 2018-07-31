/*
 * CsRenderContextFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderContextFlags.h"


namespace LHermanns
{

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

ProfileOpenGLDescriptor::ProfileOpenGLDescriptor()
{
    ContextProfile  = OpenGLContextProfile::CompatibilityProfile;
    MajorVersion    = -1;
    MinorVersion    = -1;
}

RenderContextDescriptor::RenderContextDescriptor()
{
    Vsync           = gcnew VsyncDescriptor();
    MultiSampling   = gcnew MultiSamplingDescriptor();
    VideoMode       = gcnew VideoModeDescriptor();
    ProfileOpenGL   = gcnew ProfileOpenGLDescriptor();
}


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
