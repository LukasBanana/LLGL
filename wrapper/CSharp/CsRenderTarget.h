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

namespace LLGL
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

        bool IsRenderContext();

        Extent2D^ GetResolution();

        unsigned int GetNumColorAttachments();

        bool HasDepthAttachment();
        bool HasStencilAttachment();

        property RenderPass^ RenderPass
        {
            LHermanns::LLGL::RenderPass^ get();
        }

    private:

        ::LLGL::RenderTarget*           native_     = nullptr;
        LHermanns::LLGL::RenderPass^    renderPass_ = nullptr;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
