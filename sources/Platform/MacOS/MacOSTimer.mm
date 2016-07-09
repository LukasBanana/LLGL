/*
 * MacOSTimer.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSTimer.h"


namespace LLGL
{

    
std::unique_ptr<Timer> Timer::Create()
{
    return std::unique_ptr<Timer>(new MacOSTimer());
}

MacOSTimer::MacOSTimer()
{
}

void MacOSTimer::Start()
{
}

double MacOSTimer::Stop()
{
    return 0.0;
}

double MacOSTimer::GetFrequency() const
{
    return 0.0;
}
    
    
} // /namespace LLGL



// ================================================================================
