/*
 * Stopwatch.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_STOPWATCH_H
#define LLGLEXAMPLES_STOPWATCH_H


#include <cstdint>


// Simple class to store changes in time (in seconds) from Start() to Stop() calls.
class Stopwatch
{

    public:

        Stopwatch();

        void Start();

        std::uint64_t Stop();

        void MeasureTime();

        inline double GetDeltaTime() const
        {
            return deltaTime_;
        }

        inline bool IsRunning() const
        {
            return isRunning_;
        }

        inline std::uint64_t GetFrequency() const
        {
            return frequency_;
        }

    private:

        const std::uint64_t frequency_  = 0;
        std::uint64_t       startTick_  = 0;
        double              deltaTime_  = 0.0;
        bool                isRunning_  = false;

};


#endif

