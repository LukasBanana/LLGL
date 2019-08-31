/*
 * VertexAttribute.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VertexAttribute.h>
#include "../Core/HelperMacros.h"


namespace LLGL
{


VertexAttribute::VertexAttribute(
    const std::string& name, const Format format, std::uint32_t instanceDivisor) :
        VertexAttribute { name, 0, format, instanceDivisor }
{
}

VertexAttribute::VertexAttribute(
    const std::string& semanticName, std::uint32_t semanticIndex, const Format format, std::uint32_t instanceDivisor) :
        name            { semanticName    },
        format          { format          },
        instanceDivisor { instanceDivisor },
        semanticIndex   { semanticIndex   }
{
}

std::uint32_t VertexAttribute::GetSize() const
{
    const auto& formatDesc = GetFormatDesc(format);
    if (formatDesc.compressed || formatDesc.sRGB || formatDesc.depth || formatDesc.stencil)
        return 0;
    else
        return (formatDesc.bitSize / 8);
}


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( name            ) &&
        LLGL_COMPARE_MEMBER_EQ( format          ) &&
        LLGL_COMPARE_MEMBER_EQ( instanceDivisor ) &&
        LLGL_COMPARE_MEMBER_EQ( offset          ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex   ) &&
        LLGL_COMPARE_MEMBER_EQ( systemValue     )
    );
}

LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
