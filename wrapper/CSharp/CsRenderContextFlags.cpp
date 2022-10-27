/*
 * CsRenderContextFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderContextFlags.h"


namespace SharpLLGL
{


RendererConfigurationOpenGL::RendererConfigurationOpenGL()
{
    ContextProfile  = OpenGLContextProfile::CompatibilityProfile;
    MajorVersion    = 0;
    MinorVersion    = 0;
}

SwapChainDescriptor::SwapChainDescriptor()
{
    Resolution  = gcnew Extent2D();
    Samples     = 1;
    ColorBits   = 32;
    DepthBits   = 24;
    StencilBits = 8;
    SwapBuffers = 2;
    Fullscreen  = false;
}


} // /namespace SharpLLGL



// ================================================================================
