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
#include <mutex>
#include <float.h>


namespace LLGL
{


static Extent2D GetAndroidContentSize()
{
    if (android_app* app = AndroidApp::Get().GetState())
    {
        return Extent2D
        {
            static_cast<std::uint32_t>(app->contentRect.right - app->contentRect.left),
            static_cast<std::uint32_t>(app->contentRect.bottom - app->contentRect.top)
        };
    }
    return Extent2D{};
}


/*
 * AndroidInputEventHandler
 */

class AndroidInputEventHandler
{

    public:

        AndroidInputEventHandler(const AndroidInputEventHandler&) = delete;
        AndroidInputEventHandler& operator = (const AndroidInputEventHandler&) = delete;

        static AndroidInputEventHandler& Get();

        void RegisterCanvas(Canvas* canvas);
        void UnregisterCanvas(Canvas* canvas);

        void BroadcastCommand(android_app* app, int32_t cmd);
        std::int32_t BroadcastInputEvent(android_app* app, AInputEvent* event);

    private:

        AndroidInputEventHandler() = default;

    private:

        std::mutex              lock_;
        std::vector<Canvas*>    canvases_;
        float                   prevMotionPos_[2] = { -FLT_MAX, -FLT_MAX };

};


AndroidInputEventHandler& AndroidInputEventHandler::Get()
{
    static AndroidInputEventHandler instance;
    return instance;
}

void AndroidInputEventHandler::RegisterCanvas(Canvas* canvas)
{
    std::lock_guard<std::mutex> guard{ lock_ };
    canvases_.push_back(canvas);
}

void AndroidInputEventHandler::UnregisterCanvas(Canvas* canvas)
{
    std::lock_guard<std::mutex> guard{ lock_ };
    RemoveFromList(canvases_, canvas);
}

#define FOREACH_CANVAS_CALL(FUNC) \
    do for (Canvas* canvas : canvases_) { canvas->FUNC; } while (false)

void AndroidInputEventHandler::BroadcastCommand(android_app* app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_TERM_WINDOW:
        {
            FOREACH_CANVAS_CALL(PostQuit());
        }
        break;

        case APP_CMD_WINDOW_REDRAW_NEEDED:
        {
            FOREACH_CANVAS_CALL(PostDraw());
        }
        break;

        case APP_CMD_WINDOW_RESIZED:
        {
            const Extent2D contentSize = GetAndroidContentSize();
            FOREACH_CANVAS_CALL(PostResize(contentSize));
        }
        break;
    }
}

std::int32_t AndroidInputEventHandler::BroadcastInputEvent(android_app* app, AInputEvent* event)
{
    std::lock_guard<std::mutex> guard{ lock_ };

    switch (AInputEvent_getType(event))
    {
        /*case AINPUT_EVENT_TYPE_KEY:
        {
            //TODO
        }
        return 1;*/

        case AINPUT_EVENT_TYPE_MOTION:
        {
            const float posX = AMotionEvent_getX(event, 0);
            const float posY = AMotionEvent_getY(event, 0);

            const LLGL::Offset2D position{ static_cast<std::int32_t>(posX), static_cast<std::int32_t>(posY) };
            const std::uint32_t numTouches = static_cast<std::uint32_t>(AMotionEvent_getPointerCount(event));

            switch (AMotionEvent_getAction(event))
            {
                case AMOTION_EVENT_ACTION_DOWN:
                {
                    /* Initialize previous position with current position when first touch down is detected */
                    prevMotionPos_[0] = posX;
                    prevMotionPos_[1] = posY;
                    FOREACH_CANVAS_CALL(PostPanGesture(position, numTouches, 0.0f, 0.0f, EventAction::Began));
                }
                break;

                case AMOTION_EVENT_ACTION_MOVE:
                {
                    /* Determine motion delta by difference between current and previous position */
                    const float posXPrec = AMotionEvent_getXPrecision(event);
                    const float posYPrec = AMotionEvent_getYPrecision(event);
                    const float dx = (posX - prevMotionPos_[0]) / posXPrec;
                    const float dy = (posY - prevMotionPos_[1]) / posYPrec;
                    prevMotionPos_[0] = posX;
                    prevMotionPos_[1] = posY;
                    FOREACH_CANVAS_CALL(PostPanGesture(position, numTouches, dx, dy, EventAction::Changed));
                }
                break;

                case AMOTION_EVENT_ACTION_UP:
                {
                    FOREACH_CANVAS_CALL(PostPanGesture(position, numTouches, 0.0f, 0.0f, EventAction::Ended));
                }
                break;
            }
        }
        return 1;
    }

    return 0;
}

#undef FOREACH_CANVAS_CALL


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

            /* Process sensor data */
            /*if (ident == LOOPER_ID_USER)
            {
                //TODO
            }*/

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


/*
 * AndroidCanvas class
 */

AndroidCanvas::AndroidCanvas(const CanvasDescriptor& desc) :
    desc_   { desc                                 },
    window_ { AndroidApp::Get().GetState()->window }
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
        auto* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        handle->window = window_;
        return true;
    }
    return false;
}

Extent2D AndroidCanvas::GetContentSize() const
{
    return GetAndroidContentSize();
}

void AndroidCanvas::SetTitle(const UTF8String& title)
{
    //todo...
}

UTF8String AndroidCanvas::GetTitle() const
{
    return {}; //todo...
}

void AndroidCanvas::OnAndroidAppCommand(android_app* app, int32_t cmd)
{
    AndroidInputEventHandler::Get().BroadcastCommand(app, cmd);
}

std::int32_t AndroidCanvas::OnAndroidAppInputEvent(android_app* app, AInputEvent* event)
{
    return AndroidInputEventHandler::Get().BroadcastInputEvent(app, event);
}


} // /namespace LLGL



// ================================================================================
