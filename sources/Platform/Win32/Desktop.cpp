/*
 * Desktop.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Desktop.h>
#include <LLGL/Platform/Platform.h>
#include <Windows.h>


namespace LLGL
{

namespace Desktop
{


LLGL_EXPORT Size GetResolution()
{
    return { GetSystemMetrics(SM_CXSCREEN),
             GetSystemMetrics(SM_CYSCREEN) };
}

LLGL_EXPORT int GetColorDepth()
{
    #ifdef LLGL_ARCH_ARM

    return 24;

    #else

    HWND wnd    = GetDesktopWindow();
    HDC dc      = GetDC(wnd);

    int bpp     = GetDeviceCaps(dc, BITSPIXEL);
    int planes  = GetDeviceCaps(dc, PLANES);

    if (planes > 1)
        bpp = (1 << planes);

    return bpp;

    #endif
}

LLGL_EXPORT bool SetVideoMode(const VideoModeDescriptor& videoMode)
{
    static bool prevFullscreen = false;

    LONG result = 0;

    if (videoMode.fullscreen)
    {
        DEVMODE config;
        memset(&config, 0, sizeof(DEVMODE));
        {
            config.dmSize       = sizeof(DEVMODE);
            config.dmPelsWidth  = static_cast<DWORD>(videoMode.resolution.x);
            config.dmPelsHeight = static_cast<DWORD>(videoMode.resolution.y);
            config.dmBitsPerPel = videoMode.colorDepth;
            config.dmFields     = (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT);
        }
        result = ChangeDisplaySettings(&config, CDS_FULLSCREEN);
    }
    else if (prevFullscreen)
        result = ChangeDisplaySettings(nullptr, 0);

    prevFullscreen = videoMode.fullscreen;

    return (result == DISP_CHANGE_SUCCESSFUL);
}

LLGL_EXPORT bool ResetVideoMode()
{
    return (ChangeDisplaySettings(nullptr, 0) == DISP_CHANGE_SUCCESSFUL);
}

LLGL_EXPORT void ShowCursor(bool show)
{
    if (IsCursorShown())
    {
        if (!show)
            ::ShowCursor(FALSE);
    }
    else
    {
        if (show)
            ::ShowCursor(TRUE);
    }
}

LLGL_EXPORT bool IsCursorShown()
{
    CURSORINFO info;
    if (::GetCursorInfo(&info))
        return ((info.flags & CURSOR_SHOWING) != 0);
    else
        return false;
}


} // /namespace Desktop

} // /namespace LLGL



// ================================================================================
