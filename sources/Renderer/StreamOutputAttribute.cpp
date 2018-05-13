/*
 * StreamOutputAttribute.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/StreamOutputAttribute.h>
#include "../Core/HelperMacros.h"


namespace LLGL
{


LLGL_EXPORT bool operator == (const StreamOutputAttribute& lhs, const StreamOutputAttribute& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( name           ) &&
        LLGL_COMPARE_MEMBER_EQ( stream         ) &&
        LLGL_COMPARE_MEMBER_EQ( startComponent ) &&
        LLGL_COMPARE_MEMBER_EQ( components     ) &&
        LLGL_COMPARE_MEMBER_EQ( semanticIndex  ) &&
        LLGL_COMPARE_MEMBER_EQ( outputSlot     )
    );
}

LLGL_EXPORT bool operator != (const StreamOutputAttribute& lhs, const StreamOutputAttribute& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
