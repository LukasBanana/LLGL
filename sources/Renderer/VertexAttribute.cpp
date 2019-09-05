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
    const char*         name,
    const Format        format,
    std::uint32_t       location,
    std::uint32_t       instanceDivisor,
    const SystemValue   systemValue)
:
    name            { name            },
    format          { format          },
    location        { location        },
    systemValue     { systemValue     },
    instanceDivisor { instanceDivisor }
{
}

VertexAttribute::VertexAttribute(
    const char*     semanticName,
    std::uint32_t   semanticIndex,
    const Format    format,
    std::uint32_t   location,
    std::uint32_t   instanceDivisor)
:
    name            { semanticName    },
    format          { format          },
    location        { location        },
    semanticIndex   { semanticIndex   },
    instanceDivisor { instanceDivisor }
{
}

VertexAttribute::VertexAttribute(
    const char*     name,
    const Format    format,
    std::uint32_t   location,
    std::uint32_t   offset,
    std::uint32_t   stride,
    std::uint32_t   slot,
    std::uint32_t   instanceDivisor)
:
    VertexAttribute { name, 0, format, location, offset, stride, slot, instanceDivisor }
{
}

VertexAttribute::VertexAttribute(
    const char*     semanticName,
    std::uint32_t   semanticIndex,
    const Format    format,
    std::uint32_t   location,
    std::uint32_t   offset,
    std::uint32_t   stride,
    std::uint32_t   slot,
    std::uint32_t   instanceDivisor)
:
    name            { semanticName    },
    format          { format          },
    location        { location        },
    semanticIndex   { semanticIndex   },
    slot            { slot            },
    offset          { offset          },
    stride          { stride          },
    instanceDivisor { instanceDivisor }
{
}

std::uint32_t VertexAttribute::GetSize() const
{
    const auto& formatAttribs = GetFormatAttribs(format);
    if ((formatAttribs.flags & FormatFlags::SupportsVertex) != 0)
        return (formatAttribs.bitSize / 8);
    else
        return 0;
}


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( name            ) &&
        LLGL_COMPARE_MEMBER_EQ( format          ) &&
        LLGL_COMPARE_MEMBER_EQ( location        ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex   ) &&
        LLGL_COMPARE_MEMBER_EQ( systemValue     ) &&
        LLGL_COMPARE_MEMBER_EQ( slot            ) &&
        LLGL_COMPARE_MEMBER_EQ( offset          ) &&
        LLGL_COMPARE_MEMBER_EQ( stride          ) &&
        LLGL_COMPARE_MEMBER_EQ( instanceDivisor )
    );
}

LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
