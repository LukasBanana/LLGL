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
            const GLPixelFormat&                    pixelFormat,
            const RendererConfigurationOpenGL&      profile,
            Surface&                                surface,
            Win32GLContext*                         sharedContext,
            const OpenGL::RenderSystemNativeHandle* customNativeHandle
        );
        ~Win32GLContext();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Select the pixel format for the specified surface to make it compatible with this GL context.
        bool SelectPixelFormat(HDC hDC);

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

        void CreateProxyContext(Surface& surface, const OpenGL::RenderSystemNativeHandle& nativeHandle);

        void CreateWGLContext(Surface& surface, Win32GLContext* sharedContext = nullptr);

        HGLRC CreateStandardWGLContext(HDC hDC);
        HGLRC CreateExplicitWGLContext(HDC hDC, Win32GLContext* sharedContext = nullptr);

        bool SelectMultisampledPixelFormat(HDC hDC);
        void CopyPixelFormat(Win32GLContext& sourceContext);

        void ErrorMultisampleContextFailed();

    private:

        static constexpr UINT       maxPixelFormatsMS                   = 8;

        RendererConfigurationOpenGL profile_;
        GLPixelFormat               formatDesc_;

        int                         pixelFormat_                        = 0;
        int                         pixelFormatsMS_[maxPixelFormatsMS]  = {};
        UINT                        pixelFormatsMSCount_                = 0;

        HDC                         hDC_                                = nullptr;
        HGLRC                       hGLRC_                              = nullptr;

        const bool                  isProxyGLRC_                        = false; // true if a custom native handle was provided

};


} // /namespace LLGL


#endif



// ================================================================================
