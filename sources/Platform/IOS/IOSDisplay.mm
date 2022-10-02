/*
 * IOSDisplay.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "IOSDisplay.h"
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
    static std::unique_ptr<Display> primaryDisplay = MakeUnique<IOSDisplay>();
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
 * IOSDisplay class
 */

bool IOSDisplay::IsPrimary() const
{
    return true;
}

std::wstring IOSDisplay::GetDeviceName() const
{
    //TODO
    return L"";
}

Offset2D IOSDisplay::GetOffset() const
{
    //TODO
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    return Offset2D
    {
        static_cast<std::int32_t>(screenRect.origin.x),
        static_cast<std::int32_t>(screenRect.origin.y)
    };
}

bool IOSDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool IOSDisplay::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    //TODO
    return false;
}

DisplayModeDescriptor IOSDisplay::GetDisplayMode() const
{
    DisplayModeDescriptor displayModeDesc;
    {
        /* Get attributes from main screen */
        UIScreen*   mainScreen  = [UIScreen mainScreen];

        CGRect      screenRect  = [mainScreen nativeBounds];
        CGFloat     screenScale = [mainScreen nativeScale];
        NSInteger   maxFPS      = [mainScreen maximumFramesPerSecond];

        displayModeDesc.resolution.width    = static_cast<std::uint32_t>(screenRect.size.width * screenScale);
        displayModeDesc.resolution.height   = static_cast<std::uint32_t>(screenRect.size.height * screenScale);
        displayModeDesc.refreshRate         = static_cast<std::uint32_t>(maxFPS);
    }
    return displayModeDesc;
}

std::vector<DisplayModeDescriptor> IOSDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    //TODO
    displayModeDescs.push_back(GetDisplayMode());

    return displayModeDescs;
}


} // /namespace LLGL



// ================================================================================
