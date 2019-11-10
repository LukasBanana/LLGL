/*
 * Window.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Window.h>
#include "../Core/Helper.h"


namespace LLGL
{


/* ----- Window EventListener class ----- */

void Window::EventListener::OnProcessEvents(Window& sender)
{
    // dummy
}

void Window::EventListener::OnQuit(Window& sender, bool& veto)
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

void Window::EventListener::OnLocalMotion(Window& sender, const Offset2D& position)
{
    // dummy
}

void Window::EventListener::OnGlobalMotion(Window& sender, const Offset2D& motion)
{
    // dummy
}

void Window::EventListener::OnResize(Window& sender, const Extent2D& clientAreaSize)
{
    // dummy
}

void Window::EventListener::OnGetFocus(Window& sender)
{
    // dummy
}

void Window::EventListener::OnLostFocus(Window& sender)
{
    // dummy
}

void Window::EventListener::OnTimer(Window& sender, std::uint32_t timerID)
{
    // dummy
}


/* ----- Window class ----- */

#define FOREACH_LISTENER_CALL(FUNC) \
    for (const auto& lst : eventListeners_) { lst->FUNC; }

#ifdef LLGL_MOBILE_PLATFORM

std::unique_ptr<Window> Window::Create(const WindowDescriptor& desc)
{
    /* For mobile platforms this function always returns null */
    return nullptr;
}

#endif

void Window::SetBehavior(const WindowBehavior& behavior)
{
    behavior_ = behavior;
}

bool Window::HasFocus() const
{
    return focus_;
}

bool Window::HasQuit() const
{
    return quit_;
}

bool Window::AdaptForVideoMode(VideoModeDescriptor& videoModeDesc)
{
    /* Query current window descriptor */
    auto windowDesc = GetDesc();

    /* Adapt window size and position */
    windowDesc.size = videoModeDesc.resolution;

    if (videoModeDesc.fullscreen)
    {
        windowDesc.borderless   = true;
        windowDesc.position     = { 0, 0 };
    }
    else
    {
        windowDesc.borderless   = false;
        windowDesc.centered     = true;
    }

    /* Set new window descriptor and return with success */
    SetDesc(windowDesc);

    return true;
}

bool Window::ProcessEvents()
{
    FOREACH_LISTENER_CALL( OnProcessEvents(*this) );
    OnProcessEvents();
    return !HasQuit();
}

std::unique_ptr<Display> Window::FindResidentDisplay() const
{
    auto displayList = Display::InstantiateList();

    const auto winPos   = GetPosition();
    const auto winSize  = GetSize();
    const auto winArea  = static_cast<int>(winSize.width * winSize.height);

    for (auto& display : displayList)
    {
        auto offset = display->GetOffset();
        auto extent = display->GetDisplayMode().resolution;

        int scrX = static_cast<int>(extent.width);
        int scrY = static_cast<int>(extent.height);

        /* Calculate window boundaries relative to the current display */
        int x1 = winPos.x - offset.x;
        int y1 = winPos.y - offset.y;
        int x2 = x1 + static_cast<int>(winSize.width);
        int y2 = y1 + static_cast<int>(winSize.height);

        /* Is window fully or partially inside the dispaly? */
        if (x2 >= 0 && x1 <= scrX &&
            y2 >= 0 && y1 <= scrY)
        {
            /* Is at least the half of the window inside the display? */
            x1 = std::max(0, x1);
            y1 = std::max(0, y1);

            x2 = std::min(x2 - x1, scrX);
            y2 = std::min(y2 - y1, scrY);

            auto visArea = x2 * y2;

            if (visArea * 2 >= winArea)
                return std::move(display);
        }
    }

    return nullptr;
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

void Window::PostQuit()
{
    if (!HasQuit())
    {
        bool canQuit = true;
        for (const auto& lst : eventListeners_)
        {
            bool veto = false;
            lst->OnQuit(*this, veto);
            canQuit = (canQuit && !veto);
        }
        quit_ = canQuit;
    }
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

void Window::PostLocalMotion(const Offset2D& position)
{
    FOREACH_LISTENER_CALL( OnLocalMotion(*this, position) );
}

void Window::PostGlobalMotion(const Offset2D& motion)
{
    FOREACH_LISTENER_CALL( OnGlobalMotion(*this, motion) );
}

void Window::PostResize(const Extent2D& clientAreaSize)
{
    FOREACH_LISTENER_CALL( OnResize(*this, clientAreaSize) );
}

void Window::PostGetFocus()
{
    focus_ = true;
    FOREACH_LISTENER_CALL( OnGetFocus(*this) );
}

void Window::PostLostFocus()
{
    focus_ = false;
    FOREACH_LISTENER_CALL( OnLostFocus(*this) );
}

void Window::PostTimer(std::uint32_t timerID)
{
    FOREACH_LISTENER_CALL( OnTimer(*this, timerID) );
}

#undef FOREACH_LISTENER_CALL


} // /namespace LLGL



// ================================================================================
