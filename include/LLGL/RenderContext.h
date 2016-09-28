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


/**
\brief Render context interface.
\remarks The render context is the main interface for drawing and compute operations.
*/
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
        \brief Sets a few low-level graphics API dependent states.
        \remarks This is mainly used to work around uniform render target behavior between different
        low-level graphics APIs such as OpenGL and Direct3D.
        */
        virtual void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) = 0;

        /**
        \brief Sets the new video mode for this render context.
        \remarks This may invalidate the currently set render target.
        \see SetRenderTarget
        */
        virtual void SetVideoMode(const VideoModeDescriptor& videoModeDesc);

        //! Sets the new vertical-sychronization (Vsync) configuration for this render context.
        virtual void SetVsync(const VsyncDescriptor& vsyncDesc) = 0;

        //! Returns the video mode for this render context.
        inline const VideoModeDescriptor& GetVideoMode() const
        {
            return videoModeDesc_;
        }

        /**
        \brief Sets a single viewport.
        \remarks Similar to SetViewportArray but only a single viewport is set.
        \see SetViewportArray
        */
        virtual void SetViewport(const Viewport& viewport) = 0;

        /**
        \brief Sets the array of viewports.
        \param[in] viewports Specifies the array of viewports.
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each viewport is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each viewport is on the lower-left.
        \see SetGraphicsAPIDependentState
        */
        virtual void SetViewportArray(const std::vector<Viewport>& viewports) = 0;

        /**
        \brief Sets a single scissor rectangle.
        \remarks Similar to SetScissorArray but only a single scissor rectangle is set.
        \see SetScissorArray
        */
        virtual void SetScissor(const Scissor& scissor) = 0;

        /**
        \brief Sets the specified scissor rectangles.
        \param[in] scissors Specifies the list of scissor rectangles.
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each scissor rectangle is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each scissor rectangle is on the lower-left.
        \see SetGraphicsAPIDependentState
        */
        virtual void SetScissorArray(const std::vector<Scissor>& scissors) = 0;

        //! Sets the new value to clear the color buffer. By default black (0, 0, 0, 0).
        virtual void SetClearColor(const ColorRGBAf& color) = 0;

        //! Sets the new value to clear the depth buffer with. By default 1.0.
        virtual void SetClearDepth(float depth) = 0;

        //! Sets the new value to clear the stencil buffer. By default 0.
        virtual void SetClearStencil(int stencil) = 0;

        /**
        \brief Clears the specified frame buffers.
        \param[in] flags Specifies the clear buffer flags.
        This can be a bitwise OR combination of the "ClearBuffersFlags" enumeration entries.
        \remarks To specify the clear values for each buffer use the respective "SetClear..." function
        \see ClearBuffersFlags
        \see SetClearColor
        \see SetClearDepth
        \see SetClearStencil
        */
        virtual void ClearBuffers(long flags) = 0;

        /* ----- Hardware Buffers ------ */

        /**
        \brief Sets the active vertex buffer for subsequent drawing operations.
        \param[in] vertexBuffer Specifies the vertex buffer to set. This must not be an unspecified vertex buffer,
        i.e. it must be initialized with the "RenderSystem::SetupVertexBuffer" function.
        \see RenderSystem::SetupVertexBuffer
        */
        virtual void SetVertexBuffer(VertexBuffer& vertexBuffer) = 0;
        
        /**
        \brief Sets the active index buffer for subsequent drawing operations.
        \param[in] indexBuffer Specifies the index buffer to set. This must not be an unspecified index buffer,
        i.e. it must be initialized with the "RenderSystem::SetupIndexBuffer" function.
        \remarks An active index buffer is only required for any "DrawIndexed" or "DrawIndexedInstanced" draw call.
        \see RenderSystem::SetupIndexBuffer
        */
        virtual void SetIndexBuffer(IndexBuffer& indexBuffer) = 0;
        
        /**
        \brief Sets the active constant buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] constantBuffer Specifies the constant buffer to set. This must not be an unspecified constant buffer,
        i.e. it must be initialized with the "RenderSystem::SetupConstantBuffer" function.
        \param[in] slot Specifies the slot index where to put the constant buffer.
        \param[in] shaderStageFlags Specifies at which shader stages the constant buffer is to be set. By default all shader stages are affected.
        \see RenderSystem::SetupConstantBuffer
        \see ShaderStageFlags
        */
        virtual void SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active storage buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] storageBuffer Specifies the storage buffer to set. This must not be an unspecified storage buffer,
        i.e. it must be initialized with the "RenderSystem::SetupStorageBuffer" function.
        \param[in] slot Specifies the slot index where to put the storage buffer.
        \see RenderSystem::SetupStorageBuffer
        */
        virtual void SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot) = 0;

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

        /**
        \brief Sets the active texture of the specified slot index for subsequent drawing and compute operations.
        \param[in] texture Specifies the texture to set.
        \param[in] slot Specifies the slot index where to put the texture.
        */
        virtual void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Generates the MIP ("Multum in Parvo") maps for the specified texture.
        \see https://developer.valvesoftware.com/wiki/MIP_Mapping
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /* ----- Samplers ----- */

        /**
        \brief Sets the active sampler of the specified slot index for subsequent drawing and compute operations.
        \param[in] sampler Specifies the sampler to set.
        \param[in] slot Specifies the slot index where to put the sampler.
        \see RenderSystem::CreateSampler
        */
        virtual void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Sets the active render target.
        \param[in] renderTarget Specifies the render target to set.
        \remarks Subsequent drawing operations will be rendered into the textures that are attached to the specified render target.
        \note If the specified render-target has not the same resolution as this render context, the viewports and scissor rectangles may be invalidated!
        \see UnsetRenderTarget
        */
        virtual void SetRenderTarget(RenderTarget& renderTarget) = 0;

        /**
        \brief Unsets the previously set render target.
        \remarks Subsequent drawing operations will be rendered into the main framebuffer, which can then be presented onto the screen.
        \see SetRenderTarget
        */
        virtual void UnsetRenderTarget() = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Sets the active graphics pipeline state.
        \param[in] graphicsPipeline Specifies the graphics pipeline state to set.
        \remarks This will set all blending-, rasterizer-, depth-, stencil-, and shader states.
        A valid graphics pipeline must always be set before any drawing operation can be performed.
        \see RenderSystem::CreateGraphicsPipeline
        */
        virtual void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) = 0;

        /**
        \brief Sets the active compute pipeline state.
        \param[in] computePipeline Specifies the compuite pipeline state to set.
        \remarks This will set the compute shader states.
        A valid compute pipeline must always be set before any compute operation can be performed.
        \see RenderSystem::CreateComputePipeline
        */
        virtual void SetComputePipeline(ComputePipeline& computePipeline) = 0;

        /* ----- Queries ----- */

        /**
        \brief Begins the specified query.
        \param[in] query Specifies the query to begin with.
        This must be same query object as in the subsequent "EndQuery" function call, to end the query operation.
        \remarks The "BeginQuery" and "EndQuery" functions can be wrapped around any drawing and/or compute operation.
        This can an occlusion query for instance, which determines how many fragments have passed the depth test.
        \see RenderSystem::CreateQuery
        \see EndQuery
        \see QueryResult
        */
        virtual void BeginQuery(Query& query) = 0;

        /**
        \brief Ends the specified query.
        \see RenderSystem::CreateQuery
        \see BeginQuery
        \see QueryResult
        */
        virtual void EndQuery(Query& query) = 0;

        /**
        \brief Queries the result of the specified Query object.
        \param[in,out] query Specifies the Query object whose result is to be queried.
        \param[out] result Specifies the output result.
        \return True if the result is available, otherwise false in which case 'result' is not modified.
        */
        virtual bool QueryResult(Query& query, std::uint64_t& result) = 0;

        /**
        \brief Begins conditional rendering with the specified query object.
        \param[in] query Specifies the query object which is to be used as render condition.
        This must be an occlusion query, i.e. it's type must be either
        QueryType::SamplesPassed, QueryType::AnySamplesPassed, or QueryType::AnySamplesPassedConservative.
        \param[in] mode Specifies the mode of the render conidition.
        \remarks Here is a usage example:
        \code
        context->BeginQuery(*occlusionQuery);
        // draw bounding box ...
        context->EndQuery(*occlusionQuery);
        context->BeginRenderConidtion(*occlusionQuery, LLGL::RenderConditionMode::Wait);
        // draw actual object ...
        context->EndRenderConidtion();
        \endcode
        \see QueryType
        \see RenderConditionMode
        */
        virtual void BeginRenderCondition(Query& query, const RenderConditionMode mode) = 0;

        /**
        \brief Ends the current render condition.
        \see BeginRenderCondition
        */
        virtual void EndRenderCondition() = 0;

        /* ----- Drawing ----- */

        /**
        \brief Draws the specified amount of primitives from the currently set vertex buffer.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] firstVertex Specifies the zero-based offset of the first vertex from the vertex buffer.
        */
        virtual void Draw(unsigned int numVertices, unsigned int firstVertex) = 0;

        //! \see DrawIndexed(unsigned int, unsigned int, int)
        virtual void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) = 0;

        /**
        \brief Draws the specified amount of primitives from the currently set vertex- and index buffers.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] firstIndex Specifies the zero-based offset of the first index from the index buffer.
        \param[in] vertexOffset Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
        */
        virtual void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) = 0;

        //! \see DrawInstanced(unsigned int, unsigned int, unsigned int, unsigned int)
        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) = 0;
        
        /**
        \brief Draws the specified amount of instances of primitives from the currently set vertex buffer.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] firstVertex Specifies the zero-based offset of the first vertex from the vertex buffer.
        \param[in] numInstances Specifies the number of instances to generate.
        \param[in] instanceOffset Specifies the zero-based instance offset which is added to each instance ID.
        */
        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) = 0;

        //! \see DrawIndexedInstanced(unsigned int, unsigned int, unsigned int, int, unsigned int)
        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) = 0;
        
        //! \see DrawIndexedInstanced(unsigned int, unsigned int, unsigned int, int, unsigned int)
        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset) = 0;
        
        /**
        \brief Draws the specified amount of instances of primitives from the currently set vertex- and index buffers.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] numInstances Specifies the number of instances to generate.
        \param[in] firstIndex Specifies the zero-based offset of the first index from the index buffer.
        \param[in] vertexOffset Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
        \param[in] instanceOffset Specifies the zero-based instance offset which is added to each instance ID.
        */
        virtual void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset) = 0;

        /* ----- Compute ----- */

        /**
        \brief Dispachtes a compute command.
        \param[in] threadGroupSize Specifies the number of thread groups,
        where the number of threads per group is specified statically within the compute shader.
        \see SetComputePipeline
        */
        virtual void DispatchCompute(const Gs::Vector3ui& threadGroupSize) = 0;

        /* ----- Misc ----- */

        //! Synchronizes the GPU, i.e. waits until the GPU has completed all pending commands.
        virtual void SyncGPU() = 0;

    protected:

        RenderContext() = default;

        void SetWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext);

        /**
        \brief Shares the window and video mode with another render context.
        \note This is only used by the renderer debug layer.
        */
        void ShareWindowAndVideoMode(RenderContext& other);

    private:

        std::shared_ptr<Window> window_;
        VideoModeDescriptor     videoModeDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
