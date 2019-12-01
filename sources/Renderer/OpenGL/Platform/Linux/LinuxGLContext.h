/*
 * LinuxGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
            const RenderContextDescriptor&      desc,
            const RendererConfigurationOpenGL&  config,
            Surface&                            surface,
            LinuxGLContext*                     sharedContext
        );
        ~LinuxGLContext();

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
            LinuxGLContext*                     sharedContext
        );
        void DeleteContext();

        GLXContext CreateContextCoreProfile(GLXContext glcShared, int major, int minor);
        GLXContext CreateContextCompatibilityProfile(GLXContext glcShared);

    private:

        ::Display*      display_    = nullptr;
        ::Window        wnd_        = 0;
        XVisualInfo*    visual_     = nullptr;
        GLXContext      glc_        = nullptr;
        std::uint32_t   samples_    = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
