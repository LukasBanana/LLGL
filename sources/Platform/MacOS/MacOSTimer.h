/*
 * MacOSTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_MACOS_WINDOW_H__
#define __LLGL_MACOS_WINDOW_H__


#include <LLGL/Timer.h>


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
    
        //todo...
    
};
    
    
} // /namespace LLGL


#endif



// ================================================================================
