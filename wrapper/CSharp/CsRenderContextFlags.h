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


/* ----- Structures ----- */

public ref class VsyncDescriptor
{

    public:

        VsyncDescriptor();

        property bool           Enabled;
        property unsigned int   RefreshRate;
        property unsigned int   Interval;

};

public ref class VideoModeDescriptor
{

    public:

        VideoModeDescriptor();

        property Extent2D^       Resolution;
        property int             ColorBits;
        property int             DepthBits;
        property int             StencilBits;
        property bool            Fullscreen;
        property unsigned int    SwapChainSize;

};

public ref class RendererConfigurationOpenGL
{

    public:

        RendererConfigurationOpenGL();

        property OpenGLContextProfile   ContextProfile;
        property int                    MajorVersion;
        property int                    MinorVersion;

};

public ref class RenderContextDescriptor
{

    public:

        RenderContextDescriptor();

        property VsyncDescriptor^       Vsync;
        property unsigned int           Samples;
        property VideoModeDescriptor^   VideoMode;

};


} // /namespace SharpLLGL



// ================================================================================
