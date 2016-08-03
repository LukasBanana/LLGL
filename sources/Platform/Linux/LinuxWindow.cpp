/*
 * LinuxWindow.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxWindow.h"

#include <exception>


namespace LLGL
{


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
}

void LinuxWindow::SetPosition(const Point& position)
{
    XWindowChanges attribs;
    attribs.x = position.x;
    attribs.y = position.y;
    XConfigureWindow(display_, wnd_, (CWX | CWY), &attribs);
}

Point LinuxWindow::GetPosition() const
{
    return Point();
}

void LinuxWindow::SetSize(const Size& size, bool useClientArea)
{
    XWindowChanges attribs;
    attribs.width = size.x;
    attribs.height = size.y;
    XConfigureWindow(display_, wnd_, (CWWidth | CWHeight), &attribs);
}

Size LinuxWindow::GetSize(bool useClientArea) const
{
    return Size();
}

void LinuxWindow::SetTitle(const std::wstring& title)
{
    /* Convert UTF16 to UTF8 string (X11 limitation) */
    std::string titleUTF8;
    titleUTF8.resize(title.size());
    for (std::size_t i = 0; i < title.size(); ++i)
        titleUTF8[i] = static_cast<char>(title[i]);

    /* Set windwo title */
    XStoreName(display_, wnd_, titleUTF8.c_str());
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
        throw std::runtime_error("opening X11 display failed");
}

void LinuxWindow::SetupWindow()
{
    /* Setup window parameters */
    ::Window    rootWnd     = DefaultRootWindow(display_);
    int         screen      = DefaultScreen(display_);
    int         borderSize  = 5;
    Visual*     visual      = DefaultVisual(display_, screen);
    int         depth       = DefaultDepth(display_, screen);

    XSetWindowAttributes attribs;
    attribs.background_pixel = WhitePixel(display_, screen);

    /* Create X11 window */
    wnd_ = XCreateWindow(
        display_,
        rootWnd,
        desc_.position.x,
        desc_.position.y,
        desc_.size.x,
        desc_.size.y,
        borderSize,
        depth,
        InputOutput,
        visual,
        //(CWColormap | CWEventMask | CWOverrideRedirect),
        CWBackPixel,
        (&attribs)
    );

    /* Set title and show window (if enabled) */
    SetTitle(desc_.title);

    if (desc_.visible)
        Show();
}

void LinuxWindow::ProcessKeyEvent(XEvent& event, bool down)
{
}

void LinuxWindow::ProcessMouseKeyEvent(XEvent& event, bool down)
{
}


} // /namespace LLGL



// ================================================================================
