/*
 * LinuxGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_CONTEXT_H
#define LLGL_LINUX_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include <X11/Xlib.h>


namespace LLGL
{


// Implementation of the <GLContext> interface for GNU/Linux and wrapper for a native GLX context.
class LinuxGLContext : public GLContext
{

    public:

        LinuxGLContext(
            const GLPixelFormat&                    pixelFormat,
            const RendererConfigurationOpenGL&      profile,
            Surface&                                surface,
            LinuxGLContext*                         sharedContext,
            const OpenGL::RenderSystemNativeHandle* customNativeHandle
        );
        ~LinuxGLContext();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Tries to find an X11 visual configuration for the specified pixel format and
        // modifies the sample count depending on availability. Returns null if no such visual could be found.
        static ::XVisualInfo* ChooseVisual(::Display* display, int screen, const GLPixelFormat& pixelFormat, int& outSamples);

        // Returns the native X11 <GLXContext> object.
        inline void* GetGLXContext() const
        {
            return api_.glx.context;
        }

        #ifdef LLGL_LINUX_ENABLE_WAYLAND

        inline EGLConfig GetEGLConfig() const {
            return api_.egl.config;
        }

        #endif

    private:

        bool SetSwapInterval(int interval) override;

    private:

        void CreateGLXContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            const NativeHandle&                 nativeHandle,
            LinuxGLContext*                     sharedContext
        );

        #ifdef LLGL_LINUX_ENABLE_WAYLAND
        void CreateEGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            const NativeHandle&                 nativeHandle,
            LinuxGLContext*                     sharedContext
        );
        #endif

        void DeleteGLXContext();
        #ifdef LLGL_LINUX_ENABLE_WAYLAND
        void DeleteEGLContext();
        #endif

        GLXContext CreateGLXContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits);
        GLXContext CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared);

        EGLContext CreateEGLContextCoreProfile(EGLContext glcShared, int major, int minor, int depthBits, int stencilBits, EGLConfig* config);
        EGLContext CreateEGLContextCompatibilityProfile(EGLContext glcShared, EGLConfig* config);

        void CreateProxyGLXContext(
            const GLPixelFormat&                    pixelFormat,
            const NativeHandle&                     nativeWindowHandle,
            const OpenGL::RenderSystemNativeHandle& nativeContextHandle
        );

        void CreateProxyEGLContext(
            const GLPixelFormat&                    pixelFormat,
            const NativeHandle&                     nativeWindowHandle,
            const OpenGL::RenderSystemNativeHandle& nativeContextHandle
        );

    private:
        enum class APIType : char {
            GLX,
            EGL
        };

        struct GLXData {
            ::Display*       display;
            ::GLXContext     context;
        };

        #ifdef LLGL_LINUX_ENABLE_WAYLAND

        struct EGLData {
            EGLDisplay       display;
            EGLContext       context;
            EGLConfig        config;
        };

        struct ApiData {
            union {
                GLXData glx;
                EGLData egl;
            };

            APIType type;             
        } api_;

        #else
        
        struct ApiData {
            GLXData glx;
        } api_;

        #endif


        int        samples_    = 1;
        bool       isProxyGLC_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
