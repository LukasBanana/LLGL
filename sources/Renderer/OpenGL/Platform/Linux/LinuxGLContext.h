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
        inline ::GLXContext GetGLXContext() const
        {
            return glc_;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

        void CreateGLXContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            const NativeHandle&                 nativeHandle,
            LinuxGLContext*                     sharedContext
        );

        void DeleteGLXContext();

        GLXContext CreateGLXContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits);
        GLXContext CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared);

        void CreateProxyContext(
            const GLPixelFormat&                    pixelFormat,
            const NativeHandle&                     nativeWindowHandle,
            const OpenGL::RenderSystemNativeHandle& nativeContextHandle
        );

    private:

        ::Display*      display_    = nullptr;
        ::GLXContext    glc_        = nullptr;
        int             samples_    = 1;
        bool            isProxyGLC_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
