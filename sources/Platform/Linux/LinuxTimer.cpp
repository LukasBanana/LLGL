/*
 * LinuxTimer.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxTimer.h"
#include <algorithm>


namespace LLGL
{


std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new LinuxTimer());
}

LinuxTimer::LinuxTimer()
{
}

void LinuxTimer::Start()
{
}

double LinuxTimer::Stop()
{
    return 0.0;
}

double LinuxTimer::GetFrequency() const
{
    return 1000.0; //todo...
}


} // /namespace LLGL



// ================================================================================
