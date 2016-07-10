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


std::unique_ptr<Window> Window::Create(const WindowDesc& desc)
{
    return std::unique_ptr<Window>(new LinuxWindow(desc));
}

LinuxWindow::LinuxWindow(const WindowDesc& desc) :
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
}

Point LinuxWindow::GetPosition() const
{
    return Point();
}

void LinuxWindow::SetSize(const Size& size, bool useClientArea)
{
}

Size LinuxWindow::GetSize(bool useClientArea) const
{
    return Size();
}

void LinuxWindow::SetTitle(const std::wstring& title)
{
}

std::wstring LinuxWindow::GetTitle() const
{
    return std::wstring();
}

void LinuxWindow::Show(bool show)
{
}

bool LinuxWindow::IsShown() const
{
    return false;
}

const void* LinuxWindow::GetNativeHandle() const
{
    return nullptr;
}

void LinuxWindow::ProcessSystemEvents()
{
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
    //todo...

    /* Create X11 window */
    /*wnd_ = XCreateWindow(
        display_,
        DefaultRootWindow(display_),
        desc.position.x,
        desc.position.y,
        desc.size.x,
        desc.size.y,
        0,
        visual->depth,
        io,
        visual->visual,
        (CWColormap | CWEventMask | CWOverrideRedirect),
        (&attribs)
    );*/


}


} // /namespace LLGL



// ================================================================================
