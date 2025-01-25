/*
 * LinuxWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "LinuxWindow.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Exception.h"
#include <X11/Xresource.h>


namespace LLGL
{


/*
 * LinuxX11Context class
 */

// Wrapper for XContext singleton.
class LinuxX11Context
{

    public:
        
        static void Save(::Display* display, XID id, void* userData);
        static void* Find(::Display* display, XID id);
        static void Remove(::Display* display, XID id);

    private:

        LinuxX11Context();

        static LinuxX11Context& Get();

    private:

        ::XContext ctx_;

};

LinuxX11Context::LinuxX11Context() :
    ctx_ { XUniqueContext() }
{
}

void LinuxX11Context::Save(::Display* display, XID id, void* userData)
{
    XSaveContext(display, id, LinuxX11Context::Get().ctx_, static_cast<XPointer>(userData));
}

void* LinuxX11Context::Find(::Display* display, XID id)
{
    XPointer userData = nullptr;
    XFindContext(display, id, LinuxX11Context::Get().ctx_, &userData);
    return reinterpret_cast<void*>(userData);
}

void LinuxX11Context::Remove(::Display* display, XID id)
{
    XDeleteContext(display, id, LinuxX11Context::Get().ctx_);
}

LinuxX11Context& LinuxX11Context::Get()
{
    static LinuxX11Context instance;
    return instance;
}


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    ::Display* display = LinuxSharedX11Display::GetShared()->GetNative();

    XEvent event;

    XPending(display);

    while (XQLength(display))
    {
        XNextEvent(display, &event);
        if (void* userData = LinuxX11Context::Find(display, event.xany.window))
        {
            LinuxWindow* wnd = static_cast<LinuxWindow*>(userData);
            wnd->ProcessEvent(event);
        }
    }

    XFlush(display);

    return true;
}


/*
 * Window class
 */

static Offset2D GetScreenCenteredPosition(const Extent2D& size)
{
    if (auto display = Display::GetPrimary())
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
    return MakeUnique<LinuxWindow>(desc);
}


/*
 * LinuxWindow class
 */

LinuxWindow::LinuxWindow(const WindowDescriptor& desc) :
    desc_ { desc }
{
    OpenX11Window();
    LinuxX11Context::Save(display_, wnd_, this);
}

LinuxWindow::~LinuxWindow()
{
    LinuxX11Context::Remove(display_, wnd_);
    XDestroyWindow(display_, wnd_);
}

bool LinuxWindow::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->display = display_;
        handle->window  = wnd_;
        handle->visual  = visual_;
        return true;
    }
    return false;
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

void LinuxWindow::SetTitle(const UTF8String& title)
{
    XStoreName(display_, wnd_, title.c_str());
}

UTF8String LinuxWindow::GetTitle() const
{
    char* title = nullptr;
    XFetchName(display_, wnd_, &title);
    return title;
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

    if ((desc_.flags & WindowFlags::Borderless) != 0)
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

void LinuxWindow::ProcessEvent(XEvent& event)
{
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


/*
 * ======= Private: =======
 */

void LinuxWindow::OpenX11Window()
{
    /* Get native context handle */
    const NativeHandle* nativeHandle = static_cast<const NativeHandle*>(desc_.windowContext);
    if (nativeHandle != nullptr)
    {
        /* Get X11 display from context handle */
        LLGL_ASSERT(desc_.windowContextSize == sizeof(NativeHandle));
        display_    = nativeHandle->display;
        visual_     = nativeHandle->visual;
    }
    else
    {
        /* Get shared X11 display */
        sharedX11Display_   = LinuxSharedX11Display::GetShared();
        display_            = sharedX11Display_->GetNative();
        visual_             = nullptr;
    }

    if (!display_)
        LLGL_TRAP("failed to open X11 display");

    /* Setup common parameters for window creation */
    ::Window    rootWnd     = (nativeHandle != nullptr ? nativeHandle->window : DefaultRootWindow(display_));
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

    const bool isBorderless = ((desc_.flags & WindowFlags::Borderless) != 0);
    if (isBorderless) //TODO: -> input no longer works
    {
        valueMask |= CWOverrideRedirect;
        attribs.override_redirect = true;
    }

    /* Get final window position */
    if ((desc_.flags & WindowFlags::Centered) != 0)
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
    if ((desc_.flags & WindowFlags::Visible) != 0)
        Show();

    /* Prepare borderless window */
    if (isBorderless)
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
