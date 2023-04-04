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


static std::atomic<DWORD> g_lastLowResTick;
static std::atomic<LONGLONG> g_lastHighResTick;
static std::atomic<LONGLONG> g_lastHighResElapsedTime;

/*
Specifies whether to enable the adjustment for unexpected leaps in the Win32 performance counter.
This is caused by unexpected data across the PCI to ISA bridge, aka south bridge. See Microsoft KB274323.
*/
#define LLGL_LEAP_FORWARD_ADJUSTMENT

LLGL_EXPORT std::uint64_t Frequency()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return static_cast<std::uint64_t>(frequency.QuadPart);
}

LLGL_EXPORT std::uint64_t Tick()
{
    LARGE_INTEGER highResTick;
    QueryPerformanceCounter(&highResTick);

    #ifdef LLGL_LEAP_FORWARD_ADJUSTMENT

    /* Check for unexpected leaps */
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    const DWORD     lowResTick          = GetTickCount();

    const DWORD     prevLowResTick      = g_lastLowResTick.exchange(lowResTick);
    const LONGLONG  prevHighResTick     = g_lastHighResTick.exchange(highResTick.QuadPart);

    const DWORD     elapsedLowResMS     = (lowResTick - prevLowResTick);
    LONGLONG        elapsedHighResMS    = (highResTick.QuadPart - prevHighResTick) * 1000 / frequency.QuadPart;

    const LONGLONG  millisecondsOff     = elapsedHighResMS - elapsedLowResMS;

    if (std::abs(millisecondsOff) > 100)
    {
        /* Adjust leap by difference */
        const LONGLONG prevElapsedTime = g_lastHighResElapsedTime.load();

        const LONGLONG adjustment = (std::min)(
            (millisecondsOff * frequency.QuadPart / 1000),
            (elapsedHighResMS - prevElapsedTime)
        );
        highResTick.QuadPart -= adjustment;
        elapsedHighResMS -= adjustment;

        /* Update last state of timer */
        g_lastHighResTick.store(highResTick.QuadPart);
    }

    /* Store last elapsed time */
    g_lastHighResElapsedTime.store(elapsedHighResMS);

    #endif // /LLGL_LEAP_FORWARD_ADJUSTMENT

    return highResTick.QuadPart;
}


} // /namespace Timer

} // /namespace LLGL



// ================================================================================
