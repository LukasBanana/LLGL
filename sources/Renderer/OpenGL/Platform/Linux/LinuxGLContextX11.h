/*
 * LinuxGLContext.h
 *
 * Copyright (c) 2025 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_CONTEXT_X11_H
#define LLGL_LINUX_GL_CONTEXT_X11_H


#include "LinuxGLContext.h"
#include "../../OpenGL.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>


namespace LLGL
{

// Implementation of the <LinuxGLContext> interface for GNU/Linux and wrapper for a native GLX context.
class LinuxGLContextX11 : public LinuxGLContext
{

    public:

        LinuxGLContextX11(
            const GLPixelFormat&                    pixelFormat,
            const RendererConfigurationOpenGL&      profile,
            Surface&                                surface,
            LinuxGLContextX11*                      sharedContext,
            const OpenGL::RenderSystemNativeHandle* customNativeHandle
        );
        ~LinuxGLContextX11();

        int GetSamples() const override {
            return samples_;
        }

        inline bool IsWayland() const override {
            return false;
        }

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Tries to find an X11 visual configuration for the specified pixel format and
        // modifies the sample count depending on availability. Returns null if no such visual could be found.
        static ::XVisualInfo* ChooseVisual(::Display* display, int screen, const GLPixelFormat& pixelFormat, int& outSamples);

        // Returns the native X11 <GLXContext> object.
        inline void* GetGLXContext() const
        {
            return context_;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

        void CreateGLXContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            const NativeHandle&                 nativeHandle,
            LinuxGLContextX11*                  sharedContext
        );

        void DeleteGLXContext();

        GLXContext CreateGLXContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits);
        GLXContext CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared);

        void CreateProxyGLXContext(
            const GLPixelFormat&                    pixelFormat,
            const NativeHandle&                     nativeWindowHandle,
            const OpenGL::RenderSystemNativeHandle& nativeContextHandle
        );

    private:
        ::Display*       display_;
        ::GLXContext     context_;

        int        samples_    = 1;
        bool       isProxyGLC_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
