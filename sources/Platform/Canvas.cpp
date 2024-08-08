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
    void*                                       userData        = nullptr;
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

void Canvas::EventListener::OnQuit(Canvas& sender, bool& veto)
{
    // deprecated
}

void Canvas::EventListener::OnInit(Canvas& sender)
{
    // dummy
}

void Canvas::EventListener::OnDestroy(Canvas& sender)
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

LLGL_DEPRECATED_IGNORE_PUSH()

//deprecated
void Canvas::EventListener::OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy)
{
    // dummy
}

void Canvas::EventListener::OnPanGesture(Canvas& sender, const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action)
{
    OnPanGesture(sender, position, numTouches, dx, dy); // forward call to deprecated function until it's removed
}

LLGL_DEPRECATED_IGNORE_POP()

void Canvas::EventListener::OnKeyDown(Canvas& sender, Key keyCode)
{
    // dummy
}

void Canvas::EventListener::OnKeyUp(Canvas& sender, Key keyCode)
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
    return false; // deprecated
}

bool Canvas::AdaptForVideoMode(Extent2D* resolution, bool* fullscreen)
{
    /* Default implementation of this function always return false for the Canvas class */
    return false;
}

Display* Canvas::FindResidentDisplay() const
{
    return Display::GetPrimary();
}

void Canvas::SetUserData(void* userData)
{
    pimpl_->userData = userData;
}

void* Canvas::GetUserData() const
{
    return pimpl_->userData;
}

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
    // deprecated
}

void Canvas::PostInit()
{
    FOREACH_LISTENER_CALL( OnInit(*this) );
}

void Canvas::PostDestroy()
{
    FOREACH_LISTENER_CALL( OnDestroy(*this) );
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

//deprecated
void Canvas::PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy)
{
    PostPanGesture(position, numTouches, dx, dy, EventAction::Changed); // forward call to new version until this version is removed
}

void Canvas::PostPanGesture(const Offset2D& position, std::uint32_t numTouches, float dx, float dy, EventAction action)
{
    FOREACH_LISTENER_CALL( OnPanGesture(*this, position, numTouches, dx, dy, action) );
}

void Canvas::PostKeyDown(Key keyCode)
{
    FOREACH_LISTENER_CALL( OnKeyDown(*this, keyCode) );
}

void Canvas::PostKeyUp(Key keyCode)
{
    FOREACH_LISTENER_CALL( OnKeyUp(*this, keyCode) );
}

#undef FOREACH_LISTENER_CALL


} // /namespace LLGL



// ================================================================================
