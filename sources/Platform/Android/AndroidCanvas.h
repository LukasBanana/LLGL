/*
 * AndroidCanvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_CANVAS_H
#define LLGL_ANDROID_CANVAS_H


#include <LLGL/Canvas.h>
#include <android/input.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <cstdint>


namespace LLGL
{


class AndroidCanvas : public Canvas
{

    public:

        #include <LLGL/Backend/Canvas.inl>

    public:

        AndroidCanvas(const CanvasDescriptor& desc);
        ~AndroidCanvas();

        /*
        Updates the pointer to ANativeWindow from the specified app state.
        If the input is null, the window will be reset to null.
        */
        void UpdateNativeWindow(android_app* app);

    private:

        CanvasDescriptor    desc_;
        ANativeWindow*      window_         = nullptr;
        Extent2D            contentSize_;

};


} // /namespace LLGL


#endif



// ================================================================================
