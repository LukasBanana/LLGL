/*
 * Timer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TIMER_H
#define LLGL_TIMER_H


#include <LLGL/Export.h>
#include <memory>


namespace LLGL
{


// Interface for a Timer class
class LLGL_EXPORT Timer
{

    public:

        using FrameCount = unsigned long long;

        virtual ~Timer();

        //! Creates a platform specific timer object.
        static std::unique_ptr<Timer> Create();

        //! Starts the timer.
        virtual void Start() = 0;

        //! Stops the timer and returns the elapsed time since "Start" was called.
        virtual double Stop() = 0;

        //! Returns the frequency this timer can measure time (e.g. for milliseconds this is 1000.0).
        virtual double GetFrequency() const = 0;

        /**
        \brief Measures the time (elapsed time, and frame count) for each frame.
        \see GetDeltaTime
        \see GetFrameCount()
        */
        void MeasureTime();

        /**
        \brief Restes the frame counter.
        \see GetFrameCount
        */
        void ResetFrameCounter();
        
        /**
        \brief Returns the elapsed time (in seconds) between the current and the previous frame.
        \remarks This requires that "MeasureTime" is called once every frame.
        \see MeasureTime
        */
        double GetDeltaTime() const
        {
            return deltaTime_;
        }

        /**
        \brief Returns the number of counted frames.
        \remarks This requires that "MeasureTime" is called once every frame.
        \see MeasureTime
        */
        FrameCount GetFrameCount() const
        {
            return frameCount_;
        }

    private:
        
        double      deltaTime_  = 0.0;
        FrameCount  frameCount_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
