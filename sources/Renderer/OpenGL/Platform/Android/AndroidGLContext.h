/*
 * AndroidGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
