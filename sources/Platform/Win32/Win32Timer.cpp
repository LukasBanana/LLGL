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


namespace LLGL
{

namespace Timer
{


/*
Specifies whether to enable the adjustment for unexpected leaps in the Win32 performance counter.
This is caused by unexpected data across the PCI to ISA bridge, aka south bridge. See Microsoft KB274323.
*/
#define LLGL_LEAP_FORWARD_ADJUSTMENT

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

LLGL_EXPORT std::uint64_t Tick()
{
    LARGE_INTEGER highResTick;
    QueryPerformanceCounter(&highResTick);

    #ifdef LLGL_LEAP_FORWARD_ADJUSTMENT

    static std::atomic<DWORD> lastLowResTick;
    static std::atomic<LONGLONG> lastHighResTick;
    static std::atomic<LONGLONG> lastHighResElapsedTime;

    /* Check for unexpected leaps */
    const DWORD     lowResTick          = GetTickCount();

    const DWORD     prevLowResTick      = lastLowResTick.exchange(lowResTick);
    const LONGLONG  prevHighResTick     = lastHighResTick.exchange(highResTick.QuadPart);

    static const LONGLONG frequency = GetPerformanceFrequencyQuadPart();

    const DWORD     elapsedLowResMS     = (lowResTick - prevLowResTick);
    LONGLONG        elapsedHighResMS    = (highResTick.QuadPart - prevHighResTick) * 1000 / frequency;

    const LONGLONG  millisecondsOff     = elapsedHighResMS - elapsedLowResMS;

    if (std::abs(millisecondsOff) > 100 && prevLowResTick > 0)
    {
        /* Adjust leap by difference */
        const LONGLONG prevElapsedTime = lastHighResElapsedTime.load();

        const LONGLONG adjustment = (std::min)(
            (millisecondsOff * frequency / 1000),
            (elapsedHighResMS - prevElapsedTime)
        );
        highResTick.QuadPart -= adjustment;
        elapsedHighResMS -= adjustment;

        /* Update last state of timer */
        lastHighResTick.store(highResTick.QuadPart);
    }

    /* Store last elapsed time */
    lastHighResElapsedTime.store(elapsedHighResMS);

    #endif // /LLGL_LEAP_FORWARD_ADJUSTMENT

    return highResTick.QuadPart;
}


} // /namespace Timer

} // /namespace LLGL



// ================================================================================
