/*
 * LinuxTimer.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_LINUX_WINDOW_H__
#define __LLGL_LINUX_WINDOW_H__


#include <LLGL/Timer.h>


namespace LLGL
{


class LinuxTimer : public Timer
{

    public:

        LinuxTimer();

        void Start() override;

        double Stop() override;

        double GetFrequency() const override;

};


} // /namespace LLGL


#endif



// ================================================================================
