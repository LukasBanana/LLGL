/*
 * CsVertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsVertexFormat.h"
#include "CsHelper.h"
#include <LLGL/VertexAttribute.h>


namespace LHermanns
{

namespace SharpLLGL
{


/*
 * VertexAttribute class
 */

VertexAttribute::VertexAttribute()
{
    Name            = "";
    Format          = LHermanns::SharpLLGL::Format::Undefined;
    InstanceDivisor = 0;
    Offset          = 0;
    SemanticIndex   = 0;
}

VertexAttribute::VertexAttribute(String^ name, LHermanns::SharpLLGL::Format format)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = 0;
    Offset          = 0;
    SemanticIndex   = 0;
}

VertexAttribute::VertexAttribute(String^ name, LHermanns::SharpLLGL::Format format, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = instanceDivisor;
    Offset          = 0;
    SemanticIndex   = 0;
}

VertexAttribute::VertexAttribute(String^ name, unsigned int semanticIndex, LHermanns::SharpLLGL::Format format, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = instanceDivisor;
    Offset          = 0;
    SemanticIndex   = semanticIndex;
}

unsigned int VertexAttribute::Size::get()
{
    return (::LLGL::FormatBitSize(static_cast<::LLGL::Format>(Format)) / 8);
};


/*
 * VertexFormat class
 */

VertexFormat::VertexFormat()
{
    Attributes  = gcnew List<VertexAttribute^>();
    Stride      = 0;
    InputSlot   = 0;
}

void VertexFormat::AppendAttribute(VertexAttribute^ attrib)
{
    attrib->Offset = Stride;
    Stride += attrib->Size;
    Attributes->Add(attrib);
}


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
