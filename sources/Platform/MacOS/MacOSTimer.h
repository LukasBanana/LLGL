/*
 * MacOSTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_WINDOW_H
#define LLGL_MACOS_WINDOW_H


#include <LLGL/Timer.h>
#include <mach/mach.h>
#include <mach/mach_time.h>


namespace LLGL
{
    
    
class MacOSTimer : public Timer
{
    
    public:
        
        MacOSTimer();
        
        void Start() override;
        std::uint64_t Stop() override;
        
        std::uint64_t GetFrequency() const override;
        
        bool IsRunning() const override;

    private:
    
        bool                        running_        = false;
        std::uint64_t               startTime_      = 0;
        mach_timebase_info_data_t   timebaseInfo_;

};
    
    
} // /namespace LLGL


#endif



// ================================================================================
