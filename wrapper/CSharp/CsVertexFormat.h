/*
 * CsVertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsFormat.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


public enum class SystemValue
{
    Undefined,
    ClipDistance,
    Color,
    CullDistance,
    Depth,
    DepthGreater,
    DepthLess,
    FrontFacing,
    InstanceID,
    Position,
    PrimitiveID,
    RenderTargetIndex,
    SampleMask,
    SampleID,
    Stencil,
    VertexID,
    ViewportIndex,
};

public ref class VertexAttribute
{

    public:

        VertexAttribute();
        VertexAttribute(String^ name, Format format, unsigned int location);
        VertexAttribute(String^ name, Format format, unsigned int location, unsigned int instanceDivisor);
        VertexAttribute(String^ name, Format format, unsigned int location, unsigned int instanceDivisor, SystemValue systemValue);
        VertexAttribute(String^ semanticName, unsigned int semanticIndex, Format format, unsigned int location, unsigned int instanceDivisor);
        VertexAttribute(String^ name, Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor);
        VertexAttribute(String^ semanticName, unsigned int semanticIndex, Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor);

        property unsigned int Size
        {
            unsigned int get();
        };

        property String^        Name;
        property Format         Format;
        property unsigned int   Location;
        property unsigned int   SemanticIndex;
        property SystemValue    SystemValue;
        property unsigned int   Slot;
        property unsigned int   Offset;
        property unsigned int   Stride;
        property unsigned int   InstanceDivisor;

};

public ref class VertexFormat
{

    public:

        VertexFormat();

        void AppendAttribute(VertexAttribute^ attrib);

        property List<VertexAttribute^>^ Attributes;

};


} // /namespace SharpLLGL



// ================================================================================
