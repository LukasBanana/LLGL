/*
 * LinuxWindow.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Desktop.h>
#include "LinuxWindow.h"
#include "MapKey.h"
#include <exception>


namespace LLGL
{


static Point GetScreenCenteredPosition(const Size& size)
{
    return (Desktop::GetResolution()/2 - size/2);
}

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    return std::unique_ptr<Window>(new LinuxWindow(desc));
}

LinuxWindow::LinuxWindow(const WindowDescriptor& desc) :
    desc_( desc )
{
    SetupDisplay();
    SetupWindow();
}

LinuxWindow::~LinuxWindow()
{
    XDestroyWindow(display_, wnd_);
    XCloseDisplay(display_);
}

void LinuxWindow::SetPosition(const Point& position)
{
    XMoveWindow(display_, wnd_, position.x, position.y);
}

Point LinuxWindow::GetPosition() const
{
    return Point();
}

void LinuxWindow::SetSize(const Size& size, bool useClientArea)
{
    XResizeWindow(display_, wnd_, static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y));
}

Size LinuxWindow::GetSize(bool useClientArea) const
{
    return Size();
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
        XMapWindow(display_, wnd_);
    else
        XUnmapWindow(display_, wnd_);
}

bool LinuxWindow::IsShown() const
{
    return false;
}

WindowDescriptor LinuxWindow::QueryDesc() const
{
    return desc_; //todo...
}

void LinuxWindow::Recreate(const WindowDescriptor& desc)
{
    //todo...
}

const void* LinuxWindow::GetNativeHandle() const
{
    return (&wnd_);
}

void LinuxWindow::ProcessSystemEvents()
{
    XEvent event;

    while (XPending(display_) > 0)
    {
        XNextEvent(display_, &event);

        switch (event.type)
        {
            case KeyPress:
                ProcessKeyEvent(event, true);
                break;

            case KeyRelease:
                ProcessKeyEvent(event, false);
                break;

            case ButtonPress:
                ProcessMouseKeyEvent(event, true);
                break;

            case ButtonRelease:
                ProcessMouseKeyEvent(event, false);
                break;

            case DestroyNotify:
                PostQuit();
                break;
        }
    }
}


/*
 * ======= Private: =======
 */

void LinuxWindow::SetupDisplay()
{
    /* Open X11 display */
    display_ = XOpenDisplay(nullptr);
    if (!display_)
        throw std::runtime_error("failed to open X11 display");
}

void LinuxWindow::SetupWindow()
{
    /* Setup window parameters */
    ::Window    rootWnd     = DefaultRootWindow(display_);
    int         screen      = DefaultScreen(display_);
    int         borderSize  = 5;
    ::Visual*   visual      = DefaultVisual(display_, screen);
    int         depth       = DefaultDepth(display_, screen);
    
    XSetWindowAttributes attribs;
    attribs.background_pixel    = WhitePixel(display_, screen);
    attribs.event_mask          = (ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    
    #if 1//!!!
    
    int visualAttribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    visual_ = glXChooseVisual(display_, screen, visualAttribs);
    attribs.colormap = XCreateColormap(display_, rootWnd, visual_->visual, AllocNone);
    
    #endif
    
    /* Get final window position */
    auto position = desc_.position;

    if (desc_.centered)
        position = GetScreenCenteredPosition(desc_.size);
        
    /* Create X11 window */
    wnd_ = XCreateWindow(
        display_,
        rootWnd,
        position.x,
        position.y,
        desc_.size.x,
        desc_.size.y,
        borderSize,
        visual_->depth,//depth,
        InputOutput,
        visual_->visual,//visual,
        //(CWBackPixel | CWEventMask), //(CWColormap | CWEventMask | CWOverrideRedirect),
        (CWColormap | CWEventMask),
        (&attribs)
    );

    /* Set title and show window (if enabled) */
    SetTitle(desc_.title);

    if (desc_.visible)
        Show();
}

void LinuxWindow::ProcessKeyEvent(XEvent& event, bool down)
{
    auto key = MapKey(event.xkey);
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}

void LinuxWindow::ProcessMouseKeyEvent(XEvent& event, bool down)
{
    switch (event.xbutton.button)
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

void LinuxWindow::PostMouseKeyEvent(Key key, bool down)
{
    if (down)
        PostKeyDown(key);
    else
        PostKeyUp(key);
}


} // /namespace LLGL



// ================================================================================
