/*
 * EmscriptenDisplay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "EmscriptenDisplay.h"
#include "../../Core/CoreUtils.h"


namespace LLGL
{


/*
 * Display class
 */

std::size_t Display::Count()
{
    return 0;
}

Display* const * Display::GetList()
{
    /*
    if (UpdateDisplayList() || g_displayRefList.empty())
    {
        /* Update reference list and append null terminator to array */
        /*g_displayRefList.clear();
        g_displayRefList.reserve(g_displayList.size() + 1);
        for (const auto& display : g_displayList)
            g_displayRefList.push_back(display.get());
        g_displayRefList.push_back(nullptr);
    }
    return g_displayRefList.data();
    */

    return nullptr;
}

Display* Display::Get(std::size_t index)
{
    //UpdateDisplayList();
    //return (index < g_displayList.size() ? g_displayList[index].get() : nullptr);
    return nullptr;
}

Display* Display::GetPrimary()
{
    //UpdateDisplayList();
    return nullptr;
}

bool Display::ShowCursor(bool show)
{
    //TODO
    return false;
}

bool Display::IsCursorShown()
{
    //TODO
    return true;
}

bool Display::SetCursorPosition(const Offset2D& position)
{    
    return true;
}

Offset2D Display::GetCursorPosition()
{
    Offset2D rootPosition = { 0, 0 };
    //Offset2D childPosition = { 0, 0 };
    return rootPosition;
}


/*
 * EmscriptenDisplay class
 */

EmscriptenDisplay::EmscriptenDisplay(int screenIndex) :
    screen_ { screenIndex }
{
}

bool EmscriptenDisplay::IsPrimary() const
{
    return true;
}

UTF8String EmscriptenDisplay::GetDeviceName() const
{
    return UTF8String{"device name"};
}

Offset2D EmscriptenDisplay::GetOffset() const
{
    /* Get display offset from position of root window */
    return Offset2D
    {
        0, 0
    };
}

float EmscriptenDisplay::GetScale() const
{
    return 1.0f; // dummy
}

bool EmscriptenDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool EmscriptenDisplay::SetDisplayMode(const DisplayMode& displayMode)
{
    /* Get all screen sizes from X11 extension Xrandr */
    int numSizes = 0;

    /*
    XRRScreenSize* scrSizes = XRRSizes(GetNative(), screen_, &numSizes);

    for (int i = 0; i < numSizes; ++i)
    {
        // Check if specified display mode resolution matches this screen configuration
        if (displayMode.resolution.width  == static_cast<std::uint32_t>(scrSizes[i].width) &&
            displayMode.resolution.height == static_cast<std::uint32_t>(scrSizes[i].height))
        {
            if (XRRScreenConfiguration* scrCfg = XRRGetScreenInfo(dpy, rootWnd))
            {
                Status status = XRRSetScreenConfig(dpy, scrCfg, rootWnd, i, RR_Rotate_0, 0);
                XRRFreeScreenConfigInfo(scrCfg);
                return (status != 0);
            }
        }
    }
    */

    return false;
}

DisplayMode EmscriptenDisplay::GetDisplayMode() const
{
    DisplayMode displayMode;

    return displayMode;
}

std::vector<DisplayMode> EmscriptenDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayMode> displayModes;

    DisplayMode displayMode;

    return displayModes;
}


/*
 * ======= Private: =======
 */



} // /namespace LLGL



// ================================================================================
