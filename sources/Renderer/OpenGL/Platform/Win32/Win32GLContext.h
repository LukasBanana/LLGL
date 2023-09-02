/*
 * Win32GLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_GL_CONTEXT_H
#define LLGL_WIN32_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Platform/NativeHandle.h>
#include <vector>


namespace LLGL
{


// Implementation of the <GLContext> interface for Windows and wrapper for a native WGL context.
class Win32GLContext final : public GLContext
{

    public:

        Win32GLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            Win32GLContext*                     sharedContext
        );
        ~Win32GLContext();

        int GetSamples() const override;

    public:

        // Select the pixel format for the specified surface to make it compatible with this GL context.
        void SelectPixelFormat(Surface& surface);

        // Returns the OpenGL render context handle.
        inline HGLRC GetGLRCHandle() const
        {
            return hGLRC_;
        }

        // Returns the Win32 device context handle this GL context was originally created with.
        inline HDC GetDCHandle() const
        {
            return hDC_;
        }

    private:

        bool SetSwapInterval(int interval) override;

    private:

        void CreateContext(Surface& surface, Win32GLContext* sharedContext = nullptr);

        HGLRC CreateStandardWGLContext(HDC hDC);
        HGLRC CreateExplicitWGLContext(HDC hDC, Win32GLContext* sharedContext = nullptr);

        bool SelectMultisampledPixelFormat(HDC hDC);
        void CopyPixelFormat(Win32GLContext& sourceContext);

        HDC UpdateSurfacePixelFormat(Surface& surface);

    private:

        static const UINT           maxPixelFormatsMS                   = 8;

        RendererConfigurationOpenGL profile_;
        GLPixelFormat               formatDesc_;

        int                         pixelFormat_                        = 0;
        int                         pixelFormatsMS_[maxPixelFormatsMS]  = {};
        UINT                        pixelFormatsMSCount_                = 0;

        HDC                         hDC_                                = nullptr;
        HGLRC                       hGLRC_                              = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
