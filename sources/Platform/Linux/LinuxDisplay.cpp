/*
 * LinuxDisplay.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxDisplay.h"
#include "../../Core/Helper.h"
#include <locale>
#include <codecvt>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/Xrandr.h>


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

std::vector<std::unique_ptr<Display>> Display::InstantiateList()
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

std::unique_ptr<Display> Display::InstantiatePrimary()
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
    /* Get display offset from position of root window */
    XWindowAttributes attribs = {};
    XGetWindowAttributes(GetNative(), RootWindow(GetNative(), screen_), &attribs);
    return Offset2D
    {
        attribs.x, attribs.y
    };
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
    DisplayModeDescriptor modeDesc;

    auto dpy = GetNative();
    if (auto scr = ScreenOfDisplay(dpy, screen_))
    {
        auto rootWnd = RootWindow(dpy, screen_);

        /* Get screen resolution from X11 screen */
        modeDesc.resolution.width   = static_cast<std::uint32_t>(scr->width);
        modeDesc.resolution.height  = static_cast<std::uint32_t>(scr->height);

        /* Get refresh reate from X11 extension Xrandr */
        if (auto scrCfg = XRRGetScreenInfo(dpy, rootWnd))
        {
            modeDesc.refreshRate = static_cast<std::uint32_t>(XRRConfigCurrentRate(scrCfg));
            XRRFreeScreenConfigInfo(scrCfg);
        }
    }

    return modeDesc;
}

std::vector<DisplayModeDescriptor> LinuxDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    DisplayModeDescriptor modeDesc;

    /* Get all screen sizes from X11 extension Xrandr */
    int numSizes = 0;
    auto scrSizes = XRRSizes(GetNative(), screen_, &numSizes);

    for (int i = 0; i < numSizes; ++i)
    {
        /* Initialize resolution */
        modeDesc.resolution.width   = static_cast<std::uint32_t>(scrSizes[i].width);
        modeDesc.resolution.height  = static_cast<std::uint32_t>(scrSizes[i].height);

        /* Add one display mode for each rate */
        int numRates = 0;
        auto rates = XRRRates(GetNative(), screen_, i, &numRates);

        for (int j = 0; j < numRates; ++j)
        {
            modeDesc.refreshRate = static_cast<std::uint32_t>(rates[j]);
            displayModeDescs.push_back(modeDesc);
        }
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
