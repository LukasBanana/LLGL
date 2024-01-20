/*
 * IOSGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_GL_CONTEXT_H
#define LLGL_IOS_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"

#import <OpenGLES/EAGL.h>


namespace LLGL
{


// Implementation of the <GLContext> interface for Android and wrapper for a native EGL context.
class IOSGLContext : public GLContext
{

    public:

        IOSGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            IOSGLContext*                       sharedContext
        );
        ~IOSGLContext();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Returns the native EAGL context.
        inline EAGLContext* GetEAGLContext() const
        {
            return context_;
        }

        // Returns the initial GL pixel format.
        inline const GLPixelFormat& GetPixelFormat() const
        {
            return pixelFormat_;
        }

    private:

        bool SetSwapInterval(int interval) override;

        void CreateContext(
            const RendererConfigurationOpenGL&  profile,
            IOSGLContext*                       sharedContext
        );
        void DeleteContext();

    private:

        EAGLContext*    context_    = nullptr;
        GLPixelFormat   pixelFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
