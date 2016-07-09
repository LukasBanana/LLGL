/*
 * Timer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Timer.h>


namespace LLGL
{


Timer::~Timer()
{
}

void Timer::MeasureTime()
{
    auto elapsed = Stop();
    Start();

    deltaTime_ = elapsed / GetFrequency();

    ++frameCount_;
}

void Timer::ResetFrameCounter()
{
    frameCount_ = 0;
}


} // /namespace LLGL



// ================================================================================
