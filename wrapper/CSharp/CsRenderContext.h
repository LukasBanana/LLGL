/*
 * CsRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/RenderContext.h>
#include "CsRenderTarget.h"
#include "CsRenderContextFlags.h"
#include "CsWindow.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class RenderContext : public RenderTarget
{

    public:

        /* ----- Common ----- */

        RenderContext(LLGL::RenderContext* instance);

        void Present();

        #if 0
        Format GetColorFormat();
        Format GetDepthStencilFormat();
        #endif

        property Window^ Surface
        {
            Window^ get();
        }

        /* ----- Configuration ----- */

        property VideoModeDescriptor^ VideoMode
        {
            VideoModeDescriptor^ get();
            void set(VideoModeDescriptor^ value);
        }

        property VsyncDescriptor^ Vsync
        {
            VsyncDescriptor^ get();
            void set(VsyncDescriptor^ value);
        };

    private:

        Window^ surface_ = nullptr;

};


} // /namespace SharpLLGL



// ================================================================================
