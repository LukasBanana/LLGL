/*
 * AndroidCanvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_CANVAS_H
#define LLGL_ANDROID_CANVAS_H


#include <LLGL/Canvas.h>
#include <android/native_activity.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include <cstdint>


namespace LLGL
{


class AndroidCanvas : public Canvas
{

    public:

        AndroidCanvas(const CanvasDescriptor& desc);
        ~AndroidCanvas();

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

        Extent2D GetContentSize() const override;

        void SetTitle(const UTF8String& title) override;
        UTF8String GetTitle() const override;

    public:

        static std::int32_t OnAndroidAppInputEvent(android_app* app, AInputEvent* event);

    private:

        CanvasDescriptor    desc_;
        ANativeWindow*      window_         = nullptr;
        Extent2D            contentSize_;

};


} // /namespace LLGL


#endif



// ================================================================================
