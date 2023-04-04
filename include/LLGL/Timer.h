/*
 * Timer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TIMER_H
#define LLGL_TIMER_H


#include <LLGL/Export.h>
#include <cstdint>


namespace LLGL
{

namespace Timer
{


/**
\brief Returns the timer frequency of the OS. This is the number of ticks per seconds, e.g. 1,000,000 for microseconds.
\see Tick
*/
LLGL_EXPORT std::uint64_t Frequency();

/**
\brief Returns the current 'tick' of a high resolution timer that is OS dependent. The tick frequency can be queried with Frequency.
\remarks The following example illustrates how to query the elapsed time in milliseconds between two time stamps:
\code
const std::uint64_t startTime = LLGL::Timer::Tick();
// Some events ...
const std::uint64_t endTime = LLGL::Timer::Tick();
const double elapsedSeconds = static_cast<double>(endTime - startTime) / LLGL::Timer::Frequency();
std::cout << "Elapsed time: " << elapsedSeconds * 1000.0 << "ms" << std::endl;
\endcode
\see Frequency
*/
LLGL_EXPORT std::uint64_t Tick();


} // /namespace Timer

} // /namespace LLGL


#endif



// ================================================================================
