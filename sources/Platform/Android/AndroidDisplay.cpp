/*
 * AndroidDisplay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidDisplay.h"
#include "AndroidApp.h"
#include "../../Core/CoreUtils.h"


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
    static std::unique_ptr<Display> primaryDisplay = MakeUnique<AndroidDisplay>();
    return primaryDisplay.get();
}

bool Display::ShowCursor(bool show)
{
    return false;
}

bool Display::IsCursorShown()
{
    return false;
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    return false;
}

Offset2D Display::GetCursorPosition()
{
    return { 0, 0 };
}


/*
 * AndroidDisplay class
 */

bool AndroidDisplay::IsPrimary() const
{
    return true;
}

UTF8String AndroidDisplay::GetDeviceName() const
{
    //TODO
    return UTF8String{};
}

Offset2D AndroidDisplay::GetOffset() const
{
    //TODO
    return Offset2D{};
}

float AndroidDisplay::GetScale() const
{
    return 1.0f; // dummy
}

bool AndroidDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool AndroidDisplay::SetDisplayMode(const DisplayMode& displayMode)
{
    //TODO
    return false;
}

static Extent2D GetAndroidWindowExtent()
{
    if (android_app* app = AndroidApp::Get().GetState())
    {
        if (app->window != nullptr)
        {
            const std::int32_t width  = ANativeWindow_getWidth(app->window);
            const std::int32_t height = ANativeWindow_getHeight(app->window);
            if (width > 0 && height > 0)
            {
                return Extent2D
                {
                    static_cast<std::uint32_t>(width),
                    static_cast<std::uint32_t>(height)
                };
            }
        }
    }
    return Extent2D{};
}

DisplayMode AndroidDisplay::GetDisplayMode() const
{
    DisplayMode displayMode;
    {
        displayMode.resolution  = GetAndroidWindowExtent();
        displayMode.refreshRate = 60; //TODO: assume default frame rate
    }
    return displayMode;
}

std::vector<DisplayMode> AndroidDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayMode> displayModes;

    //TODO
    displayModes.push_back(GetDisplayMode());

    return displayModes;
}


} // /namespace LLGL



// ================================================================================
