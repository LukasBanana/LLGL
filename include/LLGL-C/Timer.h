/*
 * Timer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_TIMER_H
#define LLGL_C99_TIMER_H


#include <LLGL-C/Export.h>
#include <stdint.h>


LLGL_C_EXPORT uint64_t llglTimerFrequency();
LLGL_C_EXPORT uint64_t llglTimerTick();


#endif



// ================================================================================
