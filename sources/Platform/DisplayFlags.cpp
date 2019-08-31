/*
 * DisplayFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/DisplayFlags.h>
#include "../Core/HelperMacros.h"


namespace LLGL
{


/* ----- Operators ----- */

LLGL_EXPORT bool operator == (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    return
    (
        LLGL_COMPARE_MEMBER_EQ( resolution  ) &&
        LLGL_COMPARE_MEMBER_EQ( refreshRate )
    );
}

LLGL_EXPORT bool operator != (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    return !(lhs == rhs);
}


/* ----- Functions ----- */

LLGL_EXPORT bool CompareSWO(const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs)
{
    const auto lhsNumPixels = lhs.resolution.width * lhs.resolution.height;
    const auto rhsNumPixels = rhs.resolution.width * rhs.resolution.height;

    if (lhsNumPixels < rhsNumPixels)
        return true;
    if (lhsNumPixels > rhsNumPixels)
        return false;

    return (lhs.refreshRate < rhs.refreshRate);
}

/*
Computes the greatest common divisor (GCD) for the two parameters
see https://stackoverflow.com/questions/10956543/gcd-function-in-c-sans-cmath-library
*/
static std::uint32_t ComputeGCD(std::uint32_t a, std::uint32_t b)
{
    while (b != 0)
    {
        auto r = a % b;
        a = b;
        b = r;
    }
    return a;
}

LLGL_EXPORT Extent2D GetExtentRatio(const Extent2D& extent)
{
    auto gcd = ComputeGCD(extent.width, extent.height);
    return { extent.width / gcd, extent.height / gcd };
}


} // /namespace LLGL



// ================================================================================
