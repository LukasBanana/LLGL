/*
 * C99Timer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Timer.h>
#include <LLGL-C/Timer.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT uint64_t llglTimerFrequency()
{
    return Timer::Frequency();
}

LLGL_C_EXPORT uint64_t llglTimerTick()
{
    return Timer::Tick();
}


// } /namespace LLGL



// ================================================================================
