/*
 * AndroidDisplay.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AndroidDisplay.h"
#include "../../Core/Helper.h"


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

std::wstring AndroidDisplay::GetDeviceName() const
{
    //TODO
    return L"";
}

Offset2D AndroidDisplay::GetOffset() const
{
    //TODO
    return Offset2D{};
}

bool AndroidDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool AndroidDisplay::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    //TODO
    return false;
}

DisplayModeDescriptor AndroidDisplay::GetDisplayMode() const
{
    DisplayModeDescriptor displayModeDesc;
    {
        //TODO
    }
    return displayModeDesc;
}

std::vector<DisplayModeDescriptor> AndroidDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    //TODO
    displayModeDescs.push_back(GetDisplayMode());

    return displayModeDescs;
}


} // /namespace LLGL



// ================================================================================
