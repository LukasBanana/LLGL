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


std::vector<std::unique_ptr<Display>> Display::InstantiateList()
{
    std::vector<std::unique_ptr<Display>> displayList;
    displayList.push_back(Display::InstantiatePrimary());
    return displayList;
}

std::unique_ptr<Display> Display::InstantiatePrimary()
{
    return MakeUnique<AndroidDisplay>();
}

bool Display::ShowCursor(bool show)
{
    return false;
}

bool Display::IsCursorShown()
{
    return false;
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
