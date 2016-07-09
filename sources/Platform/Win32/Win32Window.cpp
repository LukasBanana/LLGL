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

void Win32Window::SetPosition(int x, int y)
{
    SetWindowPos(wnd_, HWND_TOP, x, y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER));
}

void Win32Window::GetPosition(int& x, int& y) const
{
    RECT rc;
    GetWindowRect(wnd_, &rc);
    x = rc.left;
    y = rc.top;
}

void Win32Window::SetSize(int width, int height, bool useClientArea)
{
    if (useClientArea)
    {
        auto rc = GetClientArea(width, height, GetWindowStyle(desc_));
        SetSize(rc.right - rc.left, rc.bottom - rc.top, false);
    }
    else
        SetWindowPos(wnd_, HWND_TOP, 0, 0, width, height, (SWP_NOMOVE | SWP_NOZORDER));
}

void Win32Window::GetSize(int& width, int& height, bool useClientArea) const
{
    if (useClientArea)
    {
        RECT rc;
        GetClientRect(wnd_, &rc);
        width = rc.right - rc.left;
        height = rc.bottom - rc.top;
    }
    else
    {
        RECT rc;
        GetWindowRect(wnd_, &rc);
        width = rc.right - rc.left;
        height = rc.bottom - rc.top;
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

bool Win32Window::ProcessEvents()
{
    /* Peek all queued messages */
    MSG message;

    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return (!quit_);
}

void Win32Window::PostQuit()
{
    quit_ = true;
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

    int x = desc.x, y = desc.y;

    if (desc.centered)
    {
        x = GetSystemMetrics(SM_CXSCREEN)/2 - desc.width/2;
        y = GetSystemMetrics(SM_CYSCREEN)/2 - desc.width/2;
    }

    /* Create frame window object */
    HWND wnd = CreateWindow(
        windowClass->GetName(),
        desc.title.c_str(),
        style,
        x - xOffset,
        y - yOffset,
        desc.width + wOffset,
        desc.height + hOffset,
        HWND_DESKTOP,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!wnd)
        throw std::runtime_error("creating Win32 window failed");

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
