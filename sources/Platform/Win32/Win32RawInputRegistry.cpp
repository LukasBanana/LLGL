/*
 * Win32RawInputRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32RawInputRegistry.h"
#include "Win32Window.h"
#include "../../Core/CoreUtils.h"

#include <windowsx.h>


namespace LLGL
{

    
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC  ((USHORT)0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif


static void PostGlobalMouseMotion(HWND wnd, LPARAM lParam)
{
    /* Get window object from window handle */
    if (Win32Window* window = Win32Window::GetFromUserData(wnd))
    {
        RAWINPUT raw;
        UINT rawSize = sizeof(raw);

        GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
            &raw, &rawSize, sizeof(RAWINPUTHEADER)
        );

        if (raw.header.dwType == RIM_TYPEMOUSE)
        {
            const RAWMOUSE& mouse = raw.data.mouse;

            if (mouse.usFlags == MOUSE_MOVE_RELATIVE)
            {
                /* Post global mouse motion event */
                int dx = mouse.lLastX;
                int dy = mouse.lLastY;

                window->PostGlobalMotion({ dx, dy });
            }
        }
    }
}

Win32RawInputRegistry& Win32RawInputRegistry::Get()
{
    static Win32RawInputRegistry instance;
    return instance;
}

void Win32RawInputRegistry::Register(HWND wnd)
{
    if (activeWndForInputDevices_ == nullptr)
        RegisterWindowForInputDevices(wnd);
    wndHandles_.push_back(wnd);
}

void Win32RawInputRegistry::Unregister(HWND wnd)
{
    RemoveFromList(wndHandles_, wnd);
    if (wnd == activeWndForInputDevices_)
    {
        if (wndHandles_.empty())
            activeWndForInputDevices_ = nullptr;
        else
            RegisterWindowForInputDevices(wndHandles_.front());
    }
}

void Win32RawInputRegistry::Post(LPARAM lParam)
{
    for (HWND wnd : wndHandles_)
        PostGlobalMouseMotion(wnd, lParam);
}


/*
 * ======= Private: =======
 */

void Win32RawInputRegistry::RegisterWindowForInputDevices(HWND wnd)
{
    /* Register raw input device to capture high-resolution mouse motion events */
    RAWINPUTDEVICE device;
    {
        device.usUsagePage  = HID_USAGE_PAGE_GENERIC;
        device.usUsage      = HID_USAGE_GENERIC_MOUSE;
        device.dwFlags      = RIDEV_INPUTSINK;
        device.hwndTarget   = wnd;
    }
    RegisterRawInputDevices(&device, 1, sizeof(device));

    activeWndForInputDevices_ = wnd;

}


} // /namespace LLGL



// ================================================================================
