/*
 * Win32GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_WIN32_GL_CONTEXT_H__
#define __LLGL_WIN32_GL_CONTEXT_H__


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/Platform/NativeHandle.h>
#include <vector>


namespace LLGL
{


class Win32GLContext : public GLContext
{

    public:

        Win32GLContext(RenderContextDescriptor& desc, Window& window, Win32GLContext* sharedContext);
        ~Win32GLContext();

        bool SetSwapInterval(int interval) override;

        bool SwapBuffers() override;

    private:

        struct GLPlatformContext
        {
            static const UINT   maxNumPixelFormatsMS    = 8;

            int                 pixelFormat             = 0;    //!< Standard pixel format.
            std::vector<int>    pixelFormatsMS;                 //!< Multi-sampled pixel formats.

            HDC                 hDC                     = 0;    //!< Device context handle.
            HGLRC               hGLRC                   = 0;    //!< OpenGL render context handle.
        };

        bool Activate(bool activate) override;

        void CreateContext(Win32GLContext* sharedContext);
        void DeleteContext();

        void DeleteGLContext(HGLRC& renderContext);

        HGLRC CreateGLContext(bool useExtProfile, Win32GLContext* sharedContext = nullptr);
        HGLRC CreateStdContextProfile();
        HGLRC CreateExtContextProfile(HGLRC sharedGLRC = nullptr);

        void SetupDeviceContextAndPixelFormat();

        void SelectPixelFormat();
        bool SetupAntiAliasing();
        void CopyPixelFormat(Win32GLContext& sourceContext);

        void RecreateWindow();

        RenderContextDescriptor&    desc_;
        Window&                     window_;

        GLPlatformContext           context_;

        bool                        hasSharedContext_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
