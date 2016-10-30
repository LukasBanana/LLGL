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


/* ----- Window EventListener class ----- */

Window::EventListener::~EventListener()
{
}

void Window::EventListener::OnProcessEvents(Window& sender)
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

void Window::EventListener::OnDoubleClick(Window& sender, Key keyCode)
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

void Window::EventListener::OnResize(Window& sender, const Size& clientAreaSize)
{
    // dummy
}

bool Window::EventListener::OnQuit(Window& sender)
{
    return true; // dummy
}


/* ----- Window class ----- */

#define FOREACH_LISTENER_CALL(FUNC) \
    for (const auto& lst : eventListeners_) { lst->FUNC; }

Window::~Window()
{
}

#ifdef LLGL_MOBILE_PLATFORM

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    /* For mobile platforms this function always returns null */
    return nullptr;
}

#endif

bool Window::ProcessEvents()
{
    FOREACH_LISTENER_CALL( OnProcessEvents(*this) );

    OnProcessEvents();

    return (!quit_);
}

/* --- Event handling --- */

void Window::AddEventListener(const std::shared_ptr<EventListener>& eventListener)
{
    AddOnceToSharedList(eventListeners_, eventListener);
}

void Window::RemoveEventListener(const EventListener* eventListener)
{
    RemoveFromSharedList(eventListeners_, eventListener);
}

void Window::PostKeyDown(Key keyCode)
{
    FOREACH_LISTENER_CALL( OnKeyDown(*this, keyCode) );
}

void Window::PostKeyUp(Key keyCode)
{
    FOREACH_LISTENER_CALL( OnKeyUp(*this, keyCode) );
}

void Window::PostDoubleClick(Key keyCode)
{
    FOREACH_LISTENER_CALL( OnDoubleClick(*this, keyCode) );
}

void Window::PostChar(wchar_t chr)
{
    FOREACH_LISTENER_CALL( OnChar(*this, chr) );
}

void Window::PostWheelMotion(int motion)
{
    FOREACH_LISTENER_CALL( OnWheelMotion(*this, motion) );
}

void Window::PostLocalMotion(const Point& position)
{
    FOREACH_LISTENER_CALL( OnLocalMotion(*this, position) );
}

void Window::PostGlobalMotion(const Point& motion)
{
    FOREACH_LISTENER_CALL( OnGlobalMotion(*this, motion) );
}

void Window::PostResize(const Size& clientAreaSize)
{
    FOREACH_LISTENER_CALL( OnResize(*this, clientAreaSize) );
}

void Window::PostQuit()
{
    for (const auto& lst : eventListeners_)
    {
        if (!lst->OnQuit(*this))
            return;
    }
    quit_ = true;
}

#undef FOREACH_LISTENER_CALL


} // /namespace LLGL



// ================================================================================
