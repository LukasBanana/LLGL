/*
 * VertexAttribute.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VertexAttribute.h>
#include "HelperMacros.h"


namespace LLGL
{


VertexAttribute::VertexAttribute(
    const std::string& name, const DataType dataType, unsigned int components, unsigned int instanceDivisor) :
        VertexAttribute( name, 0, dataType, components, instanceDivisor )
{
}

VertexAttribute::VertexAttribute(
    const std::string& semanticName, unsigned int semanticIndex, const DataType dataType, unsigned int components, unsigned int instanceDivisor) :
        name            ( semanticName    ),
        dataType        ( dataType        ),
        components      ( components      ),
        instanceDivisor ( instanceDivisor ),
        semanticIndex   ( semanticIndex   )
{
}

unsigned int VertexAttribute::GetSize() const
{
    return (DataTypeSize(dataType) * components);
}


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( name            ) &&
        LLGL_COMPARE_MEMBER_EQ( dataType        ) &&
        LLGL_COMPARE_MEMBER_EQ( components      ) &&
        LLGL_COMPARE_MEMBER_EQ( conversion      ) &&
        LLGL_COMPARE_MEMBER_EQ( instanceDivisor ) &&
        LLGL_COMPARE_MEMBER_EQ( offset          ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex   )
    );
}

LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
