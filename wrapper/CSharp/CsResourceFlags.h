/*
 * CsResourceFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumeration ----- */

public enum class ResourceType
{
    Undefined,
    Buffer,
    Texture,
    Sampler,
};


/* ----- Flags ----- */

[Flags]
public enum class BindFlags
{
    None                    = 0,
    VertexBuffer            = (1 << 0),
    IndexBuffer             = (1 << 1),
    ConstantBuffer          = (1 << 2),
    StreamOutputBuffer      = (1 << 3),
    IndirectBuffer          = (1 << 4),
    Sampled                 = (1 << 5),
    Storage                 = (1 << 6),
    ColorAttachment         = (1 << 7),
    DepthStencilAttachment  = (1 << 8),
    CombinedSampler         = (1 << 9),
    CopySrc                 = (1 << 10),
    CopyDst                 = (1 << 11),
};

[Flags]
public enum class CPUAccessFlags
{
    None        = 0,
    Read        = (1 << 0),
    Write       = (1 << 1),
    ReadWrite   = (Read | Write),
};

[Flags]
public enum class MiscFlags
{
    None            = 0,
    DynamicUsage    = (1 << 0),
    FixedSamples    = (1 << 1),
    GenerateMips    = (1 << 2),
    NoInitialData   = (1 << 3),
    Append          = (1 << 4),
    Counter         = (1 << 5),
};



} // /namespace SharpLLGL



// ================================================================================
