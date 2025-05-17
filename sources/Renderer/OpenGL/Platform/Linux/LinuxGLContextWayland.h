/*
 * LinuxGLContext.h
 *
 * Copyright (c) 2025 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */


#if LLGL_LINUX_ENABLE_WAYLAND

#ifndef LLGL_LINUX_GL_CONTEXT_WAYLAND_H
#define LLGL_LINUX_GL_CONTEXT_WAYLAND_H


#include "LinuxGLContext.h"
#include "../../OpenGL.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>


namespace LLGL
{

enum class LinuxGLAPIType : char {
    GLX,
    EGL
};

// Implementation of the <GLContext> interface for GNU/Linux and wrapper for a native GLX context.
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

        int GetSamples() const override {
            return samples_;
        }

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        inline EGLConfig GetEGLConfig() const
        {
            return config_;
        }

        inline EGLContext GetEGLContext() const
        {
            return context_;
        }

        inline bool IsWayland() const override {
            return true;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

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
        EGLDisplay       display_;
        EGLContext       context_;
        EGLConfig        config_;

        int        samples_    = 1;
        bool       isProxyGLC_ = false;

};


} // /namespace LLGL


#endif // LLGL_LINUX_GL_CONTEXT_WAYLAND_H

#endif // LLGL_LINUX_ENABLE_WAYLAND

// ================================================================================
