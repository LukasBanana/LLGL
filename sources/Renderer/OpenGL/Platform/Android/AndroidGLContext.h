/*
 * AndroidGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_GL_CONTEXT_H
#define LLGL_ANDROID_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <EGL/egl.h>


namespace LLGL
{


// Implementation of the <GLContext> interface for Android and wrapper for a native EGL context.
class AndroidGLContext : public GLContext
{

    public:

        AndroidGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            AndroidGLContext*                   sharedContext
        );
        ~AndroidGLContext();

        void Resize(const Extent2D& resolution) override;
        int GetSamples() const override;

    public:

        // Returns the native EGL display.
        inline ::EGLDisplay GetEGLDisplay() const
        {
            return display_;
        }

        // Returns the native EGL context.
        inline ::EGLContext GetEGLContext() const
        {
            return context_;
        }

        // Returns the native EGL configuration.
        inline ::EGLConfig GetEGLConfig() const
        {
            return config_;
        }

    private:

        bool SetSwapInterval(int interval) override;

        bool SelectConfig(const GLPixelFormat& pixelFormat);

        void CreateContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            AndroidGLContext*                   sharedContext
        );
        void DeleteContext();

    private:

        ::EGLDisplay    display_    = nullptr;
        ::EGLContext    context_    = nullptr;
        ::EGLConfig     config_     = nullptr;
        int             samples_    = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
