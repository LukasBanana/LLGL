/*
 * WasmDisplay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WasmDisplay.h"
#include "../../Core/CoreUtils.h"
#include <emscripten.h>


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
    static Display* const displayList[] = { Display::GetPrimary(), nullptr };
    return displayList;
}

Display* Display::Get(std::size_t index)
{
    return (index == 0 ? Display::GetPrimary() : nullptr);
}

Display* Display::GetPrimary()
{
    static WasmDisplay primary;
    return &primary;
}

bool Display::ShowCursor(bool /*show*/)
{
    return false;
}

bool Display::IsCursorShown()
{
    return true;
}

bool Display::SetCursorPosition(const Offset2D& /*position*/)
{    
    return false; // dummy
}

Offset2D Display::GetCursorPosition()
{
    Offset2D rootPosition = { 0, 0 };
    //Offset2D childPosition = { 0, 0 };
    return rootPosition;
}


/*
 * WasmDisplay class
 */

bool WasmDisplay::IsPrimary() const
{
    return true;
}

UTF8String WasmDisplay::GetDeviceName() const
{
    return UTF8String{ "device name" };
}

Offset2D WasmDisplay::GetOffset() const
{
    return Offset2D{};
}

float WasmDisplay::GetScale() const
{
    return 1.0f; // dummy
}

bool WasmDisplay::ResetDisplayMode()
{
    return false; // dummy
}

bool WasmDisplay::SetDisplayMode(const DisplayMode& /*displayMode*/)
{
    return false; // dummy
}

DisplayMode WasmDisplay::GetDisplayMode() const
{
    DisplayMode displayMode;
    {
        int width = 0, height = 0;
        emscripten_get_screen_size(&width, &height);
        displayMode.resolution.width    = static_cast<std::uint32_t>(width);
        displayMode.resolution.height   = static_cast<std::uint32_t>(height);
        displayMode.refreshRate         = 60; // default to 60 Hz
    }
    return displayMode;
}

std::vector<DisplayMode> WasmDisplay::GetSupportedDisplayModes() const
{
    return { GetDisplayMode() };
}


} // /namespace LLGL



// ================================================================================
