/*
 * IOSDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_DISPLAY_H
#define LLGL_IOS_DISPLAY_H


#import <UIKit/UIKit.h>

#include <LLGL/Display.h>


namespace LLGL
{


class IOSDisplay : public Display
{

    public:

        IOSDisplay(UIScreen* screen);

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) override;
        DisplayModeDescriptor GetDisplayMode() const override;

        std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const override;

    private:

        UIScreen* screen_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
