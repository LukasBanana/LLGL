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


struct NativeHandle;

// Implementation of the <GLContext> interface for Android and wrapper for a native EGL context.
class AndroidGLContext : public GLContext
{

    public:

        AndroidGLContext(
            const RenderContextDescriptor&      desc,
            const RendererConfigurationOpenGL&  config,
            Surface&                            surface,
            AndroidGLContext*                   sharedContext
        );
        ~AndroidGLContext();

        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;
        std::uint32_t GetSamples() const override;

    private:

        bool Activate(bool activate) override;

        void CreateContext(
            const RenderContextDescriptor&      contextDesc,
            const RendererConfigurationOpenGL&  config,
            const NativeHandle&                 nativeHandle,
            AndroidGLContext*                   sharedContext
        );
        void DeleteContext();

    private:

        EGLDisplay      display_    = nullptr;
        EGLContext      context_    = nullptr;
        EGLSurface      surface_    = nullptr;
        EGLConfig       config_     = nullptr;
        std::uint32_t   samples_    = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
