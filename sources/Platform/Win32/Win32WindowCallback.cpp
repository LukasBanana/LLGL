/*
 * Win32WindowCallback.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32WindowCallback.h"
#include "Win32Window.h"

#include <windowsx.h>

#ifndef HID_USAGE_PAGE_GENERIC
#   define HID_USAGE_PAGE_GENERIC   ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#   define HID_USAGE_GENERIC_MOUSE  ((USHORT)0x02)
#endif


namespace LLGL
{


static Win32Window* GetWindowFromUserData(HWND wnd)
{
    return reinterpret_cast<Win32Window*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
}


LRESULT CALLBACK Win32WindowCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
        {
            auto window = GetWindowFromUserData(wnd);
            if (window)
                window->PostQuit();
        }
        break;


    }
    return DefWindowProc(wnd, msg, wParam, lParam);
}


} // /namespace LLGL



// ================================================================================
