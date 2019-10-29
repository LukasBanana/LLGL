/*
 * LinuxTimer.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxTimer.h"
#include <algorithm>
#include <cstdint>


namespace LLGL
{


std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new LinuxTimer {});
}

LinuxTimer::LinuxTimer()
{
    startTime_.tv_sec   = 0;
    startTime_.tv_nsec  = 0;
}

void LinuxTimer::Start()
{
    running_ = true;
    clock_gettime(CLOCK_MONOTONIC, &startTime_);
}

static std::uint64_t MonotonicTimeToUInt64(const timespec& t)
{
    return (static_cast<std::uint64_t>(t.tv_sec) * 1000000000ull + static_cast<std::uint64_t>(t.tv_nsec));
}

std::uint64_t LinuxTimer::Stop()
{
    if (running_)
    {
        running_ = false;

        timespec endTime;
        clock_gettime(CLOCK_MONOTONIC, &endTime);

        auto t0 = MonotonicTimeToUInt64(startTime_);
        auto t1 = MonotonicTimeToUInt64(endTime);

        return (t1 > t0 ? t1 - t0 : 0);
    }
    return 0;
}

std::uint64_t LinuxTimer::GetFrequency() const
{
    return 1000000000ull;
}

bool LinuxTimer::IsRunning() const
{
    return running_;
}


} // /namespace LLGL



// ================================================================================
