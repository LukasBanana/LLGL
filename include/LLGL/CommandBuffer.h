/*
 * CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_H
#define LLGL_COMMAND_BUFFER_H


#include "Export.h"
#include "RenderContextFlags.h"
#include "RenderSystemFlags.h"
#include "ColorRGBA.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "Texture.h"
#include "TextureArray.h"
#include "Sampler.h"
#include "SamplerArray.h"

#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Query.h"


namespace LLGL
{


class RenderContext;

/**
\brief Command buffer interface.
\remarks This is the main interface to commit graphics and compute commands to the GPU.
For older graphics APIs (such as OpenGL and Direct3D 11) it makes not much sense to create multiple command buffers,
but for recent graphics APIs (such as Vulkan and Direct3D 12) it might be sensible to have more than one command buffer,
to maximize CPU utilization with several worker threads and one command buffer for each thread.
Note that especially for recent graphics APIs most states in the command buffer are NOT persistent,
i.e. they need to be reset every frame (e.g. bound vertex buffers).
Assume that all states that can be changed with a setter function are not persistent except the opposite is mentioned.
*/
class LLGL_EXPORT CommandBuffer
{

    public:

        /* ----- Common ----- */

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator = (const CommandBuffer&) = delete;

        virtual ~CommandBuffer()
        {
        }

        /* ----- Configuration ----- */

        /**
        \brief Sets a few low-level graphics API dependent states.
        \remarks This can be used to work around several differences between the low-level graphics APIs
        (e.g. for a uniform render target behavior between OpenGL and Direct3D).
        */
        virtual void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) = 0;

        /**
        \brief Sets a single viewport.
        \remarks Similar to SetViewportArray but only a single viewport is set.
        \note This state is guaranteed to be persistent.
        \see SetViewportArray
        */
        virtual void SetViewport(const Viewport& viewport) = 0;

        /**
        \brief Sets an array of viewports.
        \param[in] numViewports Specifies the number of viewports to set.
        \param[in] viewportArray Pointer to the array of viewports. This must not be null!
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each viewport is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each viewport is on the lower-left.
        \note This state is guaranteed to be persistent.
        \see SetGraphicsAPIDependentState
        */
        virtual void SetViewportArray(unsigned int numViewports, const Viewport* viewportArray) = 0;

        /**
        \brief Sets a single scissor rectangle.
        \remarks Similar to SetScissorArray but only a single scissor rectangle is set.
        \note This state is guaranteed to be persistent.
        \see SetScissorArray
        */
        virtual void SetScissor(const Scissor& scissor) = 0;

        /**
        \brief Sets an array of scissor rectangles.
        \param[in] numScissors Specifies the number of scissor rectangles to set.
        \param[in] scissorArray Pointer to the array of scissor rectangles. This must not be null!
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each scissor rectangle is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each scissor rectangle is on the lower-left.
        \note This state is guaranteed to be persistent.
        \see SetGraphicsAPIDependentState
        */
        virtual void SetScissorArray(unsigned int numScissors, const Scissor* scissorArray) = 0;

        /**
        \brief Sets the new value to clear the color buffer. By default black (0, 0, 0, 0).
        \note This state is guaranteed to be persistent.
        \see Clear
        */
        virtual void SetClearColor(const ColorRGBAf& color) = 0;

        /**
        \brief Sets the new value to clear the depth buffer with. By default 1.0.
        \note This state is guaranteed to be persistent.
        \see Clear
        */
        virtual void SetClearDepth(float depth) = 0;

        /**
        \brief Sets the new value to clear the stencil buffer. By default 0.
        \note This state is guaranteed to be persistent.
        \see Clear
        */
        virtual void SetClearStencil(int stencil) = 0;

        /**
        \brief Clears the specified framebuffers of the active render target.
        \param[in] flags Specifies the clear buffer flags.
        This can be a bitwise OR combination of the "ClearFlags" enumeration entries.
        \remarks To specify the clear values for each buffer type, use the respective "SetClear..." function.
        To clear only a specific render-target color buffer, use the "ClearTarget" function.
        \see ClearFlags
        \see SetClearColor
        \see SetClearDepth
        \see SetClearStencil
        \see ClearTarget
        */
        virtual void Clear(long flags) = 0;

        /**
        \brief Clears the specified color buffer of the active render target.
        \param[in] targetIndex Specifies the zero-based index of the color target which is to be cleared.
        If this value is greater than or equal to the number of color attachments
        in the active render target, the behavior of this function is undefined.
        \param[in] color Specifies the color to clear the color buffer with.
        \remarks To clear all color buffers with the same value, use the "Clear" function.
        \see Clear
        */
        virtual void ClearTarget(unsigned int targetIndex, const LLGL::ColorRGBAf& color) = 0;

        /* ----- Buffers ------ */

        /**
        \brief Sets the specified vertex buffer for subsequent drawing operations.
        \param[in] buffer Specifies the vertex buffer to set. This buffer must have been created with the buffer type: BufferType::Vertex.
        This must not be an unspecified vertex buffer, i.e. it must be initialized with either the initial data in the "RenderSystem::CreateBuffer"
        function or with the "RenderSystem::WriteBuffer" function.
        \see RenderSystem::CreateBuffer
        \see RenderSystem::WriteBuffer
        \see SetVertexBufferArray
        */
        virtual void SetVertexBuffer(Buffer& buffer) = 0;

        /**
        \brief Sets the specified array of vertex buffers for subsequent drawing operations.
        \param[in] bufferArray Specifies the vertex buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetVertexBuffer
        */
        virtual void SetVertexBufferArray(BufferArray& bufferArray) = 0;

        /**
        \brief Sets the active index buffer for subsequent drawing operations.
        \param[in] buffer Specifies the index buffer to set. This buffer must have been created with the buffer type: BufferType::Index.
        This must not be an unspecified index buffer, i.e. it must be initialized with either the initial data in the "RenderSystem::CreateBuffer"
        function or with the "RenderSystem::WriteBuffer" function.
        \remarks An active index buffer is only required for any "DrawIndexed" or "DrawIndexedInstanced" draw call.
        \see RenderSystem::WriteIndexBuffer
        */
        virtual void SetIndexBuffer(Buffer& buffer) = 0;
        
        /**
        \brief Sets the active constant buffer at the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the constant buffer to set. This buffer must have been created with the buffer type: BufferType::Constant.
        This must not be an unspecified constant buffer, i.e. it must be initialized with either the initial data in the "RenderSystem::CreateBuffer"
        function or with the "RenderSystem::WriteBuffer" function.
        \param[in] slot Specifies the slot index where to put the constant buffer.
        \param[in] shaderStageFlags Specifies at which shader stages the constant buffer is to be set. By default all shader stages are affected.
        \see RenderSystem::WriteBuffer
        \see ShaderStageFlags
        */
        virtual void SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of constant buffers at the specified start slot index.
        \param[in] bufferArray Specifies the constant buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetConstantBuffer
        */
        virtual void SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active storage buffer of the specified slot index for subsequent drawing and compute operations.
        \param[in] buffer Specifies the storage buffer to set. This buffer must have been created with the buffer type: BufferType::Storage.
        \param[in] slot Specifies the slot index where to put the storage buffer.
        \param[in] shaderStageFlags Specifies at which shader stages the storage buffer is to be set and which resource views are to be set.
        By default all shader stages and all resource views are affected.
        \see RenderSystem::MapBuffer
        \see RenderSystem::UnmapBuffer
        */
        virtual void SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of storage buffers at the specified start slot index.
        \param[in] bufferArray Specifies the storage buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetStorageBuffer
        */
        virtual void SetStorageBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active stream-output buffer to the stream-output stage.
        \param[in] buffer Specifies the stream-output buffer to set. This buffer must have been created with the buffer type: BufferType::StreamOutput.
        \see RenderSystem::MapBuffer
        \see RenderSystem::UnmapBuffer
        */
        virtual void SetStreamOutputBuffer(Buffer& buffer) = 0;

        /**
        \brief Sets the active array of stream-output buffers.
        \param[in] bufferArray Specifies the stream-output buffer array to set.
        \see RenderSystem::CreateBufferArray
        \see SetStreamOutputBuffer
        */
        virtual void SetStreamOutputBufferArray(BufferArray& bufferArray) = 0;

        /**
        \brief Begins with stream-output for subsequent draw calls.
        \param[in] primitiveType Specifies the primitive output type of the last vertex processing shader stage (e.g. vertex- or geometry shader).
        \see EndStreamOutput
        */
        virtual void BeginStreamOutput(const PrimitiveType primitiveType) = 0;

        /**
        \brief Ends the current stream-output.
        \see BeginStreamOutput
        */
        virtual void EndStreamOutput() = 0;

        /* ----- Textures ----- */

        /**
        \brief Sets the active texture of the specified slot index for subsequent drawing and compute operations.
        \param[in] texture Specifies the texture to set.
        \param[in] slot Specifies the slot index where to put the texture.
        */
        virtual void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of textures at the specified start slot index.
        \see SetTexture
        */
        virtual void SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Samplers ----- */

        /**
        \brief Sets the active sampler of the specified slot index for subsequent drawing and compute operations.
        \param[in] sampler Specifies the sampler to set.
        \param[in] slot Specifies the slot index where to put the sampler.
        \see RenderSystem::CreateSampler
        */
        virtual void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /**
        \brief Sets the active array of samplers at the specified start slot index.
        \see SetSampler
        */
        virtual void SetSamplerArray(SamplerArray& samplerArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Sets the specified render target as the new target for subsequent rendering commands.
        \param[in] renderTarget Specifies the render target to set.
        \remarks Subsequent drawing operations will be rendered into the textures that are attached to the specified render target.
        \note If the specified render-target has not the same resolution as this render context, the viewports and scissor rectangles may be invalidated!
        \see SetRenderTarget(RenderContext&)
        */
        virtual void SetRenderTarget(RenderTarget& renderTarget) = 0;

        /**
        \brief Sets the back buffer (or rather swap-chain) of the specified render context as the new target for subsequent rendering commands.
        \remarks Subsequent drawing operations will be rendered into the main framebuffer, which can then be presented onto the screen.
        \see SetRenderTarget(RenderTarget&)
        */
        virtual void SetRenderTarget(RenderContext& renderContext) = 0;

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
        \param[in] query Specifies the Query object whose result is to be queried.
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
        \param[in] groupSizeX Specifies the number of thread groups in the X-dimension.
        \param[in] groupSizeY Specifies the number of thread groups in the Y-dimension.
        \param[in] groupSizeZ Specifies the number of thread groups in the Z-dimension.
        \see SetComputePipeline
        \see RenderingCaps::maxNumComputeShaderWorkGroups
        */
        virtual void Dispatch(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ) = 0;

        /* ----- Misc ----- */

        //! Synchronizes the GPU, i.e. waits until the GPU has completed all pending commands from this command buffer.
        virtual void SyncGPU() = 0;

    protected:

        CommandBuffer() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
