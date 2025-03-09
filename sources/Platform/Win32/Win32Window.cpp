/*
 * Win32Window.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32Window.h"
#include "Win32WindowClass.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    /* Peek all queued messages */
    MSG message;
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return true;
}


/* ----- Internal structures ----- */

struct Win32FrameAndStyle
{
    DWORD       style   = 0;
    Offset2D    position;
    Extent2D    size;
};


/* ----- Internal functions ----- */

static void SetWin32UserData(HWND wnd, void* userData)
{
    SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userData));
}

// Queries window rectangular area by the specified client area size and window style.
static RECT GetWin32ClientArea(LONG width, LONG height, DWORD style)
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

// Determines the Win32 window style for the specified descriptor.
static DWORD GetWin32WindowStyle(const WindowDescriptor& desc)
{
    DWORD style = (WS_CLIPCHILDREN | WS_CLIPSIBLINGS); // Both required for OpenGL

    const bool hasWindowContext =
    (
        desc.windowContext != nullptr &&
        desc.windowContextSize == sizeof(NativeHandle) &&
        static_cast<const NativeHandle*>(desc.windowContext)->window != 0
    );

    if (hasWindowContext)
        style |= WS_CHILD;
    else if ((desc.flags & WindowFlags::Borderless) != 0)
        style |= WS_POPUP;
    else
    {
        style |= (WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION);
        if ((desc.flags & WindowFlags::Resizable) != 0)
            style |= (WS_SIZEBOX | WS_MAXIMIZEBOX);
    }

    if ((desc.flags & WindowFlags::Visible) != 0)
        style |= WS_VISIBLE;

    if ((desc.flags & WindowFlags::AcceptDropFiles) != 0)
        style |= WM_DROPFILES;

    return style;
}

static Offset2D GetScreenCenteredPosition(const Extent2D& size)
{
    return
    {
        GetSystemMetrics(SM_CXSCREEN)/2 - static_cast<int>(size.width/2),
        GetSystemMetrics(SM_CYSCREEN)/2 - static_cast<int>(size.height/2)
    };
}

static Win32FrameAndStyle GetWin32FrameAndStyleFromDesc(const WindowDescriptor& desc)
{
    Win32FrameAndStyle frame;

    /* Get window style and client area */
    frame.style = GetWin32WindowStyle(desc);

    auto rc = GetWin32ClientArea(
        static_cast<LONG>(desc.size.width),
        static_cast<LONG>(desc.size.height),
        frame.style
    );

    /* Setup window size */
    frame.size.width   = static_cast<std::uint32_t>(rc.right - rc.left);
    frame.size.height  = static_cast<std::uint32_t>(rc.bottom - rc.top);

    /* Setup window position */
    const bool isCentered = ((desc.flags & WindowFlags::Centered) != 0);

    frame.position = (isCentered ? GetScreenCenteredPosition(desc.size) : desc.position);

    if (isCentered)
    {
        frame.position.x += rc.left;
        frame.position.y += rc.top;
    }

    return frame;
}


/*
 * Win32Window class
 */

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return MakeUnique<Win32Window>(desc);
}

Win32Window::Win32Window(const WindowDescriptor& desc) :
    wnd_   { CreateWindowHandle(desc) },
    flags_ { desc.flags               }
{
}

Win32Window::~Win32Window()
{
    DestroyWindow(wnd_);
}

bool Win32Window::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->window = wnd_;
        return true;
    }
    return false;
}

Extent2D Win32Window::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void Win32Window::SetPosition(const Offset2D& position)
{
    SetWindowPos(wnd_, HWND_TOP, position.x, position.y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER));
}

Offset2D Win32Window::GetPosition() const
{
    RECT rc;
    GetWindowRect(wnd_, &rc);
    MapWindowPoints(HWND_DESKTOP, GetParent(wnd_), reinterpret_cast<LPPOINT>(&rc), 2);
    return { rc.left, rc.top };
}

void Win32Window::SetSize(const Extent2D& size, bool useClientArea)
{
    int cx, cy;

    if (useClientArea)
    {
        RECT rc = GetWin32ClientArea(
            static_cast<LONG>(size.width),
            static_cast<LONG>(size.height),
            GetWindowLong(wnd_, GWL_STYLE)
        );
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;
    }
    else
    {
        cx = static_cast<int>(size.width);
        cy = static_cast<int>(size.height);
    }

    SetWindowPos(wnd_, HWND_TOP, 0, 0, cx, cy, (SWP_NOMOVE | SWP_NOZORDER));
}

Extent2D Win32Window::GetSize(bool useClientArea) const
{
    if (useClientArea)
    {
        RECT rc;
        GetClientRect(wnd_, &rc);
        return
        {
            static_cast<std::uint32_t>(rc.right - rc.left),
            static_cast<std::uint32_t>(rc.bottom - rc.top)
        };
    }
    else
    {
        RECT rc;
        GetWindowRect(wnd_, &rc);
        return
        {
            static_cast<std::uint32_t>(rc.right - rc.left),
            static_cast<std::uint32_t>(rc.bottom - rc.top)
        };
    }
}

void Win32Window::SetTitle(const UTF8String& title)
{
    #ifdef UNICODE
    auto titleUTF16 = title.to_utf16();
    SetWindowText(wnd_, titleUTF16.data());
    #else
    SetWindowText(wnd_, title.c_str());
    #endif
}

UTF8String Win32Window::GetTitle() const
{
    /* Retrieve window title and return as immutable string */
    int len = GetWindowTextLength(wnd_);
    if (len > 0)
    {
        auto title = MakeUniqueArray<TCHAR>(len + 1);
        GetWindowText(wnd_, &title[0], len + 1);
        return UTF8String{ title.get() };
    }
    return {};
}

void Win32Window::Show(bool show)
{
    ShowWindow(wnd_, (show ? SW_NORMAL : SW_HIDE));
}

bool Win32Window::IsShown() const
{
    return (IsWindowVisible(wnd_) ? true : false);
}

WindowDescriptor Win32Window::GetDesc() const
{
    /* Get window flags and other information for comparision */
    const LONG      style       = GetWindowLong(wnd_, GWL_STYLE);
    const Extent2D  windowSize  = GetSize();
    const Offset2D  centerPoint = GetScreenCenteredPosition(windowSize);

    /* Setup window descriptor */
    WindowDescriptor desc;
    {
        desc.title      = GetTitle();
        desc.position   = GetPosition();
        desc.size       = windowSize;

        if ((style & WS_VISIBLE  ) != 0)
            desc.flags |= WindowFlags::Visible;
        if ((style & WS_CAPTION  ) == 0)
            desc.flags |= WindowFlags::Borderless;
        if ((style & WS_SIZEBOX  ) != 0)
            desc.flags |= WindowFlags::Resizable;
        if ((style & WM_DROPFILES) != 0)
            desc.flags |= WindowFlags::AcceptDropFiles;
        if (centerPoint.x == desc.position.x && centerPoint.y == desc.position.y)
            desc.flags |= WindowFlags::Centered;

        if (parentWnd_ != nullptr)
        {
            desc.windowContext      = &parentWnd_;
            desc.windowContextSize  = sizeof(parentWnd_);
        }
    }
    return desc;
}

void Win32Window::SetDesc(const WindowDescriptor& desc)
{
    /* Get current window flags */
    const DWORD oldStyle = static_cast<DWORD>(GetWindowLong(wnd_, GWL_STYLE));

    const bool isBorderless = ((oldStyle & WS_CAPTION) == 0);
    const bool isResizable  = ((oldStyle & WS_SIZEBOX) != 0);

    /* Setup new window flags */
    DWORD newStyle = GetWin32WindowStyle(desc);

    if ((oldStyle & WS_MAXIMIZE) != 0)
        newStyle |= WS_MAXIMIZE;
    if ((oldStyle & WS_MINIMIZE) != 0)
        newStyle |= WS_MINIMIZE;

    const bool haveFlagsChanged = (oldStyle != newStyle);

    /* Check if anything changed */
    auto position           = GetPosition();
    auto size               = GetSize();

    bool positionChanged    = (desc.position.x != position.x || desc.position.y != position.y);
    bool sizeChanged        = (desc.size.width != size.width || desc.size.height != size.height);

    if (haveFlagsChanged || positionChanged || sizeChanged)
    {
        UINT flags = SWP_NOZORDER;

        if (haveFlagsChanged)
        {
            /* Hide temporarily to avoid strange effects during frame change (if frame has changed) */
            ShowWindow(wnd_, SW_HIDE);

            /* Set new window style */
            SetWindowLongPtr(wnd_, GWL_STYLE, newStyle);
            flags |= SWP_FRAMECHANGED;
        }

        /* Set new position and size */
        const Win32FrameAndStyle frame = GetWin32FrameAndStyleFromDesc(desc);

        if ((desc.flags & WindowFlags::Visible) != 0)
            flags |= SWP_SHOWWINDOW;

        if ((newStyle & WS_MAXIMIZE) != 0)
            flags |= (SWP_NOSIZE | SWP_NOMOVE);

        if (isBorderless == ((desc.flags & WindowFlags::Borderless) != 0) &&
            isResizable  == ((desc.flags & WindowFlags::Resizable ) != 0))
        {
            if (!positionChanged)
                flags |= SWP_NOMOVE;
            if (!sizeChanged)
                flags |= SWP_NOSIZE;
        }

        SetWindowPos(
            wnd_,
            0, // ignore, due to SWP_NOZORDER flag
            frame.position.x,
            frame.position.y,
            static_cast<int>(frame.size.width),
            static_cast<int>(frame.size.height),
            flags
        );
    }

    /* Store new flags */
    flags_ = desc.flags;
}

Win32Window* Win32Window::GetFromUserData(HWND wnd)
{
    return reinterpret_cast<Win32Window*>(GetWindowLongPtr(wnd, GWLP_USERDATA));
}


/*
 * ======= Private: =======
 */

static HWND GetNativeWin32ParentWindow(const void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
        return static_cast<const NativeHandle*>(nativeHandle)->window;
    else
        return nullptr;
}

HWND Win32Window::CreateWindowHandle(const WindowDescriptor& desc)
{
    /* Get final window size */
    const Win32FrameAndStyle frame = GetWin32FrameAndStyleFromDesc(desc);

    /* Get parent window */
    parentWnd_ = GetNativeWin32ParentWindow(desc.windowContext, desc.windowContextSize);
    HWND parentWndOrDesktop = (parentWnd_ != nullptr ? parentWnd_ : HWND_DESKTOP);

    #ifdef UNICODE
    SmallVector<wchar_t> title = desc.title.to_utf16();
    #else
    const UTF8String& title = desc.title;
    #endif

    /* Create frame window object */
    HWND wnd = CreateWindow(
        Win32WindowClass::Get()->GetName(),
        title.data(),
        frame.style,
        frame.position.x,
        frame.position.y,
        static_cast<int>(frame.size.width),
        static_cast<int>(frame.size.height),
        parentWndOrDesktop,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    LLGL_ASSERT(wnd != nullptr, "failed to create Win32 window");

    #ifndef LLGL_ARCH_ARM
    /* Set additional flags */
    if ((desc.flags & WindowFlags::AcceptDropFiles) != 0)
        DragAcceptFiles(wnd, TRUE);
    #endif

    /* Set reference of this object to the window user-data */
    SetWin32UserData(wnd, this);

    return wnd;
}


} // /namespace LLGL



// ================================================================================
