/*
 * IOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IOSTimer.h"


namespace LLGL
{

    
std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new IOSTimer());
}

IOSTimer::IOSTimer()
{
}

void IOSTimer::Start()
{
}

std::uint64_t IOSTimer::Stop()
{
    return 0;
}

std::uint64_t IOSTimer::GetFrequency() const
{
    return 0;
}

bool IOSTimer::IsRunning() const
{
    return false;
}
    
    
} // /namespace LLGL



// ================================================================================
