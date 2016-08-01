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


#define FOREACH_LISTENER(ITER) \
    for (const auto& ITER : eventListeners_)

Window::EventListener::~EventListener()
{
}

void Window::EventListener::OnReset(Window& sender)
{
    // dummy
}

void Window::EventListener::OnKeyDown(Window& sender, Key keyCode)
{
    // dummy
}

void Window::EventListener::OnKeyUp(Window& sender, Key keyCode)
{
    // dummy
}

void Window::EventListener::OnChar(Window& sender, wchar_t chr)
{
    // dummy
}

void Window::EventListener::OnWheelMotion(Window& sender, int motion)
{
    // dummy
}

void Window::EventListener::OnLocalMotion(Window& sender, const Point& position)
{
    // dummy
}

void Window::EventListener::OnGlobalMotion(Window& sender, const Point& motion)
{
    // dummy
}

bool Window::EventListener::OnQuit(Window& sender)
{
    return true; // dummy
}


Window::~Window()
{
}

bool Window::ProcessEvents()
{
    FOREACH_LISTENER(lst)
        lst->OnReset(*this);

    ProcessSystemEvents();

    return (!quit_);
}

/* --- Event handling --- */

void Window::AddEventListener(const std::shared_ptr<EventListener>& eventListener)
{
    AddListenerGlob(eventListeners_, eventListener);
}

void Window::RemoveEventListener(const EventListener* eventListener)
{
    RemoveListenerGlob(eventListeners_, eventListener);
}

void Window::PostKeyDown(Key keyCode)
{
    FOREACH_LISTENER(lst)
        lst->OnKeyDown(*this, keyCode);
}

void Window::PostKeyUp(Key keyCode)
{
    FOREACH_LISTENER(lst)
        lst->OnKeyUp(*this, keyCode);
}

void Window::PostChar(wchar_t chr)
{
    FOREACH_LISTENER(lst)
        lst->OnChar(*this, chr);
}

void Window::PostWheelMotion(int motion)
{
    FOREACH_LISTENER(lst)
        lst->OnWheelMotion(*this, motion);
}

void Window::PostLocalMotion(const Point& position)
{
    FOREACH_LISTENER(lst)
        lst->OnLocalMotion(*this, position);
}

void Window::PostGlobalMotion(const Point& motion)
{
    FOREACH_LISTENER(lst)
        lst->OnGlobalMotion(*this, motion);
}

void Window::PostQuit()
{
    FOREACH_LISTENER(lst)
    {
        if (!lst->OnQuit(*this))
            return;
    }
    quit_ = true;
}

#undef FOREACH_LISTENER


} // /namespace LLGL



// ================================================================================
