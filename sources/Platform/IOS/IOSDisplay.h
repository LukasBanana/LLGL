/*
 * IOSDisplay.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        IOSDisplay() = default;

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
