/*
 * LinuxTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_WINDOW_H
#define LLGL_LINUX_WINDOW_H


#include <LLGL/Timer.h>
#include <time.h>


namespace LLGL
{


class LinuxTimer : public Timer
{

    public:

        LinuxTimer();

        void Start() override;
        std::uint64_t Stop() override;

        std::uint64_t GetFrequency() const override;
        
        bool IsRunning() const override;

    private:
    
        bool        running_    = false;
        timespec    startTime_;

};


} // /namespace LLGL


#endif



// ================================================================================
