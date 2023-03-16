/*
 * MacOSDisplay.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_DISPLAY_H
#define LLGL_MACOS_DISPLAY_H


#import <Cocoa/Cocoa.h>

#include <LLGL/Display.h>


namespace LLGL
{


class MacOSDisplay : public Display
{

    public:

        MacOSDisplay(CGDirectDisplayID displayID);
        ~MacOSDisplay();

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) override;
        DisplayModeDescriptor GetDisplayMode() const override;

        std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const override;

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
