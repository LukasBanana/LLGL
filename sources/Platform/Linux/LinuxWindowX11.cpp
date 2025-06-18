/*
 * LinuxWindow.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Display.h>
#include "LinuxWindowX11.h"
#include "MapKey.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Exception.h"
#include <X11/Xresource.h>


namespace LLGL
{

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


/*
 * LinuxWindowX11 class
 */

LinuxWindowX11::LinuxWindowX11(const WindowDescriptor& desc) :
    desc_ { desc }
{
    Open();
    LinuxX11Context::Save(display_, wnd_, this);
}

LinuxWindowX11::~LinuxWindowX11()
{
    LinuxX11Context::Remove(display_, wnd_);
    XDestroyWindow(display_, wnd_);
}

bool LinuxWindowX11::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->type        = NativeType::X11;
        handle->x11.display = display_;
        handle->x11.window  = wnd_;
        handle->x11.visual  = visual_;
        LLGL_DEPRECATED_IGNORE_PUSH()
        handle->display     = display_;
        handle->window      = wnd_;
        handle->visual      = visual_;
        LLGL_DEPRECATED_IGNORE_POP()
        return true;
    }
    return false;
}

Extent2D LinuxWindowX11::GetContentSize() const
{
    /* Return the size of the client area */
    return GetSize(true);
}

void LinuxWindowX11::SetPosition(const Offset2D& position)
{
    /* Move window and store new position */
    XMoveWindow(display_, wnd_, position.x, position.y);
    desc_.position = position;
}

Offset2D LinuxWindowX11::GetPosition() const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return { attribs.x, attribs.y };
}

void LinuxWindowX11::SetSize(const Extent2D& size, bool useClientArea)
{
    XResizeWindow(display_, wnd_, size.width, size.height);
}

Extent2D LinuxWindowX11::GetSize(bool useClientArea) const
{
    XWindowAttributes attribs;
    XGetWindowAttributes(display_, wnd_, &attribs);
    return Extent2D
    {
        static_cast<std::uint32_t>(attribs.width),
        static_cast<std::uint32_t>(attribs.height)
    };
}

void LinuxWindowX11::SetTitle(const UTF8String& title)
{
    XStoreName(display_, wnd_, title.c_str());
}

UTF8String LinuxWindowX11::GetTitle() const
{
    char* title = nullptr;
    XFetchName(display_, wnd_, &title);
    return title;
}

void LinuxWindowX11::Show(bool show)
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

bool LinuxWindowX11::IsShown() const
{
    return false;
}

void LinuxWindowX11::SetDesc(const WindowDescriptor& desc)
{
    //todo...
}

WindowDescriptor LinuxWindowX11::GetDesc() const
{
    return desc_; //todo...
}

void LinuxWindowX11::ProcessEvent(XEvent& event)
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

void LinuxWindowX11::Open()
{
    /* Get native context handle */
    const NativeHandle* nativeHandle = static_cast<const NativeHandle*>(desc_.windowContext);
    if (nativeHandle != nullptr)
    {
        /* Get X11 display from context handle */
        LLGL_ASSERT(desc_.windowContextSize == sizeof(NativeHandle));
        LLGL_ASSERT(nativeHandle->type == NativeType::X11, "Window native handle type must be X11");
        display_    = nativeHandle->x11.display;
        visual_     = nativeHandle->x11.visual;
    }
    else
    {
        /* Get shared X11 display */
        sharedX11Display_   = LinuxSharedDisplayX11::GetShared();
        display_            = sharedX11Display_->GetNative();
        visual_             = nullptr;
    }

    if (!display_)
        LLGL_TRAP("failed to open X11 display");

    /* Setup common parameters for window creation */
    ::Window    rootWnd     = (nativeHandle != nullptr ? nativeHandle->x11.window : DefaultRootWindow(display_));
    int         screen      = (nativeHandle != nullptr ? nativeHandle->x11.screen : DefaultScreen(display_));
    ::Visual*   visual      = (nativeHandle != nullptr ? nativeHandle->x11.visual->visual : DefaultVisual(display_, screen));
    int         depth       = (nativeHandle != nullptr ? nativeHandle->x11.visual->depth : DefaultDepth(display_, screen));
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
        attribs.colormap = nativeHandle->x11.colorMap;
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

void LinuxWindowX11::ProcessKeyEvent(XKeyEvent& event, bool down)
{
    auto key = MapKey(event);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxWindowX11::ProcessMouseKeyEvent(XButtonEvent& event, bool down)
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

void LinuxWindowX11::ProcessExposeEvent()
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

void LinuxWindowX11::ProcessClientMessage(XClientMessageEvent& event)
{
    Atom atom = static_cast<Atom>(event.data.l[0]);
    if (atom == closeWndAtom_)
        PostQuit();
}

void LinuxWindowX11::ProcessMotionEvent(XMotionEvent& event)
{
    const Offset2D mousePos { event.x, event.y };
    PostLocalMotion(mousePos);
    PostGlobalMotion({ mousePos.x - prevMousePos_.x, mousePos.y - prevMousePos_.y });
    prevMousePos_ = mousePos;
}

void LinuxWindowX11::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

} // /namespace LLGL



// ================================================================================
