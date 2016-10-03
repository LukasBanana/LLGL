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


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( dataType        ) &&
        LLGL_COMPARE_MEMBER_EQ( conversion      ) &&
        LLGL_COMPARE_MEMBER_EQ( instanceDivisor ) &&
        LLGL_COMPARE_MEMBER_EQ( components      ) &&
        LLGL_COMPARE_MEMBER_EQ( offset          ) &&
        LLGL_COMPARE_MEMBER_EQ( name            ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex   )
    );
}

LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
