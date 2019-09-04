/*
 * CsImageFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsTypes.h"
#include "CsFormat.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class ImageFormat
{
    R,
    RG,
    RGB,
    BGR,
    RGBA,
    BGRA,
    ARGB,
    ABGR,
    Depth,
    DepthStencil,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
};


/* ----- Structures ----- */

generic <typename T>
public ref class SrcImageDescriptor
{

    public:

        SrcImageDescriptor();

        SrcImageDescriptor(ImageFormat format, DataType dataType, array<T>^ data);

        property ImageFormat    Format;
        property DataType       DataType;
        property array<T>^      Data;

};


} // /namespace SharpLLGL



// ================================================================================
