/*
 * MacOSGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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


// Implementation of the <GLContext> interface for MacOS and wrapper for a native NSGL context.
class MacOSGLContext : public GLContext
{

    public:

        MacOSGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            MacOSGLContext*                     sharedContext
        );
        ~MacOSGLContext();

        void Resize(const Extent2D& resolution) override;
        int GetSamples() const override;

    public:

        // Returns the native NSOpenGLContext object.
        inline NSOpenGLContext* GetNSGLContext() const
        {
            return ctx_;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

        bool CreatePixelFormat(const GLPixelFormat& pixelFormat, const RendererConfigurationOpenGL& profile);

        void CreateNSGLContext(MacOSGLContext* sharedContext);
        void DeleteNSGLContext();

    private:

        NSOpenGLPixelFormat*    pixelFormat_    = nullptr;
        NSOpenGLContext*        ctx_            = nullptr;
        int                     samples_        = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
