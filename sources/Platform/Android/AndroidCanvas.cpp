/*
 * AndroidCanvas.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidCanvas.h"
#include "AndroidApp.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    return MakeUnique<AndroidCanvas>(desc);
}

static Extent2D GetAndroidWindowRect()
{
    if (auto app = AndroidApp::Get().GetState())
    {
        return Extent2D
        {
            static_cast<std::uint32_t>(app->contentRect.right - app->contentRect.left),
            static_cast<std::uint32_t>(app->contentRect.bottom - app->contentRect.top)
        };
    }
    return Extent2D{};
}

AndroidCanvas::AndroidCanvas(const CanvasDescriptor& desc) :
    desc_        { desc                                 },
    window_      { AndroidApp::Get().GetState()->window },
    contentSize_ { GetAndroidWindowRect()               }
{
}

AndroidCanvas::~AndroidCanvas()
{
}

bool AndroidCanvas::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandleSize == sizeof(NativeHandle))
    {
        auto& handle = *reinterpret_cast<NativeHandle*>(nativeHandle);
        handle.window = window_;
        return true;
    }
    return false;
}

Extent2D AndroidCanvas::GetContentSize() const
{
    return contentSize_;
}

void AndroidCanvas::SetTitle(const UTF8String& title)
{
    //todo...
}

UTF8String AndroidCanvas::GetTitle() const
{
    return {}; //todo...
}

void AndroidCanvas::ResetPixelFormat()
{
    // dummy
}


/*
 * ======= Private: =======
 */

void AndroidCanvas::OnProcessEvents()
{
    android_app* appState = AndroidApp::Get().GetState();

    /* Poll all Androdi app events */
    int ident = 0, events = 0;
    android_poll_source* source = nullptr;

    while ((ident = ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
    {
        /* Process the event */
        if (source != nullptr)
            source->process(appState, source);

        /* Process sensor data */
        /*if (ident == LOOPER_ID_USER)
        {
            //TODO
        }*/

        /* Check if we are exiting */
        if (appState->destroyRequested != 0)
            PostQuit();
    }
}


} // /namespace LLGL



// ================================================================================
