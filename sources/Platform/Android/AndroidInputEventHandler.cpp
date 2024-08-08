/*
 * AndroidInputEventHandler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidInputEventHandler.h"
#include "AndroidApp.h"
#include "AndroidCanvas.h"
#include "AndroidKeyCodes.h"
#include "../../Core/CoreUtils.h"
#include <android/input.h>
#include <android/native_activity.h>
#include <mutex>
#include <float.h>


namespace LLGL
{


AndroidInputEventHandler& AndroidInputEventHandler::Get()
{
    static AndroidInputEventHandler instance;
    return instance;
}

void AndroidInputEventHandler::RegisterCanvas(AndroidCanvas* canvas)
{
    std::lock_guard<std::mutex> guard{ lock_ };
    canvases_.push_back(canvas);
}

void AndroidInputEventHandler::UnregisterCanvas(AndroidCanvas* canvas)
{
    std::lock_guard<std::mutex> guard{ lock_ };
    RemoveFromList(canvases_, canvas);
}

#define FOREACH_CANVAS_CALL(FUNC) \
    do for (AndroidCanvas* canvas : canvases_) { canvas->FUNC; } while (false)

void AndroidInputEventHandler::BroadcastCommand(android_app* app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
        {
            FOREACH_CANVAS_CALL(UpdateNativeWindow(app));
            FOREACH_CANVAS_CALL(PostInit());
        }
        break;

        case APP_CMD_TERM_WINDOW:
        {
            FOREACH_CANVAS_CALL(PostDestroy());
            FOREACH_CANVAS_CALL(UpdateNativeWindow(nullptr));
        }
        break;

        case APP_CMD_WINDOW_REDRAW_NEEDED:
        {
            FOREACH_CANVAS_CALL(PostDraw());
        }
        break;

        case APP_CMD_WINDOW_RESIZED:
        {
            const Extent2D contentSize = AndroidApp::GetContentRectSize(app);
            FOREACH_CANVAS_CALL(PostResize(contentSize));
        }
        break;

        case APP_CMD_LOST_FOCUS:
        {
            //TODO: send event to global app event handler
        }
        break;

        case APP_CMD_GAINED_FOCUS:
        {
            //TODO: send event to global app event handler
        }
        break;
    }
}

std::int32_t AndroidInputEventHandler::BroadcastInputEvent(android_app* app, AInputEvent* event)
{
    std::lock_guard<std::mutex> guard{ lock_ };

    switch (AInputEvent_getType(event))
    {
        case AINPUT_EVENT_TYPE_KEY:
        {
            const std::int32_t keyCode = AKeyEvent_getKeyCode(event);
            const Key key = MapAndroidKeyCode(keyCode);

            switch (AKeyEvent_getAction(event))
            {
                case AKEY_EVENT_ACTION_DOWN:
                {
                    FOREACH_CANVAS_CALL(PostKeyDown(key));
                }
                break;

                case AKEY_EVENT_ACTION_UP:
                {
                    FOREACH_CANVAS_CALL(PostKeyUp(key));
                }
                break;
            }
        }
        return 1;

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


} // /namespace LLGL



// ================================================================================
