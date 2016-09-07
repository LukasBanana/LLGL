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
#include "RenderState/GLStateManager.h"

#if defined(_WIN32)
#   include <LLGL/Platform/NativeHandle.h>
#   include "Platform/Win32/Win32GLPlatformContext.h"
#elif defined(__linux__)
#   include <LLGL/Platform/NativeHandle.h>
#   include "Platform/Linux/LinuxGLPlatformContext.h"
#endif


namespace LLGL
{


class GLRenderSystem;
class GLRenderTarget;

class GLRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        GLRenderContext(
            GLRenderSystem& renderSystem,
            RenderContextDescriptor desc,
            const std::shared_ptr<Window>& window,
            GLRenderContext* sharedRenderContext
        );
        ~GLRenderContext();

        void Present() override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        void SetViewports(const std::vector<Viewport>& viewports) override;
        void SetScissors(const std::vector<Scissor>& scissors) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        void SetDrawMode(const DrawMode drawMode) override;

        /* ----- Hardware Buffers ------ */

        void BindVertexBuffer(VertexBuffer& vertexBuffer) override;
        void UnbindVertexBuffer() override;

        void BindIndexBuffer(IndexBuffer& indexBuffer) override;
        void UnbindIndexBuffer() override;

        void BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index) override;
        void UnbindConstantBuffer(unsigned int index) override;

        void BindStorageBuffer(StorageBuffer& storageBuffer, unsigned int index) override;
        void UnbindStorageBuffer(unsigned int index) override;

        /* ----- Textures ----- */

        void BindTexture(unsigned int layer, Texture& texture) override;
        void UnbindTexture(unsigned int layer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ----- */

        void BindSampler(unsigned int layer, Sampler& sampler) override;
        void UnbindSampler(unsigned int layer) override;

        /* ----- Render Targets ----- */

        void BindRenderTarget(RenderTarget& renderTarget) override;
        void UnbindRenderTarget() override;

        /* ----- Pipeline States ----- */

        void BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        //void BindComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;

        /* ----- Drawing ----- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) override;
        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset) override;

        /* ----- Compute ----- */

        void DispatchCompute(const Gs::Vector3ui& threadGroupSize) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

        /* ----- GLRenderContext specific functions ----- */

        static bool GLMakeCurrent(GLRenderContext* renderContext);

    private:

        struct RenderState
        {
            GLenum      drawMode            = GL_TRIANGLES;
            GLenum      indexBufferDataType = GL_UNSIGNED_INT;
            GLintptr    indexBufferStride   = 4;
        };

        #ifndef __APPLE__
        void GetNativeContextHandle(NativeContextHandle& windowContext);
        #endif

        void CreateContext(GLRenderContext* sharedRenderContext);
        void DeleteContext();

        void AcquireStateManager(GLRenderContext* sharedRenderContext);
        void InitRenderStates();

        bool SetupVsyncInterval();

        #if defined(_WIN32)

        void DeleteGLContext(HGLRC& renderContext);

        HGLRC CreateGLContext(bool useExtProfile, GLRenderContext* sharedRenderContextGL = nullptr);
        HGLRC CreateStdContextProfile();
        HGLRC CreateExtContextProfile(HGLRC sharedGLRC = nullptr);

        void SetupDeviceContextAndPixelFormat();

        void SelectPixelFormat();
        bool SetupAntiAliasing();
        void CopyPixelFormat(GLRenderContext& sourceContext);

        void RecreateWindow();

        #endif

        GLRenderSystem&                 renderSystem_;  // reference to its render system
        RenderContextDescriptor         desc_;

        #ifndef __APPLE__
        GLPlatformContext               context_;
        #endif

        //! Specifies whether this context uses a shared GL render context (true) or has its own hardware context (false).
        bool                            hasSharedContext_   = false;

        std::shared_ptr<GLStateManager> stateMngr_;
        RenderState                     renderState_;

        GLRenderTarget*                 boundRenderTarget_  = nullptr;

        GLint                           contextHeight_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
