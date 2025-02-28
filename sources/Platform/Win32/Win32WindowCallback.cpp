/*
 * Win32WindowCallback.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32WindowCallback.h"
#include "Win32Window.h"
#include "Win32RawInputRegistry.h"
#include "MapKey.h"
#include <atomic>

#include <windowsx.h>


namespace LLGL
{


static constexpr UINT_PTR g_win32UpdateTimerID = 1;

static void PostKeyEvent(Window& window, Key keyCode, bool isDown)
{
    if (isDown)
        window.PostKeyDown(keyCode);
    else
        window.PostKeyUp(keyCode);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms646280(v=vs.85).aspx
static void PostKeyEvent(HWND wnd, WPARAM wParam, LPARAM lParam, bool isDown, bool /*isSysKey*/)
{
    /* Get window object from window handle */
    if (Win32Window* window = Win32Window::GetFromUserData(wnd))
    {
        /* Extract key code */
        const DWORD keyCodeSys      = static_cast<DWORD>(wParam);
        const DWORD keyCodeOEM      = static_cast<DWORD>(lParam & (0xff << 16)) >> 16;
        const bool  isExtendedKey   = ((lParam & (1 << 24)) != 0);

        /* Get key code mapping first */
        const Key keyCode = MapKey(static_cast<BYTE>(keyCodeSys));

        /* Check for extended keys */
        switch (keyCode)
        {
            case Key::Shift:
            {
                if (keyCodeOEM == 0x36)
                    PostKeyEvent(*window, Key::RShift, isDown);
                else if (keyCodeOEM == 0x2a)
                    PostKeyEvent(*window, Key::LShift, isDown);
            }
            break;

            case Key::Control:
            {
                if (isExtendedKey)
                    PostKeyEvent(*window, Key::RControl, isDown);
                else
                    PostKeyEvent(*window, Key::LControl, isDown);
            }
            break;

            default:
            break;
        }

        /* Post base key event */
        PostKeyEvent(*window, keyCode, isDown);
    }
}

static std::atomic_int g_mouseCaptureCounter{ 0 };

static void ReleaseMouseCapture()
{
    if (g_mouseCaptureCounter > 0)
    {
        g_mouseCaptureCounter = 0;
        ReleaseCapture();
    }
}

static void CaptureMouseButton(HWND wnd, Key keyCode, bool isDoubleClick = false)
{
    /* Get window object from window handle */
    if (Win32Window* window = Win32Window::GetFromUserData(wnd))
    {
        /* Post key events and capture mouse */
        window->PostKeyDown(keyCode);

        if (isDoubleClick)
            window->PostDoubleClick(keyCode);

        if (++g_mouseCaptureCounter == 1)
            SetCapture(wnd);
    }
}

static void ReleaseMouseButton(HWND wnd, Key keyCode)
{
    /* Get window object from window handle */
    if (Win32Window* window = Win32Window::GetFromUserData(wnd))
    {
        /* Post key event and release mouse capture */
        window->PostKeyUp(keyCode);

        if (--g_mouseCaptureCounter == 0)
            ReleaseCapture();

        if (g_mouseCaptureCounter < 0)
        {
            //TODO: this condition should never be true!!!
            g_mouseCaptureCounter = 0;
        }
    }
}

static void PostLocalMouseMotion(HWND wnd, LPARAM lParam)
{
    /* Get window object from window handle */
    if (Win32Window* window = Win32Window::GetFromUserData(wnd))
    {
        /* Extract mouse position from event parameter */
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        /* Post local mouse motion event */
        window->PostLocalMotion({ x, y });
    }
}


/* --- Window callback function --- */

LRESULT CALLBACK Win32WindowCallback(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        /* --- Common window events --- */

        case WM_CREATE:
        {
            Win32RawInputRegistry::Get().Register(wnd);
        }
        break;

        case WM_DESTROY:
        {
            Win32RawInputRegistry::Get().Unregister(wnd);
        }
        break;

        case WM_SIZE:
        {
            /* Post resize event to window */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
            {
                WORD width = LOWORD(lParam);
                WORD height = HIWORD(lParam);
                window->PostResize(Extent2D{ width, height });
            }
        }
        break;

        case WM_CLOSE:
        {
            /* Post close event to window */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                window->PostQuit();
        }
        break;

        case WM_SETFOCUS:
        {
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                window->PostGetFocus();
        }
        break;

        case WM_KILLFOCUS:
        {
            ReleaseMouseCapture();
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                window->PostLostFocus();
        }
        break;

        /* --- Keyboard events --- */

        case WM_KEYDOWN:
        {
            PostKeyEvent(wnd, wParam, lParam, true, false);
        }
        return 0;

        case WM_KEYUP:
        {
            PostKeyEvent(wnd, wParam, lParam, false, false);
        }
        return 0;

        case WM_SYSKEYDOWN:
        {
            PostKeyEvent(wnd, wParam, lParam, true, true);
        }
        return 0;

        case WM_SYSKEYUP:
        {
            PostKeyEvent(wnd, wParam, lParam, false, true);
        }
        return 0;

        case WM_CHAR:
        {
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                window->PostChar(static_cast<wchar_t>(wParam));
        }
        return 0;

        /* --- Left mouse button events --- */

        case WM_LBUTTONDOWN:
        {
            CaptureMouseButton(wnd, Key::LButton);
        }
        return 0;

        case WM_LBUTTONUP:
        {
            ReleaseMouseButton(wnd, Key::LButton);
        }
        return 0;

        case WM_LBUTTONDBLCLK:
        {
            CaptureMouseButton(wnd, Key::LButton, true);
        }
        return 0;

        /* --- Right mouse button events --- */

        case WM_RBUTTONDOWN:
        {
            CaptureMouseButton(wnd, Key::RButton);
        }
        return 0;

        case WM_RBUTTONUP:
        {
            ReleaseMouseButton(wnd, Key::RButton);
        }
        return 0;

        case WM_RBUTTONDBLCLK:
        {
            CaptureMouseButton(wnd, Key::RButton, true);
        }
        return 0;

        /* --- Middle mouse button events --- */

        case WM_MBUTTONDOWN:
        {
            CaptureMouseButton(wnd, Key::MButton);
        }
        return 0;

        case WM_MBUTTONUP:
        {
            ReleaseMouseButton(wnd, Key::MButton);
        }
        return 0;

        case WM_MBUTTONDBLCLK:
        {
            CaptureMouseButton(wnd, Key::MButton, true);
        }
        return 0;

        /* --- Mouse motion events --- */

        case WM_MOUSEWHEEL:
        {
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                window->PostWheelMotion(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
        }
        return 0;

        case WM_MOUSEMOVE:
        {
            PostLocalMouseMotion(wnd, lParam);
        }
        return 0;

        case WM_INPUT:
        {
            Win32RawInputRegistry::Get().Post(lParam);
        }
        return 0;

        /* --- Misc events --- */

        case WM_ERASEBKGND:
        {
            /* Do not erase background to avoid flickering when user resizes the window */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
            {
                if (window->SkipMsgERASEBKGND())
                    return 1;
            }
        }
        break;

        case WM_ENTERSIZEMOVE:
        {
            /* Start timer to receive updates during moving and resizing the window */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                SetTimer(wnd, g_win32UpdateTimerID, USER_TIMER_MINIMUM, nullptr);
        }
        break;

        case WM_EXITSIZEMOVE:
        {
            /* Stop previously started timer */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
                KillTimer(wnd, g_win32UpdateTimerID);
        }
        break;

        case WM_TIMER:
        {
            /* Post update so client can redraw the window during moving/resizing a window */
            if (Win32Window* window = Win32Window::GetFromUserData(wnd))
            {
                const UINT_PTR timerID = static_cast<UINT_PTR>(wParam);
                if (timerID == g_win32UpdateTimerID)
                    window->PostUpdate();
            }
        };
        break;
    }

    return DefWindowProc(wnd, msg, wParam, lParam);
}


} // /namespace LLGL



// ================================================================================
