/*
 * CsRenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>


using namespace System;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class CPUAccess
{
    ReadOnly,
    WriteOnly,
    ReadWrite,
};


/* ----- Structures ----- */

public ref class RendererInfo
{

    public:

        RendererInfo();

        /// <summary>Rendering API name and version (e.g. "OpenGL 4.6").</summary>
        property String^ RendererName;

        /// <summary>Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").</summary>
        property String^ DeviceName;

        /// <summary>Vendor name of the renderer device (e.g. "NVIDIA Corporation").</summary>
        property String^ VendorName;

        /// <summary>Shading language version (e.g. "GLSL 4.50").</summary>
        property String^ ShadingLanguageName;

};



} // /namespace SharpLLGL



// ================================================================================
