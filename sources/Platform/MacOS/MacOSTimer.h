/*
 * MacOSTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_WINDOW_H
#define LLGL_MACOS_WINDOW_H


#include <LLGL/Timer.h>
#include <cstdint>
#include <mach/mach.h>
#include <mach/mach_time.h>


namespace LLGL
{
    
    
class MacOSTimer : public Timer
{
    
    public:
        
        MacOSTimer();
        
        void Start() override;
        
        double Stop() override;
        
        double GetFrequency() const override;
        
    private:
    
        std::uint64_t               startTime_      = 0;
        mach_timebase_info_data_t   timebaseInfo_;
    
};
    
    
} // /namespace LLGL


#endif



// ================================================================================
