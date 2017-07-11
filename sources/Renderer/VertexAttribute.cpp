/*
 * VertexAttribute.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VertexAttribute.h>
#include "../Core/HelperMacros.h"


namespace LLGL
{


VertexAttribute::VertexAttribute(
    const std::string& name, const VectorType vectorType, unsigned int instanceDivisor) :
        VertexAttribute { name, 0, vectorType, instanceDivisor }
{
}

VertexAttribute::VertexAttribute(
    const std::string& semanticName, unsigned int semanticIndex, const VectorType vectorType, unsigned int instanceDivisor) :
        name            { semanticName    },
        vectorType      { vectorType      },
        instanceDivisor { instanceDivisor },
        semanticIndex   { semanticIndex   }
{
}

unsigned int VertexAttribute::GetSize() const
{
    return VectorTypeSize(vectorType);
}


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( name            ) &&
        LLGL_COMPARE_MEMBER_EQ( vectorType      ) &&
        LLGL_COMPARE_MEMBER_EQ( instanceDivisor ) &&
        LLGL_COMPARE_MEMBER_EQ( conversion      ) &&
        LLGL_COMPARE_MEMBER_EQ( offset          ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex   ) &&
        LLGL_COMPARE_MEMBER_EQ( inputSlot       )
    );
}

LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
