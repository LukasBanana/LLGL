/*
 * VideoAdapter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VideoAdapter.h>
#include "../Core/HelperMacros.h"


namespace LLGL
{


LLGL_EXPORT bool operator == (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( resolution.width  ) &&
        LLGL_COMPARE_MEMBER_EQ( resolution.height ) &&
        LLGL_COMPARE_MEMBER_EQ( refreshRate       )
    );
}

LLGL_EXPORT bool operator != (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    return !(lhs == rhs);
}

LLGL_EXPORT bool CompareSWO(const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    LLGL_COMPARE_MEMBER_SWO     ( resolution.width  );
    LLGL_COMPARE_MEMBER_SWO     ( resolution.height );
    LLGL_COMPARE_MEMBER_SWO_LAST( refreshRate       );
}


} // /namespace LLGL



// ================================================================================
