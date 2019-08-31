/*
 * Win32Display.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        std::wstring GetDeviceName() const override;

        Offset2D GetOffset() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) override;
        DisplayModeDescriptor GetDisplayMode() const override;

        std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const override;

    private:

        void GetInfo(MONITORINFO& info) const;
        void GetInfo(MONITORINFOEX& info) const;

    private:

        HMONITOR monitor_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
