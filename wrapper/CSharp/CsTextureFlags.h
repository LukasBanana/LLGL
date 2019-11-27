/*
 * CsTextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsTypes.h"
#include "CsFormat.h"
#include "CsResourceFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

/// <summary>Texture type enumeration.</summary>
public enum class TextureType
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Texture1DArray,
    Texture2DArray,
    TextureCubeArray,
    Texture2DMS,
    Texture2DMSArray,
};


/* ----- Structures ----- */

public ref class TextureDescriptor
{

    public:

        TextureDescriptor();

        property TextureType    Type;
        property BindFlags      BindFlags;
        property MiscFlags      MiscFlags;
        property Format         Format;
        property Extent3D^      Extent;
        property unsigned int   ArrayLayers;
        property unsigned int   MipLevels;
        property unsigned int   Samples;

};


} // /namespace SharpLLGL



// ================================================================================
