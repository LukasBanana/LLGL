/*
 * MacOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSTimer.h"
//#include <CoreServices/CoreServices.h>


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
    startTime_ = mach_absolute_time();
}

double MacOSTimer::Stop()
{
    auto elapsed        = mach_absolute_time() - startTime_;
    auto elapsedNano    = elapsed * timebaseInfo_.numer / timebaseInfo_.denom;
    return static_cast<double>(elapsedNano);
}

double MacOSTimer::GetFrequency() const
{
    return 1.0e9;
}
    
    
} // /namespace LLGL



// ================================================================================
