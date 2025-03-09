/*
 * Win32Timer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Timer.h>
#include "Win32LeanAndMean.h"
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <mutex>


namespace LLGL
{

namespace Timer
{


/*
Specifies whether to enable the adjustment for unexpected leaps in the Win32 performance counter.
This is caused by unexpected data across the PCI to ISA bridge, aka south bridge. See Microsoft KB274323.
*/
#ifndef LLGL_LEAP_FORWARD_ADJUSTMENT
#define LLGL_LEAP_FORWARD_ADJUSTMENT 1
#endif

static LONGLONG GetPerformanceFrequencyQuadPart()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}

LLGL_EXPORT std::uint64_t Frequency()
{
    return static_cast<std::uint64_t>(GetPerformanceFrequencyQuadPart());
}

static ULONGLONG GetWin32TickCount()
{
    #if WINVER >= 0x0600
    return GetTickCount64();
    #else
    return GetTickCount();
    #endif
}

LLGL_EXPORT std::uint64_t Tick()
{
    LARGE_INTEGER highResTick;
    QueryPerformanceCounter(&highResTick);

    #if LLGL_LEAP_FORWARD_ADJUSTMENT

    /* Check for unexpected leaps */
    static std::mutex tickMutex;
    std::lock_guard<std::mutex> guard{ tickMutex };

    static ULONGLONG lastLowResTick;
    static LONGLONG lastHighResTick;
    static LONGLONG lastHighResElapsedTime;

    static const LONGLONG frequency = GetPerformanceFrequencyQuadPart();

    const ULONGLONG lowResTick          = GetWin32TickCount();

    const ULONGLONG elapsedLowResMS     = (lowResTick - lastLowResTick);
    LONGLONG        elapsedHighResMS    = (highResTick.QuadPart - lastHighResTick) * 1000 / frequency;

    const LONGLONG  millisecondsOff     = elapsedHighResMS - elapsedLowResMS;

    if (std::abs(millisecondsOff) > 100 && lastLowResTick > 0)
    {
        /* Adjust leap by difference */
        const LONGLONG adjustment = (std::min)(
            (millisecondsOff * frequency / 1000),
            (elapsedHighResMS - lastHighResElapsedTime)
        );
        highResTick.QuadPart -= adjustment;
        elapsedHighResMS -= adjustment;
    }

    /* Store last elapsed time */
    lastLowResTick          = lowResTick;
    lastHighResTick         = highResTick.QuadPart;
    lastHighResElapsedTime  = elapsedHighResMS;

    #endif // /LLGL_LEAP_FORWARD_ADJUSTMENT

    return highResTick.QuadPart;
}


} // /namespace Timer

} // /namespace LLGL



// ================================================================================
