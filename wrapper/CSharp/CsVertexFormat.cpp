/*
 * CsVertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsVertexFormat.h"
#include "CsHelper.h"
#include <LLGL/VertexAttribute.h>


namespace SharpLLGL
{


/*
 * VertexAttribute class
 */

VertexAttribute::VertexAttribute()
{
    Name            = "";
    Format          = SharpLLGL::Format::Undefined;
    InstanceDivisor = 0;
    Offset          = 0;
    SemanticIndex   = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = 0;
    Offset          = 0;
    SemanticIndex   = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
}

VertexAttribute::VertexAttribute(String^ name, SharpLLGL::Format format, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = instanceDivisor;
    Offset          = 0;
    SemanticIndex   = 0;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
}

VertexAttribute::VertexAttribute(String^ name, unsigned int semanticIndex, SharpLLGL::Format format, unsigned int instanceDivisor)
{
    Name            = name;
    Format          = format;
    InstanceDivisor = instanceDivisor;
    Offset          = 0;
    SemanticIndex   = semanticIndex;
    SystemValue     = SharpLLGL::SystemValue::Undefined;
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



// ================================================================================
