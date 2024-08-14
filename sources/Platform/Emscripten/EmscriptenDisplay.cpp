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
    return nullptr;
}

Display* Display::Get(std::size_t index)
{
    return nullptr;
}

Display* Display::GetPrimary()
{
    return nullptr;
}

bool Display::ShowCursor(bool show)
{
    return false;
}

bool Display::IsCursorShown()
{
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
    return UTF8String{ "device name" };
}

Offset2D EmscriptenDisplay::GetOffset() const
{
    return Offset2D{};
}

float EmscriptenDisplay::GetScale() const
{
    return 1.0f; // dummy
}

bool EmscriptenDisplay::ResetDisplayMode()
{
    return false;
}

bool EmscriptenDisplay::SetDisplayMode(const DisplayMode& displayMode)
{
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


} // /namespace LLGL



// ================================================================================
