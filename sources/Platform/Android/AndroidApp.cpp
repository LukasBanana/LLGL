/*
 * AndroidApp.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidApp.h"
#include "AndroidInputEventHandler.h"
#include "../../Core/Assertion.h"
#include <thread>


namespace LLGL
{


typedef void (*PFN_ANDROID_APP_CMD)(android_app* app, int32_t cmd);

struct AndroidAppInit
{
    void*               clientUserData;
    PFN_ANDROID_APP_CMD clientOnAppCmd;
    bool                isWindowReady;
    bool                isContentReady;
};

static void AndroidAppLoopCmdCallback(android_app* app, int32_t cmd)
{
    AndroidInputEventHandler::Get().BroadcastCommand(app, cmd);
}

static std::int32_t AndroidAppLoopInputEventCallback(android_app* app, AInputEvent* event)
{
    return AndroidInputEventHandler::Get().BroadcastInputEvent(app, event);
}

static void AndroidAppInitCmdCallback(android_app* app, int32_t cmd)
{
    AndroidAppInit* init = reinterpret_cast<AndroidAppInit*>(app->userData);

    /* Check for window initialization */
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            init->isWindowReady = (app->window != nullptr);
            break;

        case APP_CMD_CONTENT_RECT_CHANGED:
            init->isContentReady = true;
            break;
    }

    /* Forward call to command callback from client code */
    if (init->clientOnAppCmd != nullptr)
    {
        app->userData = init->clientUserData;
        init->clientOnAppCmd(app, cmd);
        app->userData = init;
    }
}

static void WaitUntilNativeWindowIsInitialized(android_app* app)
{
    /* Store client data from app state */
    AndroidAppInit init = {};

    init.clientUserData = app->userData;
    init.clientOnAppCmd = app->onAppCmd;

    /* Poll all Android app events */
    app->userData = reinterpret_cast<void*>(&init);
    app->onAppCmd = AndroidAppInitCmdCallback;

    int ident = 0, events = 0;
    android_poll_source* source = nullptr;

    while (!init.isWindowReady || !init.isContentReady)
    {
        if ((ident = ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
        {
            /* Process the event */
            if (source != nullptr)
                source->process(app, source);

            /* Check if we are exiting */
            if (app->destroyRequested != 0)
                break;
        }
        else
        {
            /* If no event was processed, yield to other threads */
            std::this_thread::yield();
        }
    }

    if (init.clientOnAppCmd != nullptr)
    {
        /* Restore client data if it was previously specified */
        app->userData = init.clientUserData;
        app->onAppCmd = init.clientOnAppCmd;
    }
    else
    {
        /* ... Otherwise, use LLGL specific callback to handle window resize/rotation */
        app->userData = nullptr;
        app->onAppCmd = AndroidAppLoopCmdCallback;
    }
}

AndroidApp& AndroidApp::Get()
{
    static AndroidApp instance;
    return instance;
}

void AndroidApp::Initialize(const AndroidContext& context, android_app* state)
{
    context_ = context;

    /*
    The app state is optional, but a "native app glue" application must supply it: the pump below
    is what advances such an application through its startup lifecycle, and skipping it leaves the
    Activity in a state the platform - and any XR runtime waiting on it - never sees become ready.
    An application that drives its own event loop, such as one whose Activity is written in Java,
    has no app state to give and does not need this.
    */
    if (state == nullptr)
        return;

    state_ = state;

    /*
    Fill in anything the application left blank from the app state, so that one which supplies
    only androidApp - as every "native app glue" application did before AndroidContext existed -
    keeps working with no changes.
    */
    if (state_->activity != nullptr)
    {
        if (context_.applicationVM == nullptr)
            context_.applicationVM = state_->activity->vm;
        if (context_.applicationActivity == nullptr)
            context_.applicationActivity = state_->activity->clazz;
        if (context_.assetManager == nullptr)
            context_.assetManager = state_->activity->assetManager;
    }

    if (state_->window == nullptr)
    {
        /* Process events until native window is initialized (APP_CMD_INIT_WINDOW) */
        WaitUntilNativeWindowIsInitialized(state);
    }
    if (state_->onInputEvent == nullptr)
    {
        /* Set default event handler */
        state_->onInputEvent = AndroidAppLoopInputEventCallback;
    }
}

Extent2D AndroidApp::GetContentRectSize(android_app* appState)
{
    if (appState != nullptr)
    {
        return Extent2D
        {
            static_cast<std::uint32_t>(appState->contentRect.right - appState->contentRect.left),
            static_cast<std::uint32_t>(appState->contentRect.bottom - appState->contentRect.top)
        };
    }
    return Extent2D{};
}


} // /namespace LLGL



// ================================================================================
