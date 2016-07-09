/*
 * Window.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Window.h>
#include "../Core/Helper.h"


namespace LLGL
{


Window::Listener::~Listener()
{
}

void Window::Listener::OnKeyDown(Key keyCode)
{
    // dummy
}

void Window::Listener::OnKeyUp(Key keyCode)
{
    // dummy
}
                
void Window::Listener::OnChar(wchar_t chr)
{
    // dummy
}

void Window::Listener::OnWheelMotion(int motion)
{
    // dummy
}

void Window::Listener::OnLocalMotion(int x, int y)
{
    // dummy
}

void Window::Listener::OnGlobalMotion(int dx, int dy)
{
    // dummy
}


Window::~Window()
{
}

bool Window::ProcessEvents()
{
    ProcessSystemEvents();
    return (!quit_);
}

/* --- Event handling --- */

#define FOREACH_LISTENER(i) \
    for (const auto& i : listeners_)

void Window::AddListener(const std::shared_ptr<Listener>& listener)
{
    AddListenerGlob(listeners_, listener);
}

void Window::RemoveListener(const Listener* listener)
{
    RemoveListenerGlob(listeners_, listener);
}

void Window::PostKeyDown(Key keyCode)
{
    FOREACH_LISTENER(lst)
        lst->OnKeyDown(keyCode);
}

void Window::PostKeyUp(Key keyCode)
{
    FOREACH_LISTENER(lst)
        lst->OnKeyUp(keyCode);
}

void Window::PostChar(wchar_t chr)
{
    FOREACH_LISTENER(lst)
        lst->OnChar(chr);
}

void Window::PostWheelMotion(int motion)
{
    FOREACH_LISTENER(lst)
        lst->OnWheelMotion(motion);
}

void Window::PostLocalMotion(int x, int y)
{
    FOREACH_LISTENER(lst)
        lst->OnLocalMotion(x, y);
}

void Window::PostGlobalMotion(int dx, int dy)
{
    FOREACH_LISTENER(lst)
        lst->OnGlobalMotion(dx, dy);
}

void Window::PostQuit()
{
    quit_ = true;
}

#undef FOREACH_LISTENER


} // /namespace LLGL



// ================================================================================
