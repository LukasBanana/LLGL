/*
 * AndroidInputEventHandler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_INPUT_EVENT_HANDLER_H
#define LLGL_ANDROID_INPUT_EVENT_HANDLER_H


#include <LLGL/Constants.h>
#include <android_native_app_glue.h>
#include <vector>
#include <mutex>
#include <cstdint>
#include <float.h>


namespace LLGL
{


class AndroidCanvas;

class AndroidInputEventHandler
{

    public:

        AndroidInputEventHandler(const AndroidInputEventHandler&) = delete;
        AndroidInputEventHandler& operator = (const AndroidInputEventHandler&) = delete;

        static AndroidInputEventHandler& Get();

        void RegisterCanvas(AndroidCanvas* canvas);
        void UnregisterCanvas(AndroidCanvas* canvas);

        void BroadcastCommand(android_app* app, int32_t cmd);
        std::int32_t BroadcastInputEvent(android_app* app, AInputEvent* event);

    private:

        struct MotionEvent
        {
            float prevPosition[2] = { -FLT_MAX, -FLT_MAX };
        };

    private:

        AndroidInputEventHandler() = default;

    private:

        std::mutex                  lock_;
        std::vector<AndroidCanvas*> canvases_;
        MotionEvent                 motionEvents_[LLGL_MAX_NUM_INPUT_POINTERS];

};


} // /namespace LLGL


#endif



// ================================================================================
