/*
 * LinuxTimer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Timer.h>
#include <time.h>


namespace LLGL
{

namespace Timer
{


static const std::uint64_t g_nsecFrequency = 1000000000ull;

LLGL_EXPORT std::uint64_t Frequency()
{
    return g_nsecFrequency;
}

static std::uint64_t MonotonicTimeToUInt64(const timespec& t)
{
    return (static_cast<std::uint64_t>(t.tv_sec) * g_nsecFrequency + static_cast<std::uint64_t>(t.tv_nsec));
}

LLGL_EXPORT std::uint64_t Tick()
{
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return MonotonicTimeToUInt64(t);
}


} // /namespace Timer

} // /namespace LLGL



// ================================================================================
