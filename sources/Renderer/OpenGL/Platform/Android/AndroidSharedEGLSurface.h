/*
 * AndroidSharedEGLSurface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_SHARED_EGL_SURFACE_H
#define LLGL_ANDROID_SHARED_EGL_SURFACE_H


#include <EGL/egl.h>
#include <android/native_window.h>
#include <memory>


namespace LLGL
{


/*
Wrapper class for EGLSurface to be shared across multiple AndroidGLContext and AndroidGLSwapChainContext objects.
EGLSurface is shared because it is required when the initial EGLContext is made current.
By that time, we won't have a user created surface but we have the native android_app window.
So this surface is created with the first AndroidGLContext and is then shared with subsequently created AndroidGLSwapChainContext objects.
*/
class AndroidSharedEGLSurface
{

    public:

        AndroidSharedEGLSurface(EGLDisplay display, EGLConfig config, ANativeWindow* window);
        ~AndroidSharedEGLSurface();

    public:

        void InitEGLSurface(ANativeWindow* window);
        void DestroyEGLSurface();

        // Returns true if this EGL surface is a Pbuffer. This is the case if this surface was created without a native window.
        inline bool IsPbuffer() const
        {
            return (window_ == nullptr);
        }

        // Returns the native EGLSurface object.
        inline EGLSurface GetEGLSurface() const
        {
            return surface_;
        }

        // Returns the native ANativeWindow object. May be null.
        inline ANativeWindow* GetNativeWindow() const
        {
            return window_;
        }

    private:

        EGLDisplay      display_    = nullptr;
        EGLConfig       config_     = nullptr;
        EGLSurface      surface_    = nullptr;
        ANativeWindow*  window_     = nullptr;

};

using AndroidSharedEGLSurfacePtr = std::shared_ptr<AndroidSharedEGLSurface>;


} // /namespace LLGL


#endif



// ================================================================================
