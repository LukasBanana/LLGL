/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_H__
#define __LLGL_RENDER_CONTEXT_H__


#include "Export.h"
#include "Window.h"
#include "RenderContextDescriptor.h"
#include "RenderContextFlags.h"
#include "RenderSystemFlags.h"
#include "ColorRGBA.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "StorageBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Sampler.h"
#include "Query.h"

#include <Gauss/Vector3.h>
#include <string>
#include <map>


namespace LLGL
{


//! Render context interface.
class LLGL_EXPORT RenderContext
{

    public:

        /* ----- Common ----- */

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator = (const RenderContext&) = delete;

        virtual ~RenderContext();

        //! Presents the current frame on the screen.
        virtual void Present() = 0;

        //! Returns the window which is used to draw all content.
        inline Window& GetWindow() const
        {
            return *window_;
        }

        /* ----- Configuration ----- */

        /**
        \briefs Sets a few low-level graphics API dependent states.
        \remarks This is mainly used to work around uniform render target behavior between different
        low-level graphics APIs such as OpenGL and Direct3D.
        */
        virtual void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) = 0;

        //! Sets the new video mode for this render context.
        virtual void SetVideoMode(const VideoModeDescriptor& videoModeDesc);

        //! Sets the new vertical-sychronization (Vsync) configuration for this render context.
        virtual void SetVsync(const VsyncDescriptor& vsyncDesc) = 0;

        //! Returns the video mode for this render context.
        inline const VideoModeDescriptor& GetVideoMode() const
        {
            return videoModeDesc_;
        }

        virtual void SetViewports(const std::vector<Viewport>& viewports) = 0;
        virtual void SetScissors(const std::vector<Scissor>& scissors) = 0;

        virtual void SetClearColor(const ColorRGBAf& color) = 0;
        virtual void SetClearDepth(float depth) = 0;
        virtual void SetClearStencil(int stencil) = 0;

        virtual void ClearBuffers(long flags) = 0;

        virtual void SetDrawMode(const DrawMode drawMode) = 0;

        /* ----- Hardware Buffers ------ */

        virtual void BindVertexBuffer(VertexBuffer& vertexBuffer) = 0;
        virtual void UnbindVertexBuffer() = 0;

        virtual void BindIndexBuffer(IndexBuffer& indexBuffer) = 0;
        virtual void UnbindIndexBuffer() = 0;

        virtual void BindConstantBuffer(unsigned int index, ConstantBuffer& constantBuffer) = 0;
        virtual void UnbindConstantBuffer(unsigned int index) = 0;

        virtual void BindStorageBuffer(unsigned int index, StorageBuffer& storageBuffer) = 0;
        virtual void UnbindStorageBuffer(unsigned int index) = 0;

        /**
        \brief Maps the specified storage buffer from GPU to CPU memory space.
        \param[in] storageBuffer Specifies the storage buffer which is to be mapped.
        \param[in] access Specifies the CPU buffer access requirement, i.e. if the CPU can read and/or write the mapped memory.
        \return Raw pointer to the mapped memory block. You should be aware of the storage buffer size, to not cause memory violations.
        \throws std::runtime_error If a storage buffer is already being mapped.
        \see UnmapStorageBuffer
        */
        virtual void* MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access) = 0;

        /**
        \brief Unmaps the previously mapped storage buffer.
        \see MapStorageBuffer
        */
        virtual void UnmapStorageBuffer() = 0;

        /* ----- Textures ----- */

        virtual void BindTexture(unsigned int layer, Texture& texture) = 0;
        virtual void UnbindTexture(unsigned int layer) = 0;

        /**
        \brief Generates the MIP ("Multum in Parvo") maps for the specified texture.
        \see https://developer.valvesoftware.com/wiki/MIP_Mapping
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /* ----- Samplers ----- */

        virtual void BindSampler(unsigned int layer, Sampler& sampler) = 0;
        virtual void UnbindSampler(unsigned int layer) = 0;

        /* ----- Render Targets ----- */

        virtual void BindRenderTarget(RenderTarget& renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;

        /* ----- Pipeline States ----- */

        virtual void BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline) = 0;
        virtual void BindComputePipeline(ComputePipeline& computePipeline) = 0;

        /* ----- Queries ----- */

        virtual void BeginQuery(Query& query) = 0;
        virtual void EndQuery(Query& query) = 0;

        /**
        \brief Queries the result of the specified Query object.
        \param[in,out] query Specifies the Query object whose result is to be queried.
        \param[out] result Specifies the output result.
        \return True if the result is available, otherwise false in which case 'result' is not modified.
        */
        virtual bool QueryResult(Query& query, std::uint64_t& result) = 0;

        /* ----- Drawing ----- */

        virtual void Draw(unsigned int numVertices, unsigned int firstVertex) = 0;

        virtual void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) = 0;
        virtual void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) = 0;

        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) = 0;
        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) = 0;

        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) = 0;
        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int indexOffset) = 0;
        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int indexOffset, unsigned int instanceOffset) = 0;

        /* ----- Compute ----- */

        virtual void DispatchCompute(const Gs::Vector3ui& threadGroupSize) = 0;

        /* ----- Misc ----- */

        //! Synchronizes the GPU, i.e. waits until the GPU has completed all pending commands.
        virtual void SyncGPU() = 0;

    protected:

        RenderContext() = default;

        void SetWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext);

    private:

        std::shared_ptr<Window> window_;
        VideoModeDescriptor     videoModeDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
