/*
 * AndroidTimer.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AndroidTimer.h"


namespace LLGL
{

    
std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new AndroidTimer());
}

AndroidTimer::AndroidTimer()
{
}

void AndroidTimer::Start()
{
}

std::uint64_t AndroidTimer::Stop()
{
    return 0;
}

std::uint64_t AndroidTimer::GetFrequency() const
{
    return 0;
}

bool AndroidTimer::IsRunning() const
{
    return false;
}
    
    
} // /namespace LLGL



// ================================================================================
