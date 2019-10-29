/*
 * MacOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSTimer.h"


namespace LLGL
{

    
std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new MacOSTimer());
}

MacOSTimer::MacOSTimer()
{
    mach_timebase_info(&timebaseInfo_);
    if (timebaseInfo_.denom == 0)
        throw std::runtime_error("failed to retrieve base information for high resolution timer");
}

void MacOSTimer::Start()
{
    running_    = true;
    startTime_  = mach_absolute_time();
}

std::uint64_t MacOSTimer::Stop()
{
    if (running_)
    {
        running_ = false;
        auto elapsed        = mach_absolute_time() - startTime_;
        auto elapsedNano    = elapsed * timebaseInfo_.numer / timebaseInfo_.denom;
        return static_cast<std::uint64_t>(elapsedNano);
    }
    return 0;
}

std::uint64_t MacOSTimer::GetFrequency() const
{
    return 1000000000ull;
}

bool MacOSTimer::IsRunning() const
{
    return running_;
}
    
    
} // /namespace LLGL



// ================================================================================
