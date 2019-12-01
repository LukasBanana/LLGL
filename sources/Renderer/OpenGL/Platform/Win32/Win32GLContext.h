/*
 * Win32GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
            const RenderContextDescriptor&      desc,
            const RendererConfigurationOpenGL&  config,
            Surface&                            surface,
            Win32GLContext*                     sharedContext
        );
        ~Win32GLContext();

        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;
        std::uint32_t GetSamples() const override;

    private:

        struct WGLContextParams
        {
            RendererConfigurationOpenGL profile;
            int                         colorBits   = 32;
            int                         depthBits   = 24;
            int                         stencilBits = 8;
        };

    private:

        bool Activate(bool activate) override;

        void CreateContext(const WGLContextParams& params, Win32GLContext* sharedContext = nullptr);
        void DeleteContext();

        void DeleteGLContext(HGLRC& renderContext);

        HGLRC CreateGLContext(const WGLContextParams& params, bool useExtProfile, Win32GLContext* sharedContext = nullptr);
        HGLRC CreateStdContextProfile();
        HGLRC CreateExtContextProfile(const WGLContextParams& params, HGLRC sharedGLRC = nullptr);

        void SetupDeviceContextAndPixelFormat(const WGLContextParams& params);
        void SelectPixelFormat(const WGLContextParams& params);
        bool SetupAntiAliasing(const WGLContextParams& params);
        void CopyPixelFormat(Win32GLContext& sourceContext);

        void RecreateWindow(const WGLContextParams& params);

    private:

        static const UINT   maxPixelFormatsMS                   = 8;

        int                 pixelFormat_                        = 0;        // Standard pixel format.
        int                 pixelFormatsMS_[maxPixelFormatsMS]  = {};       // Multi-sampled pixel formats.
        UINT                pixelFormatsMSCount_                = 0;
        int                 samples_                            = 1;

        HDC                 hDC_                                = 0;        // Device context handle.
        HGLRC               hGLRC_                              = 0;        // OpenGL render context handle.

        Surface&            surface_;

        bool                hasSharedContext_                   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
