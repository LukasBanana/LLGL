/*
 * MacOSTimer.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Timer.h>
#include <mach/mach_time.h>


namespace LLGL
{

namespace Timer
{


static const std::uint64_t g_nsecFrequency = 1000000000ull;

LLGL_EXPORT std::uint64_t Frequency()
{
    return g_nsecFrequency;
}

LLGL_EXPORT std::uint64_t Tick()
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    if (timebase.denom > 0)
        return (mach_absolute_time() * timebase.numer) / timebase.denom;
    else
        return 0;
}


} // /namespace Timer

} // /namespace LLGL



// ================================================================================
