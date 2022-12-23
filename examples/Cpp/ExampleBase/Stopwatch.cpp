/*
 * Stopwatch.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    auto endTick = LLGL::Timer::Tick();
    return endTick - startTick_;
}

void Stopwatch::MeasureTime()
{
    auto elapsed = Stop();
    Start();
    deltaTime_ = static_cast<double>(elapsed) / static_cast<double>(GetFrequency());
}


