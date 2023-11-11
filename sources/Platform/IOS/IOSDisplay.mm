/*
 * IOSDisplay.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "IOSDisplay.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


std::size_t Display::Count()
{
    return 1;
}

Display* const * Display::GetList()
{
    static Display* displayList[] = { GetPrimary(), nullptr };
    return displayList;
}

Display* Display::Get(std::size_t index)
{
    return (index == 0 ? GetPrimary() : nullptr);
}

Display* Display::GetPrimary()
{
    static std::unique_ptr<Display> primaryDisplay = MakeUnique<IOSDisplay>([UIScreen mainScreen]);
    return primaryDisplay.get();
}

bool Display::ShowCursor(bool show)
{
    return false; // dummy
}

bool Display::IsCursorShown()
{
    return false; // dummy
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    return false; // dummy
}

Offset2D Display::GetCursorPosition()
{
    return { 0, 0 }; // dummy
}


/*
 * IOSDisplay class
 */

IOSDisplay::IOSDisplay(UIScreen* screen) :
    screen_ { screen }
{
}

bool IOSDisplay::IsPrimary() const
{
    return (screen_ == [UIScreen mainScreen]);
}

UTF8String IOSDisplay::GetDeviceName() const
{
    //TODO
    return UTF8String{};
}

Offset2D IOSDisplay::GetOffset() const
{
    CGRect screenRect = [screen_ bounds];
    return Offset2D
    {
        static_cast<std::int32_t>(screenRect.origin.x),
        static_cast<std::int32_t>(screenRect.origin.y)
    };
}

float IOSDisplay::GetScale() const
{
    return static_cast<float>([screen_ scale]);
}

bool IOSDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool IOSDisplay::SetDisplayMode(const DisplayMode& displayMode)
{
    //TODO
    return false;
}

static Extent2D GetScaledScreenResolution(CGSize size, CGFloat screenScale = 1.0f)
{
    Extent2D resolution;
    resolution.width    = static_cast<std::uint32_t>(size.width  * screenScale);
    resolution.height   = static_cast<std::uint32_t>(size.height * screenScale);
    return resolution;
}

static void ConvertUIScreenMode(DisplayMode& dst, UIScreen* screen, UIScreenMode* mode)
{
    const NSInteger maxFPS = [screen maximumFramesPerSecond];
    dst.resolution  = GetScaledScreenResolution(mode.size);
    dst.refreshRate = static_cast<std::uint32_t>(maxFPS);
}

DisplayMode IOSDisplay::GetDisplayMode() const
{
    DisplayMode displayMode;
    {
        ConvertUIScreenMode(displayMode, screen_, [screen_ currentMode]);
    }
    return displayMode;
}

std::vector<DisplayMode> IOSDisplay::GetSupportedDisplayModes() const
{
    const NSUInteger numModes = [[screen_ availableModes] count];

    std::vector<DisplayMode> displayModes;
    displayModes.reserve(static_cast<std::size_t>(numModes));

    for_range(i, numModes)
    {
        DisplayMode displayMode;
        {
            ConvertUIScreenMode(displayMode, screen_, [[screen_ availableModes] objectAtIndex:i]);
        }
        displayModes.push_back(displayMode);
    }

    return displayModes;
}


} // /namespace LLGL



// ================================================================================
