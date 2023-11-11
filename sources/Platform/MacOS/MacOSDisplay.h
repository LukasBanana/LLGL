/*
 * MacOSDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_DISPLAY_H
#define LLGL_MACOS_DISPLAY_H


#import <Cocoa/Cocoa.h>

#include <LLGL/Display.h>


namespace LLGL
{


class MacOSDisplay final : public Display
{

    public:

        MacOSDisplay(CGDirectDisplayID displayID);
        ~MacOSDisplay();

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        std::vector<DisplayMode> GetSupportedDisplayModes() const override;

    public:

        // Returns the native display ID.
        inline CGDirectDisplayID GetID() const
        {
            return displayID_;
        }

    private:

        CGDirectDisplayID   displayID_              = 0;
        CGDisplayModeRef    defaultDisplayModeRef_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
