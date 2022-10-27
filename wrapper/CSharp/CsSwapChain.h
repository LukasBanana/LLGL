/*
 * CsSwapChain.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/SwapChain.h>
#include "CsRenderTarget.h"
#include "CsSwapChainFlags.h"
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

        bool ResizeBuffers(Extent2D^ Resolution, ResizeBuffersFlags Flags);

        bool SwitchFullscreen(bool Enable);

        property Window^ Surface
        {
            Window^ get();
        }

        /* ----- Configuration ----- */

        property unsigned int VsyncInterval
        {
            void set(unsigned int value);
        };

    private:

        Window^ surface_ = nullptr;

};


} // /namespace SharpLLGL



// ================================================================================
