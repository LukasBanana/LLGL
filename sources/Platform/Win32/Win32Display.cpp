/*
 * Win32Display.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32Display.h"
#include "../../Core/CoreUtils.h"
#include <algorithm>


namespace LLGL
{


/*
 * Internals
 */

struct Win32DisplayContainer
{
    Win32DisplayContainer() = default;
    Win32DisplayContainer(
        std::unique_ptr<Win32Display>&& display,
        int                             cacheIndex)
    :
        display    { std::forward<std::unique_ptr<Win32Display>&&>(display) },
        cacheIndex { cacheIndex                                             }
    {
    }

    std::unique_ptr<Win32Display>   display;
    int                             cacheIndex = 0;
};

static std::vector<Win32DisplayContainer>   g_displayList;
static std::vector<Display*>                g_displayRefList;
static Win32Display*                        g_primaryDisplay;
static int                                  g_displayCacheIndex;

struct MonitorChangedInfo
{
    std::size_t numRegisteredMonitors;
    std::size_t numUnregisteredMonitors;
};

static void Convert(DisplayMode& dst, const DEVMODE& src)
{
    dst.resolution.width    = static_cast<std::uint32_t>(src.dmPelsWidth);
    dst.resolution.height   = static_cast<std::uint32_t>(src.dmPelsHeight);
    dst.refreshRate         = static_cast<std::uint32_t>(src.dmDisplayFrequency);
}

static void Convert(DEVMODE& dst, const DisplayMode& src)
{
    dst.dmPelsWidth         = static_cast<DWORD>(src.resolution.width);
    dst.dmPelsHeight        = static_cast<DWORD>(src.resolution.height);
    dst.dmDisplayFrequency  = static_cast<DWORD>(src.refreshRate);
}

static BOOL CALLBACK Win32MonitorChangedEnumProc(HMONITOR monitor, HDC hDC, LPRECT rect, LPARAM data)
{
    auto& info = *reinterpret_cast<MonitorChangedInfo*>(data);
    auto it = std::find_if(
        g_displayList.begin(), g_displayList.end(),
        [monitor](const Win32DisplayContainer& entry) -> bool
        {
            return (entry.display->GetNative() == monitor);
        }
    );
    if (it != g_displayList.end())
        info.numRegisteredMonitors++;
    else
        info.numUnregisteredMonitors++;
    return TRUE;
}

static bool HasMonitorListChanged()
{
    /* Check if there are any unregistered monitors or if we lost any monitors */
    MonitorChangedInfo info = { 0, 0 };
    EnumDisplayMonitors(nullptr, nullptr, Win32MonitorChangedEnumProc, reinterpret_cast<LPARAM>(&info));
    return (info.numUnregisteredMonitors > 0 || info.numRegisteredMonitors != g_displayList.size());
}

static BOOL CALLBACK Win32MonitorEnumProc(HMONITOR monitor, HDC hDC, LPRECT rect, LPARAM data)
{
    auto it = std::find_if(
        g_displayList.begin(), g_displayList.end(),
        [monitor](const Win32DisplayContainer& entry) -> bool
        {
            return (entry.display->GetNative() == monitor);
        }
    );
    if (it != g_displayList.end())
    {
        /* Update cache index */
        it->cacheIndex = g_displayCacheIndex;
    }
    else
    {
        /* Allocate new display object */
        auto display = MakeUnique<Win32Display>(monitor);
        if (display->IsPrimary())
            g_primaryDisplay = display.get();
        g_displayList.emplace_back(std::move(display), g_displayCacheIndex);
    }
    return TRUE;
}

static bool UpdateDisplayList()
{
    if (HasMonitorListChanged())
    {
        /* Clear primary display */
        g_primaryDisplay = nullptr;

        /* Move to next cache index to determine which display entry is outdated */
        g_displayCacheIndex = (g_displayCacheIndex + 1) % 2;

        /* Gather new monitors */
        EnumDisplayMonitors(nullptr, nullptr, Win32MonitorEnumProc, 0);

        /* Remove outdated entries from the map */
        RemoveAllFromListIf(
            g_displayList,
            [](const Win32DisplayContainer& entry) -> bool
            {
                /* Add entry to reference list if cache index matches, otherwise remove from list */
                return (entry.cacheIndex != g_displayCacheIndex);
            }
        );

        return true;
    }
    return false;
}

static bool IsCursorVisible(bool& visible)
{
    CURSORINFO info;
    info.cbSize = sizeof(CURSORINFO);
    if (::GetCursorInfo(&info))
    {
        visible = ((info.flags & CURSOR_SHOWING) != 0);
        return true;
    }
    return false;
}


/*
 * Display class
 */

std::size_t Display::Count()
{
    UpdateDisplayList();
    return g_displayList.size();
}

Display* const * Display::GetList()
{
    if (UpdateDisplayList() || g_displayRefList.empty())
    {
        /* Update reference list and append null terminator to array */
        g_displayRefList.clear();
        g_displayRefList.reserve(g_displayList.size() + 1);
        for (const auto& entry : g_displayList)
            g_displayRefList.push_back(entry.display.get());
        g_displayRefList.push_back(nullptr);
    }
    return g_displayRefList.data();
}

Display* Display::Get(std::size_t index)
{
    UpdateDisplayList();
    return (index < g_displayList.size() ? g_displayList[index].display.get() : nullptr);
}

Display* Display::GetPrimary()
{
    UpdateDisplayList();
    return g_primaryDisplay;
}

bool Display::ShowCursor(bool show)
{
    bool visible = false;
    if (IsCursorVisible(visible))
    {
        if (visible)
        {
            if (!show)
                ::ShowCursor(FALSE);
        }
        else
        {
            if (show)
                ::ShowCursor(TRUE);
        }
        return true;
    }
    return false;
}

bool Display::IsCursorShown()
{
    bool visible = true;
    IsCursorVisible(visible);
    return visible;
}

bool Display::SetCursorPosition(const Offset2D& position)
{
    return (::SetCursorPos(position.x, position.y) != FALSE);
}

Offset2D Display::GetCursorPosition()
{
    POINT pos;
    if (::GetCursorPos(&pos) != FALSE)
        return { pos.x, pos.y };
    else
        return { 0, 0 };
}


/*
 * Win32Display class
 */

Win32Display::Win32Display(HMONITOR monitor) :
    monitor_ { monitor }
{
}

bool Win32Display::IsPrimary() const
{
    MONITORINFO info;
    GetInfo(info);
    return ((info.dwFlags & MONITORINFOF_PRIMARY) != 0);
}

UTF8String Win32Display::GetDeviceName() const
{
    MONITORINFOEX infoEx;
    GetInfo(infoEx);
    return infoEx.szDevice;
}

Offset2D Win32Display::GetOffset() const
{
    MONITORINFO info;
    GetInfo(info);
    return
    {
        static_cast<std::int32_t>(info.rcMonitor.left),
        static_cast<std::int32_t>(info.rcMonitor.top),
    };
}

float Win32Display::GetScale() const
{
    return 1.0f; // dummy
}

bool Win32Display::ResetDisplayMode()
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Change settings for this display to default l*/
    auto result = ChangeDisplaySettingsEx(infoEx.szDevice, nullptr, nullptr, 0, nullptr);

    return (result == DISP_CHANGE_SUCCESSFUL);
}

bool Win32Display::SetDisplayMode(const DisplayMode& displayMode)
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Change settings for this display */
    DEVMODE devMode = {};
    {
        devMode.dmSize      = sizeof(devMode);
        devMode.dmFields    = (DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY);
        Convert(devMode, displayMode);
    }
    auto result = ChangeDisplaySettingsEx(infoEx.szDevice, &devMode, nullptr, CDS_FULLSCREEN, nullptr);

    return (result == DISP_CHANGE_SUCCESSFUL);
}

DisplayMode Win32Display::GetDisplayMode() const
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Get current display settings */
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);

    if (EnumDisplaySettings(infoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode) != FALSE)
    {
        DisplayMode displayMode;
        Convert(displayMode, devMode);
        return displayMode;
    }

    return {};
}

std::vector<DisplayMode> Win32Display::GetSupportedDisplayModes() const
{
    std::vector<DisplayMode> displayModes;

    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Enumerate all display settings for this display */
    const DWORD fieldBits = (DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY);

    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);

    for (DWORD modeNum = 0; EnumDisplaySettings(infoEx.szDevice, modeNum, &devMode) != FALSE; ++modeNum)
    {
        /* Only enumerate display settings where the width, height, and frequency fields have been initialized */
        if ((devMode.dmFields & fieldBits) == fieldBits)
        {
            DisplayMode displayMode;
            Convert(displayMode, devMode);
            displayModes.push_back(displayMode);
        }
    }

    /* Sort final display mode list and remove duplciate entries */
    FinalizeDisplayModes(displayModes);

    return displayModes;
}


/*
 * ======= Private: =======
 */

void Win32Display::GetInfo(MONITORINFO& info) const
{
    info.cbSize = sizeof(info);
    GetMonitorInfo(monitor_, &info);
}

void Win32Display::GetInfo(MONITORINFOEX& info) const
{
    info.cbSize = sizeof(info);
    GetMonitorInfo(monitor_, &info);
}


} // /namespace LLGL



// ================================================================================
