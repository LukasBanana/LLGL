/*
 * GLRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_CONTEXT_H__
#define __LLGL_GL_RENDER_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include "OpenGL.h"

#if defined(_WIN32)
#   include "Win32/Win32GLPlatformContext.h"
#endif


namespace LLGL
{


class GLRenderContext : public RenderContext
{

    public:

        GLRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext);

        ~GLRenderContext();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        void Present() override;

    private:

        void CreateContext(GLRenderContext* sharedRenderContext);
        void DeleteContext();

        void QueryGLVerion(GLint& major, GLint& minor);

        #if defined(_WIN32)

        void DeleteGLContext(HGLRC& renderContext);

        HGLRC CreateGLContext(bool useExtProfile, GLRenderContext* sharedRenderContextGL = nullptr);
        HGLRC CreateStdContextProfile();
        HGLRC CreateExtContextProfile(HGLRC sharedGLRC = nullptr);

        void SetupDeviceContextAndPixelFormat();

        void SelectPixelFormat();
        bool SetupAntiAliasing();
        void CopyPixelFormat(GLRenderContext& sourceContext);

        bool SetupVSyncInterval();

        void RecreateWindow();

        #endif

        RenderContextDescriptor desc_;
        std::shared_ptr<Window> window_;

        GLPlatformContext       context_;

        //! Specifies whether this context uses a shared GL render context (true) or has its own hardware context (false).
        bool                    hasSharedContext_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
