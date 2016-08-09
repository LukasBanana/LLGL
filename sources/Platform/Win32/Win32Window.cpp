/*
 * Win32Window.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32Window.h"
#include "Win32WindowClass.h"
#include <LLGL/Platform/NativeHandle.h>


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

static DWORD GetWindowStyle(const WindowDescriptor& desc)
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

static Point GetScreenCenteredPosition(const Size& size)
{
    return { GetSystemMetrics(SM_CXSCREEN)/2 - size.x/2,
             GetSystemMetrics(SM_CYSCREEN)/2 - size.y/2 };
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new Win32Window(desc));
}

Win32Window::Win32Window(const WindowDescriptor& desc) :
    wnd_( CreateWindowHandle(desc) )
{
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
        auto rc = GetClientArea(size.x, size.y, GetWindowLong(wnd_, GWL_STYLE));
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

WindowDescriptor Win32Window::QueryDesc() const
{
    /* Get window flags and other information for comparision */
    auto windowFlags    = GetWindowLong(wnd_, GWL_STYLE);
    auto windowSize     = GetSize();
    auto centerPoint    = GetScreenCenteredPosition(windowSize);

    /* Setup window descriptor */
    WindowDescriptor desc;

    desc.title                  = GetTitle();
    desc.position               = GetPosition();
    desc.size                   = windowSize;

    desc.visible                = ((windowFlags & WS_VISIBLE  ) != 0);
    desc.borderless             = ((windowFlags & WS_CAPTION  ) == 0);
    desc.resizable              = ((windowFlags & WS_SIZEBOX  ) != 0);
    desc.acceptDropFiles        = ((windowFlags & WM_DROPFILES) != 0);
    desc.preventForPowerSafe    = false; //todo...
    desc.centered               = (centerPoint.x == desc.position.x && centerPoint.y == desc.position.y);

    desc.windowContext          = nullptr; //todo...

    return desc;
}

void Win32Window::Recreate(const WindowDescriptor& desc)
{
    /* Destroy previous window handle and create a new one */
    DestroyWindow(wnd_);
    wnd_ = CreateWindowHandle(desc);
}

void Win32Window::GetNativeHandle(void* nativeHandle) const
{
    auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
    handle.window = wnd_;
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

HWND Win32Window::CreateWindowHandle(const WindowDescriptor& desc)
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
        position = GetScreenCenteredPosition(desc.size);

    /* Get parent window */
    HWND parentWnd = HWND_DESKTOP;

    if (desc.windowContext)
    {
        auto& nativeContext = *reinterpret_cast<const NativeContextHandle*>(desc.windowContext);
        parentWnd = nativeContext.parentWindow;
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
        parentWnd,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (!wnd)
        throw std::runtime_error("failed to create window");

    /* Set additional flags */
    if (desc.acceptDropFiles)
        DragAcceptFiles(wnd, TRUE);

    /* Set reference of this object to the window user-data */
    SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    return wnd;
}


} // /namespace LLGL



// ================================================================================
