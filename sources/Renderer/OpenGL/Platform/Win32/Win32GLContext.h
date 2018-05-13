/*
 * Win32GLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_WIN32_GL_CONTEXT_H
#define LLGL_WIN32_GL_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <LLGL/Platform/NativeHandle.h>
#include <vector>


namespace LLGL
{


class Win32GLContext : public GLContext
{

    public:

        Win32GLContext(RenderContextDescriptor& desc, Surface& surface, Win32GLContext* sharedContext);
        ~Win32GLContext();

        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        void Resize(const Size& resolution) override;

    private:

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

        static const UINT           maxNumPixelFormatsMS_   = 8;

        int                         pixelFormat_            = 0;    //!< Standard pixel format.
        std::vector<int>            pixelFormatsMS_;                 //!< Multi-sampled pixel formats.

        HDC                         hDC_                    = 0;    //!< Device context handle.
        HGLRC                       hGLRC_                  = 0;    //!< OpenGL render context handle.

        RenderContextDescriptor&    desc_;
        Surface&                    surface_;

        bool                        hasSharedContext_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
