/*
 * CsVertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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


public ref class VertexAttribute
{

    public:

        VertexAttribute();
        VertexAttribute(String^ name, Format format);
        VertexAttribute(String^ name, Format format, unsigned int instanceDivisor);
        VertexAttribute(String^ name, unsigned int semanticIndex, Format format, unsigned int instanceDivisor);

        property unsigned int Size
        {
            unsigned int get();
        };

        property String^        Name;
        property Format         Format;
        property unsigned int   InstanceDivisor;
        property unsigned int   Offset;
        property unsigned int   SemanticIndex;

};

public ref class VertexFormat
{

    public:

        VertexFormat();

        void AppendAttribute(VertexAttribute^ attrib);

        property List<VertexAttribute^>^    Attributes;
        property unsigned int               Stride;
        property unsigned int               InputSlot;

};


} // /namespace SharpLLGL



// ================================================================================
