/*
 * Win32Timer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_WINDOW_H
#define LLGL_WIN32_WINDOW_H


#include <LLGL/Timer.h>
#include <Windows.h>


namespace LLGL
{


class Win32Timer final : public Timer
{

    public:

        Win32Timer();

        void Start() override;
        std::uint64_t Stop() override;

        std::uint64_t GetFrequency() const override;

        bool IsRunning() const override;

    private:

        LARGE_INTEGER   clockFrequency_;
        LARGE_INTEGER   t0_;
        LARGE_INTEGER   t1_;

        DWORD           startTick_          = 0;
        LONGLONG        prevElapsedTime_    = 0;

        bool            running_            = false;

};


} // /namespace LLGL


#endif



// ================================================================================
