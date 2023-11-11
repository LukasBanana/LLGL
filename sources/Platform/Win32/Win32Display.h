/*
 * Win32Display.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_DISPLAY_H
#define LLGL_WIN32_DISPLAY_H


#include <LLGL/Display.h>
#include "Win32LeanAndMean.h"
#include <Windows.h>


namespace LLGL
{


class Win32Display final : public Display
{

    public:

        Win32Display(HMONITOR monitor);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

    public:

        // Returns the native display handle as HMONITOR.
        HMONITOR GetNative() const
        {
            return monitor_;
        }

    private:

        void GetInfo(MONITORINFO& info) const;
        void GetInfo(MONITORINFOEX& info) const;

    private:

        HMONITOR monitor_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
