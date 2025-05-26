/*
 * LinuxSharedEGLSurface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_SHARED_EGL_SURFACE_H
#define LLGL_LINUX_SHARED_EGL_SURFACE_H


#include <EGL/egl.h>
#include <memory>

#include <wayland-egl-core.h>


namespace LLGL
{


/*
Wrapper class for EGLSurface to be shared across multiple LinuxGLContext and LinuxGLSwapChainContext objects.
EGLSurface is shared because it is required when the initial EGLContext is made current.
So this surface is created with the first LinuxGLContext and is then shared with subsequently created LinuxGLSwapChainContext objects.
*/
class LinuxSharedEGLSurface
{

    public:

        LinuxSharedEGLSurface(EGLDisplay display, EGLConfig config, wl_egl_window* window);
        ~LinuxSharedEGLSurface();

    public:

        void InitEGLSurface(wl_egl_window* window);
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

        // Returns the native window. May be null.
        inline wl_egl_window* GetNativeWindow() const
        {
            return window_;
        }

    private:

        EGLDisplay      display_    = nullptr;
        EGLConfig       config_     = nullptr;
        EGLSurface      surface_    = nullptr;
        wl_egl_window*  window_     = nullptr;

};

using LinuxSharedEGLSurfacePtr = std::shared_ptr<LinuxSharedEGLSurface>;


} // /namespace LLGL


#endif



// ================================================================================
