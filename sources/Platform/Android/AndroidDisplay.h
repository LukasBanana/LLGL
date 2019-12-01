/*
 * AndroidDisplay.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_DISPLAY_H
#define LLGL_ANDROID_DISPLAY_H


#include <LLGL/Display.h>


namespace LLGL
{


class AndroidDisplay : public Display
{

    public:

        AndroidDisplay() = default;

        bool IsPrimary() const override;

        std::wstring GetDeviceName() const override;

        Offset2D GetOffset() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) override;
        DisplayModeDescriptor GetDisplayMode() const override;

        std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const override;

};


} // /namespace LLGL


#endif



// ================================================================================
