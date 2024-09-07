/*
 * Stopwatch.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Stopwatch.h"
#include <LLGL/Timer.h>


Stopwatch::Stopwatch() :
    frequency_ { LLGL::Timer::Frequency() }
{
}

void Stopwatch::Start()
{
    isRunning_ = true;
    startTick_ = LLGL::Timer::Tick();
}

std::uint64_t Stopwatch::Stop()
{
    isRunning_ = false;
    const std::uint64_t endTick = LLGL::Timer::Tick();
    return endTick - startTick_;
}

void Stopwatch::MeasureTime()
{
    const bool wasRunning = IsRunning();
    const std::uint64_t elapsed = Stop();
    Start();
    if (wasRunning)
        deltaTime_ = static_cast<double>(elapsed) / static_cast<double>(GetFrequency());
}


