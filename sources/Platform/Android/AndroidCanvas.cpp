/*
 * AndroidCanvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidCanvas.h"
#include "AndroidApp.h"
#include "AndroidInputEventHandler.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    if (android_app* app = AndroidApp::Get().GetState())
    {
        /* Poll all Android app events */
        int ident = 0, events = 0;
        android_poll_source* source = nullptr;

        while ((ident = ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
        {
            /* Process the event */
            if (source != nullptr)
                source->process(app, source);

            /* Check if we are exiting */
            if (app->destroyRequested != 0)
                return false;
        }
        return true;
    }
    return false;
}


/*
 * Canvas class
 */

std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    return MakeUnique<AndroidCanvas>(desc);
}


// LLGL's own windowing is driven by the "native app glue" event loop, so a Canvas is only
// available to an application that supplied RenderSystemDescriptor::androidApp.  One that
// brings its own Surface - or whose Activity is written in Java and has no app state to give -
// never reaches here.
static ANativeWindow* GetNativeWindowFromAppState()
{
    android_app* appState = AndroidApp::Get().GetState();
    LLGL_ASSERT(appState != nullptr, "LLGL::Canvas on Android requires RenderSystemDescriptor::androidApp");
    return appState->window;
}

/*
 * AndroidCanvas class
 */

AndroidCanvas::AndroidCanvas(const CanvasDescriptor& desc) :
    desc_   { desc                                 },
    window_ { GetNativeWindowFromAppState() }
{
    AndroidInputEventHandler::Get().RegisterCanvas(this);
}

AndroidCanvas::~AndroidCanvas()
{
    AndroidInputEventHandler::Get().UnregisterCanvas(this);
}

bool AndroidCanvas::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        auto* handle = static_cast<NativeHandle*>(nativeHandle);
        handle->window = window_;
        return true;
    }
    return false;
}

Extent2D AndroidCanvas::GetContentSize() const
{
    return AndroidApp::GetContentRectSize(AndroidApp::Get().GetState());
}

void AndroidCanvas::SetTitle(const UTF8String& title)
{
    //todo...
}

UTF8String AndroidCanvas::GetTitle() const
{
    return {}; //todo...
}

void AndroidCanvas::UpdateNativeWindow(android_app* app)
{
    window_ = (app != nullptr ? app->window : nullptr);
}


} // /namespace LLGL



// ================================================================================
