/*
 * CsSamplerFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsPipelineStateFlags.h"
#include "CsColor.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class SamplerAddressMode
{
    Repeat,
    Mirror,
    Clamp,
    Border,
    MirrorOnce,
};

public enum class SamplerFilter
{
    Nearest,
    Linear,
};


/* ----- Structures ----- */

public ref class SamplerDescriptor
{

    public:

        SamplerDescriptor();

        property SamplerAddressMode AddressModeU;
        property SamplerAddressMode AddressModeV;
        property SamplerAddressMode AddressModeW;
        property SamplerFilter      MinFilter;
        property SamplerFilter      MagFilter;
        property SamplerFilter      MipMapFilter;
        property bool               MipMapping;
        property float              MipMapLODBias;
        property float              MinLOD;
        property float              MaxLOD;
        property unsigned int       MaxAnisotropy;
        property bool               CompareEnabled;
        property CompareOp          CompareOp;
        property ColorRGBA<float>^  BorderColor;

};


} // /namespace SharpLLGL



// ================================================================================
