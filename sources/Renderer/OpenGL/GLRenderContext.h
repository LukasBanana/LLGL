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
#include "GLStateManager.h"

#if defined(_WIN32)
#   include "Win32/Win32GLPlatformContext.h"
#endif


namespace LLGL
{


class GLRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext);
        ~GLRenderContext();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        void Present() override;

        /* ----- Rendering ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- GLRenderContext specific functions ----- */

        static bool GLMakeCurrent(GLRenderContext* renderContext);

    private:

        void CreateContext(GLRenderContext* sharedRenderContext);
        void DeleteContext();

        void AcquireStateManager(GLRenderContext* sharedRenderContext);
        void InitRenderStates();

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

        RenderContextDescriptor         desc_;

        GLPlatformContext               context_;

        //! Specifies whether this context uses a shared GL render context (true) or has its own hardware context (false).
        bool                            hasSharedContext_   = false;

        std::shared_ptr<GLStateManager> stateMngr_;

};


} // /namespace LLGL


#endif



// ================================================================================
