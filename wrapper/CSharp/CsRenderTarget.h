/*
 * CsRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/RenderTarget.h>
#include "CsTypes.h"
#include "CsRenderSystemChilds.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LHermanns
{

namespace SharpLLGL
{


public ref class RenderTarget
{

    public:

        /* ----- Common ----- */

        RenderTarget(::LLGL::RenderTarget* native);

        property ::LLGL::RenderTarget* Native
        {
            ::LLGL::RenderTarget* get();
        }

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
            LHermanns::SharpLLGL::RenderPass^ get();
        }

    private:

        ::LLGL::RenderTarget*           native_     = nullptr;
        LHermanns::SharpLLGL::RenderPass^    renderPass_ = nullptr;

};


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
