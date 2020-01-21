/*
 * Win32Timer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32Timer.h"
#include <algorithm>


namespace LLGL
{


/*
Specifies whether to enable the adjustment for unexpected leaps in the Win32 performance counter.
This is caused by unexpected data across the PCI to ISA bridge, aka south bridge. See Microsoft KB274323.
*/
#define LLGL_LEAP_FORWARD_ADJUSTMENT

std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Win32Timer>(new Win32Timer{});
}

Win32Timer::Win32Timer()
{
    QueryPerformanceFrequency(&clockFrequency_);
}

void Win32Timer::Start()
{
    /* Query current performance counter ticks */
    QueryPerformanceCounter(&t0_);

    #ifdef LLGL_LEAP_FORWARD_ADJUSTMENT
    startTick_ = GetTickCount();
    #endif

    running_ = true;
}

std::uint64_t Win32Timer::Stop()
{
    /* Reset running state */
    if (!running_)
        return 0;

    running_ = false;

    /* Query elapsed ticks */
    QueryPerformanceCounter(&t1_);
    auto elapsedTime = t1_.QuadPart - t0_.QuadPart;

    #ifdef LLGL_LEAP_FORWARD_ADJUSTMENT

    /* Compute the number of millisecond ticks elapsed */
    auto msecTicks          = static_cast<long long>(1000 * elapsedTime / clockFrequency_.QuadPart);

    /* Check for unexpected leaps */
    auto elapsedLowTicks    = static_cast<long long>(GetTickCount() - startTick_);
    auto msecOff            = msecTicks - elapsedLowTicks;

    if (std::abs(msecOff) > 100)
    {
        /* Adjust the starting time forwards */
        LONGLONG msecAdjustment = std::min<LONGLONG>(
            ( msecOff * clockFrequency_.QuadPart / 1000 ),
            ( elapsedTime - prevElapsedTime_ )
        );
        elapsedTime -= msecAdjustment;
    }

    /* Store the current elapsed time for adjustments next time */
    prevElapsedTime_ = elapsedTime;

    #endif

    /* Return final elapsed time */
    return static_cast<std::uint64_t>(elapsedTime);
}

std::uint64_t Win32Timer::GetFrequency() const
{
    return static_cast<std::uint64_t>(clockFrequency_.QuadPart);
}

bool Win32Timer::IsRunning() const
{
    return running_;
}


} // /namespace LLGL



// ================================================================================
