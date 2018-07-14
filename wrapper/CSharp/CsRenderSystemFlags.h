/*
 * CsRenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>


namespace LHermanns
{

namespace LLGL
{


public ref class RendererInfo
{

    public:

        /// <summary>Rendering API name and version (e.g. "OpenGL 4.6").</summary>
        property System::String^ RendererName;

        /// <summary>Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").</summary>
        property System::String^ DeviceName;

        /// <summary>Vendor name of the renderer device (e.g. "NVIDIA Corporation").</summary>
        property System::String^ VendorName;

        /// <summary>Shading language version (e.g. "GLSL 4.50").</summary>
        property System::String^ ShadingLanguageName;

};



} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
