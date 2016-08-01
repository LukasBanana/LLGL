/*
 * Win32Window.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32Window.h"
#include "Win32WindowClass.h"


namespace LLGL
{


// Queries window rectangular area by the specified client area size and window style.
static RECT GetClientArea(LONG width, LONG height, DWORD style)
{
    RECT rc;
    {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = width;
        rc.bottom   = height;
    }
    AdjustWindowRect(&rc, style, FALSE);
    return rc;
}

static DWORD GetWindowStyle(const WindowDesc& desc)
{
    DWORD style = (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    
    if (desc.borderless)
        style |= WS_POPUP;
    else
        style |= (WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);

    if (desc.visible)
        style |= WS_VISIBLE;

    if (!desc.borderless)
    {
        if (desc.acceptDropFiles)
            style |= WM_DROPFILES;
        if (desc.resizable)
            style |= (WS_SIZEBOX | WS_MAXIMIZEBOX);
    }

    return style;
}

std::unique_ptr<Window> Window::Create(const WindowDesc& desc)
{
    return std::unique_ptr<Window>(new Win32Window(desc));
}

Win32Window::Win32Window(const WindowDesc& desc) :
    desc_   ( desc                     ),
    wnd_    ( CreateWindowHandle(desc) ),
    dc_     ( GetDC(wnd_)              )
{
    SetUserData(this);
}

Win32Window::~Win32Window()
{
    if (wnd_)
        DestroyWindow(wnd_);
}

void Win32Window::SetPosition(const Point& position)
{
    SetWindowPos(wnd_, HWND_TOP, position.x, position.y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER));
}

Point Win32Window::GetPosition() const
{
    RECT rc;
    GetWindowRect(wnd_, &rc);
    return { rc.left, rc.top };
}

void Win32Window::SetSize(const Size& size, bool useClientArea)
{
    if (useClientArea)
    {
        auto rc = GetClientArea(size.x, size.y, GetWindowStyle(desc_));
        SetSize({ rc.right - rc.left, rc.bottom - rc.top }, false);
    }
    else
        SetWindowPos(wnd_, HWND_TOP, 0, 0, size.x, size.y, (SWP_NOMOVE | SWP_NOZORDER));
}

Size Win32Window::GetSize(bool useClientArea) const
{
    if (useClientArea)
    {
        RECT rc;
        GetClientRect(wnd_, &rc);
        return { rc.right - rc.left, rc.bottom - rc.top };
    }
    else
    {
        RECT rc;
        GetWindowRect(wnd_, &rc);
        return { rc.right - rc.left, rc.bottom - rc.top };
    }
}

void Win32Window::SetTitle(const std::wstring& title)
{
    SetWindowText(wnd_, title.c_str());
}

std::wstring Win32Window::GetTitle() const
{
    wchar_t title[MAX_PATH];
    GetWindowText(wnd_, title, MAX_PATH);
    return std::wstring(title);
}

void Win32Window::Show(bool show)
{
    ShowWindow(wnd_, (show ? SW_NORMAL : SW_HIDE));
}

bool Win32Window::IsShown() const
{
    return (IsWindowVisible(wnd_) ? true : false);
}

const void* Win32Window::GetNativeHandle() const
{
    return (&wnd_);
}

void Win32Window::ProcessSystemEvents()
{
    /* Peek all queued messages */
    MSG message;

    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}


/*
 * ======= Private: =======
 */

HWND Win32Window::CreateWindowHandle(const WindowDesc& desc)
{
    auto windowClass = Win32WindowClass::Instance();

    /* Get final window size */
    auto style = GetWindowStyle(desc);
    auto rc = GetClientArea(0, 0, style);

    int xOffset = -rc.left;
    int yOffset = -rc.top;
    int wOffset = rc.right - rc.left;
    int hOffset = rc.bottom - rc.top;

    auto position = desc.position;

    if (desc.centered)
    {
        position.x = GetSystemMetrics(SM_CXSCREEN)/2 - desc.size.x/2;
        position.y = GetSystemMetrics(SM_CYSCREEN)/2 - desc.size.y/2;
    }

    /* Create frame window object */
    HWND wnd = CreateWindow(
        windowClass->GetName(),
        desc.title.c_str(),
        style,
        position.x - xOffset,
        position.y - yOffset,
        desc.size.x + wOffset,
        desc.size.y + hOffset,
        HWND_DESKTOP,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!wnd)
        throw std::runtime_error("failed to create window");

    /* Set additional flags */
    if (desc.acceptDropFiles)
        DragAcceptFiles(wnd, TRUE);

    return wnd;
}

void Win32Window::SetUserData(void* userData)
{
    SetWindowLongPtr(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userData));
}


} // /namespace LLGL



// ================================================================================
