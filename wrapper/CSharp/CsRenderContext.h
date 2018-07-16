/*
 * CsRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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


namespace LHermanns
{

namespace LLGL
{


public ref class RenderContext : public RenderTarget
{

    public:

        /* ----- Common ----- */

        RenderContext(::LLGL::RenderContext* instance);

        void Present();

        #if 0
        Format QueryColorFormat();
        Format QueryDepthStencilFormat();
        #endif

        property Window^ Surface
        {
            Window^ get();
        }

        #if 0
        /* ----- Configuration ----- */

        bool SetVideoMode(VideoModeDescriptor^ videoModeDesc)
        VideoModeDescriptor^ GetVideoMode();

        SetVsync(VsyncDescriptor^ vsyncDesc);
        VsyncDescriptor^ GetVsync();
        #endif

    private:

        Window^ surface_ = nullptr;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
