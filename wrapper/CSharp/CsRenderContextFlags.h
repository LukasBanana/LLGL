/*
 * CsRenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsTypes.h"
#include "CsPipelineStateFlags.h"


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class OpenGLContextProfile
{
    CompatibilityProfile,
    CoreProfile,
    ESProfile,
};


/* ----- Flags ----- */

[Flags]
public enum class ResizeBuffersFlags
{
    None            = 0,
    AdaptSurface    = (1 << 0),
    FullscreenMode  = (1 << 1),
    WindowedMode    = (1 << 2),
};


/* ----- Structures ----- */

public ref class SwapChainDescriptor
{

    public:

        SwapChainDescriptor();

        property Extent2D^      Resolution;
        property unsigned int   Samples;
        property int            ColorBits;
        property int            DepthBits;
        property int            StencilBits;
        property unsigned int   SwapBuffers;
        property bool           Fullscreen;

};

public ref class RendererConfigurationOpenGL
{

    public:

        RendererConfigurationOpenGL();

        property OpenGLContextProfile   ContextProfile;
        property int                    MajorVersion;
        property int                    MinorVersion;

};


} // /namespace SharpLLGL



// ================================================================================
