/*
 * MacOSGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_GL_CONTEXT_H
#define LLGL_MACOS_GL_CONTEXT_H


#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include "../GLContext.h"


namespace LLGL
{


class MacOSGLContext : public GLContext
{

    public:

        MacOSGLContext(
            const RenderContextDescriptor&      desc,
            const RendererConfigurationOpenGL&  config,
            Surface&                            surface,
            MacOSGLContext*                     sharedContext
        );
        ~MacOSGLContext();

        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    private:

        bool Activate(bool activate) override;

        void CreatePixelFormat(const RenderContextDescriptor& desc, const RendererConfigurationOpenGL& config);

        void CreateNSGLContext(const NativeHandle& nativeHandle, MacOSGLContext* sharedContext);
        void DeleteNSGLContext();

    private:

        NSOpenGLPixelFormat*    pixelFormat_    = nullptr;
        NSOpenGLContext*        ctx_            = nullptr;
        NSWindow*               wnd_            = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
