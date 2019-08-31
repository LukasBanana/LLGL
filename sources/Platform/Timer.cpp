/*
 * Timer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Timer.h>


namespace LLGL
{


void Timer::MeasureTime()
{
    auto elapsed = Stop();
    Start();
    deltaTime_ = static_cast<double>(elapsed) / static_cast<double>(GetFrequency());
}


} // /namespace LLGL



// ================================================================================
