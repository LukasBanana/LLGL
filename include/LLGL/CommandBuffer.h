/*
 * CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_BUFFER_H
#define LLGL_COMMAND_BUFFER_H


#include "RenderSystemChild.h"
#include "CommandBufferFlags.h"
#include "RenderSystemFlags.h"
#include "ColorRGBA.h"

#include "Buffer.h"
#include "BufferArray.h"
#include "ResourceHeap.h"
#include "PipelineLayoutFlags.h"

#include "RenderPass.h"
#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Query.h"

#include <cstdint>


namespace LLGL
{


class RenderContext;

/**
\brief Command buffer interface.
\remarks This is the main interface to record graphics and compute commands to be submitted to the GPU.
For older graphics APIs (such as OpenGL and Direct3D 11) it makes not much sense to create multiple command buffers,
but for recent graphics APIs (such as Vulkan, Direct3D 12, and Metal) it might be sensible to have more than one command buffer,
to maximize CPU utilization with several worker threads and one command buffer for each thread.
However, multi-threading for command buffers is currently not supported by LLGL!
Assume that all states that can be changed with a setter function are not persistent except the opposite is mentioned.
Before any command can be recorded, the command buffer must be set into record mode, which is done by the CommandQueue::Begin function.
There are only a few exceptions of functions that can be used outside of recording,
which are CommandBuffer::SetClearColor, CommandBuffer::SetClearDepth, and CommandBuffer::SetClearStencil.
\see CommandQueue::Begin(CommandBuffer&, long)
*/
class LLGL_EXPORT CommandBuffer : public RenderSystemChild
{

    public:

        /* ----- Configuration ----- */

        /**
        \brief Sets a few low-level graphics API dependent states.
        \param[in] stateDesc Specifies a pointer to the renderer spcific state descriptor. If this is a null pointer, the function has no effect.
        \param[in] stateDescSize Specifies the size (in bytes) of the renderer spcific state descriptor structure.
        If this value is not equal to the state descriptor structure that is required for the respective renderer, the function has no effect.
        \remarks This can be used to work around several differences between the low-level graphics APIs,
        e.g. for a uniform render target behavior between OpenGL and Direct3D.
        Here is a usage example:
        \code
        LLGL::OpenGLDependentStateDescriptor myOpenGLStateDesc;
        myOpenGLStateDesc.invertFrontFace = true;
        myCommandBuffer->SetGraphicsAPIDependentState(&myOpenGLStateDesc, sizeof(myOpenGLStateDesc));
        \endcode
        \note Invalid arguments are ignored by this function silently (except for corrupted pointers).
        \see OpenGLDependentStateDescriptor
        */
        virtual void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) = 0;

        /* ----- Viewport and Scissor ----- */

        /**
        \brief Sets a single viewport.
        \remarks Similar to SetViewports but only a single viewport is set.
        \see SetViewports
        */
        virtual void SetViewport(const Viewport& viewport) = 0;

        /**
        \brief Sets an array of viewports.
        \param[in] numViewports Specifies the number of viewports to set. Most render system have a limit of 16 viewports.
        \param[in] viewports Pointer to the array of viewports. This must not be null!
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each viewport is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each viewport is on the lower-left.
        \note This state is guaranteed to be persistent.
        \see SetGraphicsAPIDependentState
        \see RenderingLimits::maxNumViewports
        */
        virtual void SetViewports(std::uint32_t numViewports, const Viewport* viewports) = 0;

        /**
        \brief Sets a single scissor rectangle.
        \remarks Similar to SetScissors but only a single scissor rectangle is set.
        \see SetScissors
        */
        virtual void SetScissor(const Scissor& scissor) = 0;

        /**
        \brief Sets an array of scissor rectangles, but only if the scissor test was enabled in the previously set graphics pipeline (otherwise, this function has no effect).
        \param[in] numScissors Specifies the number of scissor rectangles to set.
        \param[in] scissors Pointer to the array of scissor rectangles. This must not be null!
        \remarks This function behaves differently on the OpenGL render system, depending on the state configured
        with the "SetGraphicsAPIDependentState" function. If 'stateOpenGL.screenSpaceOriginLowerLeft' is false,
        the origin of each scissor rectangle is on the upper-left (like for all other render systems).
        If 'stateOpenGL.screenSpaceOriginLowerLeft' is true, the origin of each scissor rectangle is on the lower-left.
        \see SetGraphicsAPIDependentState
        \see RasterizerDescriptor::scissorTestEnabled
        */
        virtual void SetScissors(std::uint32_t numScissors, const Scissor* scissors) = 0;

        /* ----- Clear ----- */

        /**
        \brief Sets the new value to clear the color buffer. By default black (0, 0, 0, 0).
        \note This state is guaranteed to be persistent and can be used outside of command buffer recording.
        \see Clear
        */
        virtual void SetClearColor(const ColorRGBAf& color) = 0;

        /**
        \brief Sets the new value to clear the depth buffer with. By default 1.0.
        \note This state is guaranteed to be persistent and can be used outside of command buffer recording.
        \see Clear
        */
        virtual void SetClearDepth(float depth) = 0;

        /**
        \brief Sets the new value to clear the stencil buffer. By default 0.
        \param[in] stencil Specifies the value to clear the stencil buffer.
        This value is masked with <code>2^m-1</code>, where \c m is the number of bits in the stencil buffer (e.g. <code>stencil & 0xFF</code> for an 8-bit stencil buffer).
        \note This state is guaranteed to be persistent and can be used outside of command buffer recording.
        \see Clear
        */
        virtual void SetClearStencil(std::uint32_t stencil) = 0;

        /**
        \brief Clears the specified group of attachments of the active render target.
        \param[in] flags Specifies the clear buffer flags.
        This can be a bitwise OR combination of the "ClearFlags" enumeration entries.
        If this contains the ClearFlags::Color bit, all color attachments of the active render target are cleared with the color previously set by SetClearColor.
        \remarks To specify the clear values for each buffer type, use the respective "SetClear..." function.
        To clear only a specific render-target color buffer, use the "ClearAttachments" function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        \see ClearFlags
        \see SetClearColor
        \see SetClearDepth
        \see SetClearStencil
        \see ClearAttachments
        */
        virtual void Clear(long flags) = 0;

        /**
        \brief Clears the specified attachments of the active render target.
        \param[in] numAttachments Specifies the number of attachments to clear.
        \param[in] attachments Pointer to the array of attachment clear commands. This must not be null!
        \remarks To clear all color buffers with the same value, use the "Clear" function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        \see Clear
        */
        virtual void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) = 0;

        /* ----- Input Assembly ------ */

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

        /* ----- Stream Output Buffers ------ */

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

        /* ----- Resource Heaps ----- */

        /**
        \brief Binds the specified resource heap to the graphics pipeline.
        \param[in] resourceHeap Specifies the resource heap that contains all shader resources that will be bound to the shader pipeline.
        \param[in] firstSet Specifies the set number of the first layout descriptor.
        \remarks This may invalidate the previously bound resource heap for both the graphics and compute pipeline.
        \note Parameter 'firstSet' is only supported with: Vulkan.
        */
        virtual void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) = 0;

        /**
        \brief Binds the specified resource heap to the compute pipeline.
        \param[in] resourceHeap Specifies the resource heap that contains all shader resources that will be bound to the shader pipeline.
        \param[in] firstSet Specifies the set number of the first layout descriptor.
        \remarks This may invalidate the previously bound resource heap for both the graphics and compute pipeline.
        \note Parameter 'firstSet' is only supported with: Vulkan.
        */
        virtual void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) = 0;

        /* ----- Render Passes ----- */

        /**
        \brief Begins with a new render pass.
        \param[in] renderTarget Specifies the render target in which the subsequent draw operations will be stored.
        \param[in] renderPass Specifies an optional render pass object. If this is null, the default render pass for the specified render target will be used.
        This render pass object must be compatible with the render pass object the specified render target was created with.
        \param[in] numClearValues Specifies the number of clear values that are specified in the <code>clearValues</code> parameter.
        This should be greater than or equal to the number of render pass attachments whose load operation (i.e. AttachmentFormatDescriptor::loadOp) is set to AttachmentLoadOp::Clear.
        Otherwise, the default values from \c SetClearColor, \c SetClearDepth, and \c SetClearStencil are used.
        \param[in] clearValues Optional pointer to the array of clear values.
        If <code>numClearValues</code> is not zero, this must be a valid pointer to an array of at least <code>numClearValues</code> entries.
        Each entry in the array is used to clear the attachment whose load operation is set to AttachmentLoadOp::Clear,
        where the depth attachment (i.e. RenderPassDescriptor::depthAttachment) appears as the penultimate
        and the stencil attachment (i.e. RenderPassDescriptor::stencilAttachment) appears as the last entry.
        \remarks This function starts a new render pass section and must be ended with the EndRenderPass function.
        A simple frame setup could look like this:
        \code
        myCmdQueue->Begin(*myCmdBuffer);
        {
            myCmdBuffer->BeginRenderPass(*myRenderTarget);
            {
                myCmdBuffer->SetGraphicsPipeline(*myGfxPipeline);
                myCmdBuffer->SetGraphicsResourceHeap(*myResourceHeap);
                myCmdBuffer->Draw(...);
            }
            myCmdBuffer->EndRenderPass();
        }
        myCmdQueue->End(*myCmdBuffer);
        myRenderContext->Present();
        \endcode
        \see RenderSystem::CreateRenderPass
        \see RenderSystem::CreateRenderTarget
        \see RenderTargetDescriptor::renderPass
        \see AttachmentFormatDescriptor::loadOp
        \see EndRenderPass
        */
        virtual void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr
        ) {}//= 0;

        /**
        \brief Ends the current render pass.
        \see BeginRenderPass
        */
        virtual void EndRenderPass() {}//= 0;

        #if 1//TODO: remove

        /* ----- Render Targets ----- */

        /**
        \brief Sets the specified render target as the new target for subsequent rendering commands.
        \param[in] renderTarget Specifies the render target to set.
        \remarks Subsequent drawing operations will be rendered into the textures that are attached to the specified render target.
        \note This function may invalidate the viewports and scissor rectangles.
        It is hence advisable to always set the viewports (and scissor rectangles, if enabled) after a new render target is bound.
        \see SetRenderTarget(RenderContext&)
        \deprecated Will be removed soon, use BeginRenderPass.
        */
        virtual void SetRenderTarget(RenderTarget& renderTarget) {};

        /**
        \brief Sets the back buffer (or rather swap-chain) of the specified render context as the new target for subsequent rendering commands.
        \remarks Subsequent drawing operations will be rendered into the main framebuffer, which can then be presented onto the screen.
        \see SetRenderTarget(RenderTarget&)
        \deprecated Will be removed soon, use BeginRenderPass.
        */
        virtual void SetRenderTarget(RenderContext& renderContext) {};

        #endif

        /* ----- Pipeline States ----- */

        /**
        \brief Sets the active graphics pipeline state.
        \param[in] graphicsPipeline Specifies the graphics pipeline state to set.
        \remarks This will set all blending-, rasterizer-, depth-, stencil-, and shader states.
        A valid graphics pipeline must always be set before any drawing operation can be performed,
        and a valid render target (or render context) must always be set before any graphics pipeline can be set:
        \code
        // First set render target
        myCmdBuffer->SetRenderTarget(...);

        // Then set graphics pipeline
        myCmdBuffer->SetGraphicsPipeline(...);

        // Then perform drawing operations
        myCmdBuffer->SetGraphicsResourceHeap(...);
        myCmdBuffer->Draw(...);
        \endcode
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
        \brief Queries the result of the specified query object.
        \param[in] query Specifies the query object whose result is to be queried. This query object must not have been created with the QueryType::PipelineStatistics type.
        \param[out] result Specifies the output result in form of a 64-bit unsigned integer.
        \return True if the result is available, otherwise false in which case 'result' is not modified.
        \remarks For a query of type QueryType::PipelineStatistics, the function CommandBuffer::QueryPipelineStatisticsResult must be used.
        \see QueryPipelineStatisticsResult
        */
        virtual bool QueryResult(Query& query, std::uint64_t& result) = 0;

        /**
        \brief Queries the result of the specified query object for pipeline statistics.
        \param[in] query Specifies the query object whose result is to be queried. This query object must have been created with the QueryType::PipelineStatistics type.
        \param[out] result Specifies the output result in form of the QueryPipelineStatistics structure.
        \remarks For a query of type other than QueryType::PipelineStatistics, the function CommandBuffer::QueryResult must be used.
        \see QueryResult
        */
        virtual bool QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result) = 0;

        /**
        \brief Begins conditional rendering with the specified query object.
        \param[in] query Specifies the query object which is to be used as render condition.
        This must be an occlusion query, i.e. it's type must be either
        QueryType::SamplesPassed, QueryType::AnySamplesPassed, or QueryType::AnySamplesPassedConservative.
        \param[in] mode Specifies the mode of the render condition.
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
        \note The parameter <code>firstVertex</code> modifies the vertex ID within the shader pipeline differently for <code>SV_VertexID</code>
        in HLSL and <code>gl_VertexID</code> in GLSL (or <code>gl_VertexIndex</code> for Vulkan), due to rendering API differences.
        The system value <code>SV_VertexID</code> in HLSL will always start with zero,
        but the system value <code>gl_VertexID</code> in GLSL (or <code>gl_VertexIndex</code> for Vulkan)
        will start with the value of <code>firstVertex</code>.
        */
        virtual void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) = 0;

        //! \see DrawIndexed(std::uint32_t, std::uint32_t, std::int32_t)
        virtual void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex) = 0;

        /**
        \brief Draws the specified amount of primitives from the currently set vertex- and index buffers.
        \param[in] numIndices Specifies the number of indices to generate.
        \param[in] firstIndex Specifies the zero-based offset of the first index from the index buffer.
        \param[in] vertexOffset Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
        */
        virtual void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) = 0;

        //! \see DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
        virtual void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t firstInstance) = 0;

        /**
        \brief Draws the specified amount of instances of primitives from the currently set vertex buffer.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] firstVertex Specifies the zero-based offset of the first vertex from the vertex buffer.
        \param[in] numInstances Specifies the number of instances to generate.
        \param[in] firstInstance Specifies the zero-based offset of the first instance.
        \note The parameter <code>firstVertex</code> modifies the vertex ID within the shader pipeline differently for <code>SV_VertexID</code>
        in HLSL and <code>gl_VertexID</code> in GLSL (or <code>gl_VertexIndex</code> for Vulkan), due to rendering API differences.
        The system value <code>SV_VertexID</code> in HLSL will always start with zero,
        but the system value <code>gl_VertexID</code> in GLSL (or <code>gl_VertexIndex</code> for Vulkan)
        will start with the value of <code>firstVertex</code>.
        The same holds true for the parameter <code>firstInstance</code> and the system values <code>SV_InstanceID</code> in HLSL and <code>gl_InstanceID</code> in GLSL (or <code>gl_InstanceIndex</code> for Vulkan).
        */
        virtual void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) = 0;

        //! \see DrawIndexedInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t)
        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) = 0;

        //! \see DrawIndexedInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t)
        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) = 0;

        /**
        \brief Draws the specified amount of instances of primitives from the currently set vertex- and index buffers.
        \param[in] numIndices Specifies the number of indices to generate.
        \param[in] numInstances Specifies the number of instances to generate.
        \param[in] firstIndex Specifies the zero-based offset of the first index from the index buffer.
        \param[in] vertexOffset Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
        \param[in] firstInstance Specifies the zero-based offset of the first instance.
        \note The parameter <code>firstInstance</code> modifies the instance ID within the shader pipeline differently for <code>SV_InstanceID</code>
        in HLSL and <code>gl_InstanceID</code> in GLSL (or <code>gl_InstanceIndex</code> for Vulkan), due to rendering API differences.
        The system value <code>SV_InstanceID</code> in HLSL will always start with zero,
        but the system value <code>gl_InstanceID</code> in GLSL (or <code>gl_InstanceIndex</code> for Vulkan)
        will start with the value of <code>firstInstance</code>.
        */
        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) = 0;

        /* ----- Compute ----- */

        /**
        \brief Dispachtes a compute command.
        \param[in] groupSizeX Specifies the number of thread groups in the X-dimension.
        \param[in] groupSizeY Specifies the number of thread groups in the Y-dimension.
        \param[in] groupSizeZ Specifies the number of thread groups in the Z-dimension.
        \see SetComputePipeline
        \see RenderingLimits::maxNumComputeShaderWorkGroups
        */
        virtual void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) = 0;

    protected:

        CommandBuffer() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
