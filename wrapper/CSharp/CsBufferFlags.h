/*
 * CsBufferFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsFormat.h"
#include "CsVertexFormat.h"
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

public enum class StorageBufferType
{
    Undefined,
    TypedBuffer,
    StructuredBuffer,
    ByteAddressBuffer,
    RWTypedBuffer,
    RWStructuredBuffer,
    RWByteAddressBuffer,
    AppendStructuredBuffer,
    ConsumeStructuredBuffer,
};


/* ----- Structures ----- */

public ref class BufferDescriptor
{

    public:

        BufferDescriptor();

        property System::UInt64             Size;
        property unsigned int               Stride;
        property Format                     Format;
        property BindFlags                  BindFlags;
        property CPUAccessFlags             CPUAccessFlags;
        property MiscFlags                  MiscFlags;
        property List<VertexAttribute^>^    VertexAttribs;

};


} // /namespace SharpLLGL



// ================================================================================
