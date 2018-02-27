/*
 * LinuxTimer.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxTimer.h"
#include <algorithm>
#include <cstdint>


namespace LLGL
{


std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new LinuxTimer());
}

LinuxTimer::LinuxTimer()
{
    startTime_.tv_sec   = 0;
    startTime_.tv_nsec  = 0;
}

void LinuxTimer::Start()
{
    clock_gettime(CLOCK_MONOTONIC, &startTime_);
}

static std::uint64_t MonotonicTimeToUInt64(const timespec& t)
{
    return (static_cast<std::uint64_t>(t.tv_sec) * 1000000000ull + static_cast<std::uint64_t>(t.tv_nsec));
}

double LinuxTimer::Stop()
{
    timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    
    auto t0 = MonotonicTimeToUInt64(startTime_);
    auto t1 = MonotonicTimeToUInt64(endTime);
    
    return (t1 > t0 ? static_cast<double>(t1 - t0) : 0.0);
}

double LinuxTimer::GetFrequency() const
{
    return 1.0e9;
}


} // /namespace LLGL



// ================================================================================
