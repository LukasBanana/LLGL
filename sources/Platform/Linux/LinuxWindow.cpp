/*
 * LinuxWindow.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "LinuxWindow.h"
#include "MapKey.h"
#include <exception>


namespace LLGL
{


static Offset2D GetScreenCenteredPosition(const Extent2D& size)
{
    if (auto display = Display::InstantiatePrimary())
    {
        const auto resolution = display->GetDisplayMode().resolution;
        return
        {
            static_cast<int>((resolution.width  - size.width )/2),
            static_cast<int>((resolution.height - size.height)/2),
        };
    }
    return {};
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new LinuxWindow(desc));
}

LinuxWindow::LinuxWindow(const WindowDescriptor& desc) :
    desc_ { desc }
{
    OpenWindow();
}

LinuxWindow::~LinuxWindow()
{
    XDestroyWindow(display_, wnd_);
    XCloseDisplay(display_);
}

bool LinuxWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandleSize == sizeof(NativeHandle))
    {
        auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
        handle.display  = display_;
        handle.window   = wnd_;
        handle.visual   = visual_;
        return true;
    }
    return false;
}

void LinuxWindow::ResetPixelFormat()
{
    // dummy
}

Extent2D LinuxWindow::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void LinuxWindow::SetPosition(const Offset2D& position)
{
    /* Move window and store new position */
    XMoveWindow(display_, wnd_, position.x, position.y);
    desc_.position = position;
}

Offset2D LinuxWindow::GetPosition() const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return { attribs.x, attribs.y };
}

void LinuxWindow::SetSize(const Extent2D& size, bool useClientArea)
{
    XResizeWindow(display_, wnd_, size.width, size.height);
}

Extent2D LinuxWindow::GetSize(bool useClientArea) const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return Extent2D
    {
        static_cast<std::uint32_t>(attribs.width),
        static_cast<std::uint32_t>(attribs.height)
    };
}

void LinuxWindow::SetTitle(const std::wstring& title)
{
    /* Convert UTF16 to UTF8 string (X11 limitation) and set window title */
    std::string s(title.begin(), title.end());
    XStoreName(display_, wnd_, s.c_str());
}

std::wstring LinuxWindow::GetTitle() const
{
    return std::wstring();
}

void LinuxWindow::Show(bool show)
{
    if (show)
    {
        /* Map window and reset window position */
        XMapWindow(display_, wnd_);
        XMoveWindow(display_, wnd_, desc_.position.x, desc_.position.y);
    }
    else
        XUnmapWindow(display_, wnd_);
        
    if (desc_.borderless)
        XSetInputFocus(display_, (show ? wnd_ : None), RevertToParent, CurrentTime);
}

bool LinuxWindow::IsShown() const
{
    return false;
}

void LinuxWindow::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

WindowDescriptor LinuxWindow::GetDesc() const
{
    return desc_; //todo...
}

void LinuxWindow::OnProcessEvents()
{
    XEvent event;

    XPending(display_);

    while (XQLength(display_))
    {
        XNextEvent(display_, &event);

        switch (event.type)
        {
            case KeyPress:
                ProcessKeyEvent(event.xkey, true);
                break;

            case KeyRelease:
                ProcessKeyEvent(event.xkey, false);
                break;

            case ButtonPress:
                ProcessMouseKeyEvent(event.xbutton, true);
                break;

            case ButtonRelease:
                ProcessMouseKeyEvent(event.xbutton, false);
                break;
                
            case Expose:
                ProcessExposeEvent();
                break;

            case MotionNotify:
                ProcessMotionEvent(event.xmotion);
                break;

            case DestroyNotify:
                PostQuit();
                break;
                
            case ClientMessage:
                ProcessClientMessage(event.xclient);
                break;
        }
    }

    XFlush(display_);
}


/*
 * ======= Private: =======
 */

void LinuxWindow::OpenWindow()
{
    /* Get native context handle */
    auto nativeHandle = reinterpret_cast<const NativeContextHandle*>(desc_.windowContext);
    if (nativeHandle)
    {
        /* Get X11 display from context handle */
        display_    = nativeHandle->display;
        visual_     = nativeHandle->visual;
    }
    else
    {
        /* Open X11 display */
        display_    = XOpenDisplay(nullptr);
        visual_     = nullptr;
    }

    if (!display_)
        throw std::runtime_error("failed to open X11 display");
        
    /* Setup common parameters for window creation */
    ::Window    rootWnd     = (nativeHandle != nullptr ? nativeHandle->parentWindow : DefaultRootWindow(display_));
    int         screen      = (nativeHandle != nullptr ? nativeHandle->screen : DefaultScreen(display_));
    ::Visual*   visual      = (nativeHandle != nullptr ? nativeHandle->visual->visual : DefaultVisual(display_, screen));
    int         depth       = (nativeHandle != nullptr ? nativeHandle->visual->depth : DefaultDepth(display_, screen));
    int         borderSize  = 0;
    
    /* Setup window attributes */
    XSetWindowAttributes attribs;
    attribs.background_pixel    = WhitePixel(display_, screen);
    attribs.border_pixel        = 0;
    attribs.event_mask          = (ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    
    unsigned long valueMask     = CWEventMask | CWBorderPixel;//(CWColormap | CWEventMask | CWOverrideRedirect)

    if (nativeHandle)
    {
        valueMask |= CWColormap;
        attribs.colormap = nativeHandle->colorMap;
    }
    else
        valueMask |= CWBackPixel;
        
    if (desc_.borderless) //WARNING -> input no longer works
    {
        valueMask |= CWOverrideRedirect;
        attribs.override_redirect = true;
    }

    /* Get final window position */
    if (desc_.centered)
        desc_.position = GetScreenCenteredPosition(desc_.size);

    /* Create X11 window */
    wnd_ = XCreateWindow(
        display_,
        rootWnd,
        desc_.position.x,
        desc_.position.y,
        desc_.size.width,
        desc_.size.height,
        borderSize,
        depth,
        InputOutput,
        visual,
        valueMask,
        (&attribs)
    );
    
    /* Set title and show window (if enabled) */
    SetTitle(desc_.title);

    /* Show window */
    if (desc_.visible)
        Show();
    
    /* Prepare borderless window */
    if (desc_.borderless)
    {
        XGrabKeyboard(display_, wnd_, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(display_, wnd_, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, wnd_, None, CurrentTime);
    }
    
    /* Enable WM_DELETE_WINDOW protocol */
    closeWndAtom_ = XInternAtom(display_, "WM_DELETE_WINDOW", False); 
    XSetWMProtocols(display_, wnd_, &closeWndAtom_, 1);
}

void LinuxWindow::ProcessKeyEvent(XKeyEvent& event, bool down)
{
    auto key = MapKey(event);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxWindow::ProcessMouseKeyEvent(XButtonEvent& event, bool down)
{
    switch (event.button)
    {
        case Button1:
            PostMouseKeyEvent(Key::LButton, down);
            break;
        case Button2:
            PostMouseKeyEvent(Key::MButton, down);
            break;
        case Button3:
            PostMouseKeyEvent(Key::RButton, down);
            break;
        case Button4:
            PostWheelMotion(1);
            break;
        case Button5:
            PostWheelMotion(-1);
            break;
    }
}

void LinuxWindow::ProcessExposeEvent()
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    
    const Extent2D size
    {
        static_cast<std::uint32_t>(attribs.width),
        static_cast<std::uint32_t>(attribs.height)
    };
    
    PostResize(size);
}

void LinuxWindow::ProcessClientMessage(XClientMessageEvent& event)
{
    Atom atom = static_cast<Atom>(event.data.l[0]);
    if (atom == closeWndAtom_)
        PostQuit();
}

void LinuxWindow::ProcessMotionEvent(XMotionEvent& event)
{
    const Offset2D mousePos { event.x, event.y };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;
}

void LinuxWindow::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}


} // /namespace LLGL



// ================================================================================
