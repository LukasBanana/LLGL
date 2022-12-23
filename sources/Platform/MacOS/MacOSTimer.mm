/*
 * MacOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
