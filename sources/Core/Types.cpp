/*
 * Types.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Types.h>
#include "CoreUtils.h"
#include <stdint.h>
#include <algorithm>


namespace LLGL
{


/* ----- Extent Operators ----- */

static std::uint32_t AddUInt32Clamped(std::uint32_t lhs, std::uint32_t rhs)
{
    return static_cast<std::uint32_t>(
        std::min<std::uint64_t>(
            static_cast<std::uint64_t>(lhs) + static_cast<std::uint64_t>(rhs),
            UINT32_MAX
        )
    );
}

static constexpr std::uint32_t SubUInt32Clamped(std::uint32_t lhs, std::uint32_t rhs)
{
    return (rhs < lhs ? lhs - rhs : 0u);
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

static constexpr std::int32_t ClampToInt32(std::int64_t x)
{
    return static_cast<std::int32_t>(Clamp<std::int64_t>(x, INT32_MIN, INT32_MAX));
}

static constexpr std::int32_t AddInt32Clamped(std::int32_t lhs, std::int32_t rhs)
{
    return ClampToInt32(static_cast<std::int64_t>(lhs) + static_cast<std::int64_t>(rhs));
}

static constexpr std::int32_t SubInt32Clamped(std::int32_t lhs, std::int32_t rhs)
{
    return ClampToInt32(static_cast<std::int64_t>(lhs) - static_cast<std::int64_t>(rhs));
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
