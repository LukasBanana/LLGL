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

#undef FOREACH_LISTENER_CALL


} // /namespace LLGL



// ================================================================================
