/*
 * VideoAdapter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VideoAdapter.h>
#include "HelperMacros.h"


namespace LLGL
{


LLGL_EXPORT bool operator == (const VideoDisplayMode& lhs, const VideoDisplayMode& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( width       ) &&
        LLGL_COMPARE_MEMBER_EQ( height      ) &&
        LLGL_COMPARE_MEMBER_EQ( refreshRate )
    );
}

LLGL_EXPORT bool operator != (const VideoDisplayMode& lhs, const VideoDisplayMode& rhs)
{
    return !(lhs == rhs);
}

LLGL_EXPORT bool CompareSWO(const VideoDisplayMode& lhs, const VideoDisplayMode& rhs)
{
    LLGL_COMPARE_MEMBER_SWO     ( width       );
    LLGL_COMPARE_MEMBER_SWO     ( height      );
    LLGL_COMPARE_MEMBER_SWO_LAST( refreshRate );
}


} // /namespace LLGL



// ================================================================================
