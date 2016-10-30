/*
 * IOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

double IOSTimer::Stop()
{
    return 0.0;
}

double IOSTimer::GetFrequency() const
{
    return 0.0;
}
    
    
} // /namespace LLGL



// ================================================================================
