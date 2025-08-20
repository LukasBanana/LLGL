/*
 * LinuxGLContextWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_CONTEXT_WAYLAND_H
#define LLGL_LINUX_GL_CONTEXT_WAYLAND_H


#include "LinuxGLContext.h"
#include "../../OpenGL.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>

#include "LinuxSharedEGLSurface.h"


namespace LLGL
{

// Implementation of the <LinuxGLContext> interface for GNU/Linux and wrapper for a native EGL context.
class LinuxGLContextWayland : public LinuxGLContext
{

    public:

        LinuxGLContextWayland(
            const GLPixelFormat&                    pixelFormat,
            const RendererConfigurationOpenGL&      profile,
            Surface&                                surface,
            LinuxGLContextWayland*                  sharedContext,
            const OpenGL::RenderSystemNativeHandle* customNativeHandle
        );
        ~LinuxGLContextWayland();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

        OpenGL::RenderSystemNativeType GetNativeType() const override;

    public:

        // Returns the native EGL display.
        inline EGLDisplay GetEGLDisplay() const
        {
            return display_;
        }

        // Returns the native EGL configuration.
        inline EGLConfig GetEGLConfig() const
        {
            return config_;
        }

        // Returns the native EGL context.
        inline EGLContext GetEGLContext() const
        {
            return context_;
        }

        /*
        Returns the shared EGLSurface object. This is primarily associated with LinuxGLSwapChainContext,
        but we need a surface for the initial EGLContext when it's made current.
        */
        inline const LinuxSharedEGLSurfacePtr& GetSharedEGLSurface() const
        {
            return sharedSurface_;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

        bool SelectConfig(const GLPixelFormat& pixelFormat);

        EGLContext CreateEGLContextCoreProfile(EGLContext glcShared, int major, int minor, int depthBits, int stencilBits, EGLConfig* config);
        EGLContext CreateEGLContextCompatibilityProfile(EGLContext glcShared, EGLConfig* config);

        void CreateEGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            const NativeHandle&                 nativeHandle,
            LinuxGLContextWayland*              sharedContext
        );

        void DeleteEGLContext();

        void CreateProxyEGLContext(
            const GLPixelFormat&                    pixelFormat,
            const NativeHandle&                     nativeWindowHandle,
            const OpenGL::RenderSystemNativeHandle& nativeContextHandle
        );

    private:
        LinuxSharedEGLSurfacePtr sharedSurface_;
        EGLDisplay  display_;
        EGLContext  context_;
        EGLConfig   config_;
        int         samples_    = 1;
        bool        isProxyGLC_ = false;

};


} // /namespace LLGL



#endif



// ================================================================================