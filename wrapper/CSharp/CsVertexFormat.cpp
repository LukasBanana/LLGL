/*
 * CsVertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsVertexFormat.h"
#include "CsHelper.h"
#include <LLGL/VertexAttribute.h>
#include <algorithm>


namespace SharpLLGL
{


/*
 * VertexAttribute class
 */

VertexAttribute::VertexAttribute()
{
    Name            = "";
    Format          = SharpLLGL::Format::Undefined;
    Location        = 0;
    SemanticIndex   = 0;
    Slot            = 0;
    Offset          = 0;
    Stride          = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
    InstanceDivisor = 0;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format, unsigned int location)
{
    Name            = name;
    Format          = format;
    Location        = location;
    SemanticIndex   = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
    Slot            = 0;
    Offset          = 0;
    Stride          = 0;
    InstanceDivisor = 0;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format, unsigned int location, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    Location        = location;
    SemanticIndex   = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
    Slot            = 0;
    Offset          = 0;
    Stride          = 0;
    InstanceDivisor = instanceDivisor;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format, unsigned int location, unsigned int instanceDivisor, SharpLLGL::SystemValue systemValue)
{
    //TODO
}

VertexAttribute::VertexAttribute(String^ name, unsigned int semanticIndex, SharpLLGL::Format format, unsigned int location, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    Location        = location;
    SemanticIndex   = semanticIndex;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
    Slot            = 0;
    Offset          = 0;
    Stride          = 0;
    InstanceDivisor = instanceDivisor;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor)
{
    //TODO
}

VertexAttribute::VertexAttribute(String^ semanticName, unsigned int semanticIndex, SharpLLGL::Format format, unsigned int location, unsigned int offset, unsigned int stride, unsigned int slot, unsigned int instanceDivisor)
{
    //TODO
}

unsigned int VertexAttribute::Size::get()
{
    const auto& formatAttribs = LLGL::GetFormatAttribs(static_cast<LLGL::Format>(Format));
    if ((formatAttribs.flags & LLGL::FormatFlags::SupportsVertex) != 0)
        return (formatAttribs.bitSize / 8);
    else
        return 0;
};


/*
 * VertexFormat class
 */

VertexFormat::VertexFormat()
{
    Attributes = gcnew List<VertexAttribute^>();
}

void VertexFormat::AppendAttribute(VertexAttribute^ attrib)
{
    /* Set offset to previous offset plus previous format size */
    if (Attributes->Count > 0)
        attrib->Offset = Attributes[Attributes->Count - 1]->Stride;

    /* Append new attribute */
    Attributes->Add(attrib);

    /* Update stride for all attributes */
    unsigned int stride = 0;
    for (int i = 0; i < Attributes->Count; ++i)
        stride = std::max(stride, Attributes[i]->Offset + Attributes[i]->Size);

    for (int i = 0; i < Attributes->Count; ++i)
        Attributes[i]->Stride = stride;
}


} // /namespace SharpLLGL



// ================================================================================
