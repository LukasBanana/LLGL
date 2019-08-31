/*
 * Types.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Types.h>
#include <limits>
#include <algorithm>


namespace LLGL
{


/* ----- Extent Operators ----- */

static std::uint32_t AddUInt32Clamped(std::uint32_t lhs, std::uint32_t rhs)
{
    const auto xMax = std::numeric_limits<std::uint32_t>::max();

    std::uint64_t x = lhs;
    x += rhs;

    if (x > static_cast<std::uint64_t>(xMax))
        return xMax;
    else
        return lhs + rhs;
}

static std::uint32_t SubUInt32Clamped(std::uint32_t lhs, std::uint32_t rhs)
{
    if (rhs < lhs)
        return lhs - rhs;
    else
        return 0;
}

LLGL_EXPORT Extent2D operator + (const Extent2D& lhs, const Extent2D& rhs)
{
    return Extent2D
    {
        AddUInt32Clamped(lhs.width , rhs.width ),
        AddUInt32Clamped(lhs.height, rhs.height)
    };
}

LLGL_EXPORT Extent2D operator - (const Extent2D& lhs, const Extent2D& rhs)
{
    return Extent2D
    {
        SubUInt32Clamped(lhs.width , rhs.width ),
        SubUInt32Clamped(lhs.height, rhs.height)
    };
}

LLGL_EXPORT Extent3D operator + (const Extent3D& lhs, const Extent3D& rhs)
{
    return Extent3D
    {
        AddUInt32Clamped(lhs.width , rhs.width ),
        AddUInt32Clamped(lhs.height, rhs.height),
        AddUInt32Clamped(lhs.depth , rhs.depth )
    };
}

LLGL_EXPORT Extent3D operator - (const Extent3D& lhs, const Extent3D& rhs)
{
    return Extent3D
    {
        SubUInt32Clamped(lhs.width , rhs.width ),
        SubUInt32Clamped(lhs.height, rhs.height),
        SubUInt32Clamped(lhs.depth , rhs.depth )
    };
}

/* ----- Offset Operators ----- */

static std::int32_t ClampToInt32(std::int64_t x)
{
    const auto xMin = static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min());
    const auto xMax = static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max());

    x = std::max(xMin, std::min(x, xMax));

    return static_cast<std::int32_t>(x);
}

static std::int32_t AddInt32Clamped(std::int32_t lhs, std::int32_t rhs)
{
    std::int64_t x = lhs;
    x += rhs;
    return ClampToInt32(x);
}

static std::int32_t SubInt32Clamped(std::int32_t lhs, std::int32_t rhs)
{
    std::int64_t x = lhs;
    x -= rhs;
    return ClampToInt32(x);
}

LLGL_EXPORT Offset2D operator + (const Offset2D& lhs, const Offset2D& rhs)
{
    return Offset2D
    {
        AddInt32Clamped(lhs.x, rhs.x),
        AddInt32Clamped(lhs.y, rhs.y)
    };
}

LLGL_EXPORT Offset2D operator - (const Offset2D& lhs, const Offset2D& rhs)
{
    return Offset2D
    {
        SubInt32Clamped(lhs.x, rhs.x),
        SubInt32Clamped(lhs.y, rhs.y)
    };
}

LLGL_EXPORT Offset3D operator + (const Offset3D& lhs, const Offset3D& rhs)
{
    return Offset3D
    {
        AddInt32Clamped(lhs.x, rhs.x),
        AddInt32Clamped(lhs.y, rhs.y),
        AddInt32Clamped(lhs.z, rhs.z)
    };
}

LLGL_EXPORT Offset3D operator - (const Offset3D& lhs, const Offset3D& rhs)
{
    return Offset3D
    {
        SubInt32Clamped(lhs.x, rhs.x),
        SubInt32Clamped(lhs.y, rhs.y),
        SubInt32Clamped(lhs.z, rhs.z)
    };
}


} // /namespace LLGL



// ================================================================================
