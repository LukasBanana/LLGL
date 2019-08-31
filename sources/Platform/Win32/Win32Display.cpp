/*
 * Win32Display.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32Display.h"
#include "../../Core/Helper.h"
#include <locale>
#include <codecvt>


namespace LLGL
{


/*
 * Internals
 */

// Thread local reference to the output list of the Display::InstantiateList function
thread_local static std::vector<std::unique_ptr<Display>>* g_displayListRef;

static void Convert(DisplayModeDescriptor& dst, const DEVMODE& src)
{
    dst.resolution.width    = static_cast<std::uint32_t>(src.dmPelsWidth);
    dst.resolution.height   = static_cast<std::uint32_t>(src.dmPelsHeight);
    dst.refreshRate         = static_cast<std::uint32_t>(src.dmDisplayFrequency);
}

static void Convert(DEVMODE& dst, const DisplayModeDescriptor& src)
{
    dst.dmPelsWidth         = static_cast<DWORD>(src.resolution.width);
    dst.dmPelsHeight        = static_cast<DWORD>(src.resolution.height);
    dst.dmDisplayFrequency  = static_cast<DWORD>(src.refreshRate);
}

static BOOL CALLBACK Win32MonitorEnumProc(HMONITOR monitor, HDC hDC, LPRECT rect, LPARAM data)
{
    if (g_displayListRef)
    {
        g_displayListRef->push_back(MakeUnique<Win32Display>(monitor));
        return TRUE;
    }
    return FALSE;
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

std::vector<std::unique_ptr<Display>> Display::InstantiateList()
{
    std::vector<std::unique_ptr<Display>> displayList;

    g_displayListRef = (&displayList);
    {
        EnumDisplayMonitors(nullptr, nullptr, Win32MonitorEnumProc, 0);
    }
    g_displayListRef = nullptr;

    return displayList;
}

std::unique_ptr<Display> Display::InstantiatePrimary()
{
    auto monitor = MonitorFromPoint({}, MONITOR_DEFAULTTOPRIMARY);
    return MakeUnique<Win32Display>(monitor);
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

std::wstring Win32Display::GetDeviceName() const
{
    MONITORINFOEX infoEx;
    GetInfo(infoEx);
    #ifdef UNICODE
    return std::wstring(infoEx.szDevice);
    #else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(infoEx.szDevice);
    #endif
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

bool Win32Display::ResetDisplayMode()
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Change settings for this display to default l*/
    auto result = ChangeDisplaySettingsEx(infoEx.szDevice, nullptr, nullptr, 0, nullptr);

    return (result == DISP_CHANGE_SUCCESSFUL);
}

bool Win32Display::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Change settings for this display */
    DEVMODE devMode = {};
    {
        devMode.dmSize      = sizeof(devMode);
        devMode.dmFields    = (DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY);
        Convert(devMode, displayModeDesc);
    }
    auto result = ChangeDisplaySettingsEx(infoEx.szDevice, &devMode, nullptr, CDS_FULLSCREEN, nullptr);

    return (result == DISP_CHANGE_SUCCESSFUL);
}

DisplayModeDescriptor Win32Display::GetDisplayMode() const
{
    /* Get display device name */
    MONITORINFOEX infoEx;
    GetInfo(infoEx);

    /* Get current display settings */
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);

    if (EnumDisplaySettings(infoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode) != FALSE)
    {
        DisplayModeDescriptor displayModeDesc;
        Convert(displayModeDesc, devMode);
        return displayModeDesc;
    }

    return {};
}

std::vector<DisplayModeDescriptor> Win32Display::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

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
            DisplayModeDescriptor outputDesc;
            Convert(outputDesc, devMode);
            displayModeDescs.push_back(outputDesc);
        }
    }

    /* Sort final display mode list and remove duplciate entries */
    FinalizeDisplayModes(displayModeDescs);

    return displayModeDescs;
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
