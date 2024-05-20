/*
 * UWPDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "UWPDisplay.h"
#include "../../Core/CoreUtils.h"
#include <algorithm>


namespace LLGL
{


/*
 * Display class
 */

std::size_t Display::Count()
{
    return 0; //todo
}

Display* const * Display::GetList()
{
    return nullptr; //todo
}

Display* Display::Get(std::size_t index)
{
    return nullptr; //todo
}

Display* Display::GetPrimary()
{
    return nullptr; //todo
}

bool Display::ShowCursor(bool show)
{
    return false; //todo
}

bool Display::IsCursorShown()
{
    return true; //todo
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    return false; //todo
}

Offset2D Display::GetCursorPosition()
{
    return { 0, 0 }; //todo
}


/*
 * UWPDisplay class
 */

UWPDisplay::UWPDisplay()
{
}

bool UWPDisplay::IsPrimary() const
{
    return true; //todo
}

UTF8String UWPDisplay::GetDeviceName() const
{
    return ""; //todo
}

Offset2D UWPDisplay::GetOffset() const
{
    return {}; //todo
}

float UWPDisplay::GetScale() const
{
    return 1.0f; //todo
}

bool UWPDisplay::ResetDisplayMode()
{
    return false; //todo
}

bool UWPDisplay::SetDisplayMode(const DisplayMode& displayMode)
{
    return false; //todo
}

DisplayMode UWPDisplay::GetDisplayMode() const
{
    return {}; //todo
}

std::vector<DisplayMode> UWPDisplay::GetSupportedDisplayModes() const
{
    return {}; //todo
}


} // /namespace LLGL



// ================================================================================
