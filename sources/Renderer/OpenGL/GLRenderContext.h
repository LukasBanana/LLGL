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
#   include "Platform/Win32/Win32GLPlatformContext.h"
#elif defined(__linux__)
#   include "Platform/Linux/LinuxGLPlatformContext.h"
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

        /* ----- Configuration ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        void SetDrawMode(const DrawMode drawMode) override;

        /* ----- Hardware buffers ------ */

        void BindVertexBuffer(VertexBuffer& vertexBuffer) override;
        void UnbindVertexBuffer() override;

        void BindIndexBuffer(IndexBuffer& indexBuffer) override;
        void UnbindIndexBuffer() override;

        /* --- Drawing --- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices) override;
        void DrawIndexed(unsigned int numVertices, int indexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances) override;
        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset) override;
        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset, unsigned int instanceOffset) override;

        /* ----- Shader ----- */

        void BindShaderProgram(ShaderProgram& shaderProgram) override;
        void UnbindShaderProgram() override;

        void DispatchCompute(const Gs::Vector3ui& threadGroupSize) override;

        /* ----- GLRenderContext specific functions ----- */

        static bool GLMakeCurrent(GLRenderContext* renderContext);

    private:

        struct RenderState
        {
            GLenum drawMode             = GL_TRIANGLES;
            GLenum indexBufferDataType  = GL_UNSIGNED_INT;
        };

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
        RenderState                     renderState_;

};


} // /namespace LLGL


#endif



// ================================================================================
