/*
 * Canvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Canvas.h>
#include "../Core/CoreUtils.h"
#include <vector>


namespace LLGL
{


struct Canvas::Pimpl
{
    std::vector<std::shared_ptr<EventListener>> eventListeners;
    bool                                        quit            = false;
};

Canvas::Canvas() :
    pimpl_ { new Pimpl{} }
{
}

Canvas::~Canvas()
{
    delete pimpl_;
}


/* ----- Canvas EventListener class ----- */

void Canvas::EventListener::OnProcessEvents(Canvas& sender)
{
    // dummy
}

void Canvas::EventListener::OnQuit(Canvas& sender, bool& veto)
{
    // dummy
}

void Canvas::EventListener::OnDraw(Canvas& sender)
{
    // dummy
}

void Canvas::EventListener::OnResize(Canvas& sender, const Extent2D& clientAreaSize)
{
    // dummy
}

void Canvas::EventListener::OnTapGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches)
{
    // dummy
}

void Canvas::EventListener::OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy)
{
    // dummy
}


/* ----- Window class ----- */

#define FOREACH_LISTENER_CALL(FUNC) \
    for (const auto& lst : pimpl_->eventListeners) { lst->FUNC; }

#ifndef LLGL_MOBILE_PLATFORM

std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    /* For non-mobile platforms this function always returns null */
    return nullptr;
}

#endif

bool Canvas::HasQuit() const
{
    return pimpl_->quit;
}

bool Canvas::AdaptForVideoMode(Extent2D* resolution, bool* fullscreen)
{
    /* Default implementation of this function always return false for the Canvas class */
    return false;
}

bool Canvas::ProcessEvents()
{
    FOREACH_LISTENER_CALL( OnProcessEvents(*this) );
    OnProcessEvents();
    return !HasQuit();
}

Display* Canvas::FindResidentDisplay() const
{
    return Display::GetPrimary();
}

/* --- Event handling --- */

void Canvas::AddEventListener(const std::shared_ptr<EventListener>& eventListener)
{
    AddOnceToSharedList(pimpl_->eventListeners, eventListener);
}

void Canvas::RemoveEventListener(const EventListener* eventListener)
{
    RemoveFromSharedList(pimpl_->eventListeners, eventListener);
}

void Canvas::PostQuit()
{
    if (!HasQuit())
    {
        bool canQuit = true;
        for (const auto& lst : pimpl_->eventListeners)
        {
            bool veto = false;
            lst->OnQuit(*this, veto);
            canQuit = (canQuit && !veto);
        }
        pimpl_->quit = canQuit;
    }
}

void Canvas::PostDraw()
{
    FOREACH_LISTENER_CALL( OnDraw(*this) );
}

void Canvas::PostResize(const Extent2D& clientAreaSize)
{
    FOREACH_LISTENER_CALL( OnResize(*this, clientAreaSize) );
}

void Canvas::PostTapGesture(const Offset2D& position, std::uint32_t numTouches)
{
    FOREACH_LISTENER_CALL( OnTapGesture(*this, position, numTouches) );
}

void Canvas::PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy)
{
    FOREACH_LISTENER_CALL( OnPanGesture(*this, position, numTouches, dx, dy) );
}

#undef FOREACH_LISTENER_CALL


} // /namespace LLGL



// ================================================================================
