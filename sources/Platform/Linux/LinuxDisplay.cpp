/*
 * LinuxDisplay.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxDisplay.h"
#include "../../Core/Helper.h"
#include <locale>
#include <codecvt>
#include <X11/extensions/xf86vmode.h>


namespace LLGL
{


/*
 * LinuxSharedX11Display class
 */

LinuxSharedX11Display::LinuxSharedX11Display() :
    native_ { XOpenDisplay(nullptr) }
{
    if (!native_)
        throw std::runtime_error("failed to open connection to X server");
}
 
LinuxSharedX11Display::~LinuxSharedX11Display()
{
    XCloseDisplay(native_);
}


/*
 * Display class
 */

std::vector<std::unique_ptr<Display>> Display::QueryList()
{
    std::vector<std::unique_ptr<Display>> displayList;
    
    /* Allocate shared X11 display handler */
    auto sharedX11Display   = std::make_shared<LinuxSharedX11Display>();
    auto dpy                = sharedX11Display->GetNative();
    
    for (int i = 0, n = ScreenCount(dpy); i < n; ++i)
    {
        /* Make new display with current screen index */
        displayList.emplace_back(MakeUnique<LinuxDisplay>(sharedX11Display, i));
    }

    return displayList;
}

std::unique_ptr<Display> Display::QueryPrimary()
{
    /* Allocate single X11 display handler */
    auto sharedX11Display   = std::make_shared<LinuxSharedX11Display>();
    auto dpy                = sharedX11Display->GetNative();
    
    /* Make new display with default screen index */
    return MakeUnique<LinuxDisplay>(sharedX11Display, DefaultScreen(dpy));
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


/*
 * LinuxDisplay class
 */

LinuxDisplay::LinuxDisplay(const std::shared_ptr<LinuxSharedX11Display>& sharedX11Display, int screenIndex) :
    sharedX11Display_ { sharedX11Display },
    screen_           { screenIndex      }
{
}

bool LinuxDisplay::IsPrimary() const
{
    return (screen_ == DefaultScreen(GetNative()));
}

std::wstring LinuxDisplay::GetDeviceName() const
{
    const char* name = DisplayString(GetNative());
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(name);
}

Offset2D LinuxDisplay::GetOffset() const
{
    //TODO
    return {};
}

bool LinuxDisplay::ResetDisplayMode()
{
    //TODO
    return false;
}

bool LinuxDisplay::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    //TODO
    return false;
}

DisplayModeDescriptor LinuxDisplay::GetDisplayMode() const
{
    DisplayModeDescriptor displayModeDesc;
    {
        if (auto scr = ScreenOfDisplay(GetNative(), screen_))
        {
            displayModeDesc.resolution.width    = static_cast<std::uint32_t>(scr->width);
            displayModeDesc.resolution.height   = static_cast<std::uint32_t>(scr->height);
        }
    }
    return displayModeDesc;
}

std::vector<DisplayModeDescriptor> LinuxDisplay::QuerySupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    /* Get all video modes from X11 extension xf86 */
    int numModes = 0;
    XF86VidModeModeInfo** vidModes;
    
    if (XF86VidModeGetAllModeLines(GetNative(), screen_, &numModes, &vidModes))
    {
        for (int i = 0; i < numModes; ++i)
        {
            DisplayModeDescriptor modeDesc;
            {
                modeDesc.resolution.width   = static_cast<std::uint32_t>(vidModes[i]->hdisplay);
                modeDesc.resolution.height  = static_cast<std::uint32_t>(vidModes[i]->vdisplay);
                modeDesc.refreshRate        = static_cast<std::uint32_t>(vidModes[i]->hsyncend - vidModes[i]->hsyncstart);
            }
            displayModeDescs.push_back(modeDesc);
        }
        XFree(vidModes);
    }

    /* Sort final display mode list and remove duplciate entries */
    FinalizeDisplayModes(displayModeDescs);

    return displayModeDescs;
}


/*
 * ======= Private: =======
 */

::Display* LinuxDisplay::GetNative() const
{
    return sharedX11Display_->GetNative();
}
 

} // /namespace LLGL



// ================================================================================
