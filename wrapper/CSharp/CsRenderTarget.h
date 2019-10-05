/*
 * CsRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/RenderTarget.h>
#include "CsTypes.h"
#include "CsRenderSystemChild.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class RenderTarget
{

    public:

        /* ----- Common ----- */

        RenderTarget(LLGL::RenderTarget* native);

        property bool IsRenderContext
        {
            bool get();
        }

        property Extent2D^ Resolution
        {
            Extent2D^ get();
        }

        property unsigned int NumColorAttachments
        {
            unsigned int get();
        }

        property bool HasDepthAttachment
        {
            bool get();
        }

        property bool HasStencilAttachment
        {
            bool get();
        }

        property RenderPass^ RenderPass
        {
            SharpLLGL::RenderPass^ get();
        }

    private:

        LLGL::RenderTarget*     native_     = nullptr;
        SharpLLGL::RenderPass^  renderPass_ = nullptr;

    internal:

        property LLGL::RenderTarget* Native
        {
            LLGL::RenderTarget* get();
        }

};


} // /namespace SharpLLGL



// ================================================================================
