/*
 * Timer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TIMER_H
#define LLGL_TIMER_H


#include <LLGL/Export.h>
#include <memory>
#include <cstdint>


namespace LLGL
{


// Interface for a Timer class
class LLGL_EXPORT Timer
{

    public:

        virtual ~Timer();

        //! Creates a platform specific timer object.
        static std::unique_ptr<Timer> Create();

        //! Starts the timer.
        virtual void Start() = 0;

        //! Stops the timer and returns the elapsed ticks since "Start" was called.
        virtual std::uint64_t Stop() = 0;

        //! Returns the frequency resolution of this timer, or rather 'ticks per second' (e.g. for microseconds this is 1000000).
        virtual std::uint64_t GetFrequency() const = 0;

        /**
        \brief Returns true if the timer is currently running.
        \remarks This is true between a call to "Start" and a call to "Stop".
        \see Start
        \see Stop
        */
        virtual bool IsRunning() const = 0;

        /**
        \brief Measures the time (elapsed time, and frame count) for each frame.
        \see GetDeltaTime
        */
        void MeasureTime();

        /**
        \brief Returns the elapsed time (in seconds) between the current and the previous frame.
        \remarks This requires that "MeasureTime" is called once every frame.
        \see MeasureTime
        */
        double GetDeltaTime() const
        {
            return deltaTime_;
        }

    private:
        
        double deltaTime_ = 0.0;

};


} // /namespace LLGL


#endif



// ================================================================================
