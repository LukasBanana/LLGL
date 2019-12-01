/*
 * AndroidTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_TIMER_H
#define LLGL_ANDROID_TIMER_H


#include <LLGL/Timer.h>


namespace LLGL
{
    
    
class AndroidTimer : public Timer
{
    
    public:
        
        AndroidTimer();
        
        void Start() override;
        
        std::uint64_t Stop() override;
        
        std::uint64_t GetFrequency() const override;

        bool IsRunning() const override;
    
    private:
    
        //todo...
    
};
    
    
} // /namespace LLGL


#endif



// ================================================================================
