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
#include "QueryHeap.h"

#include <cstdint>


namespace LLGL
{


class RenderContext;

/**
\brief Command buffer interface.
\remarks This is the main interface to encode graphics and compute commands to be submitted to the GPU.
You can assume that all states that can be changed with a setter function are not persistent across several encoding sections, unless the opposite is mentioned.
Before any command can be encoded, the command buffer must be set into encode mode, which is done by the CommandBuffer::Begin function.
There are only a few exceptions of functions that can be used outside of encoding,
which are CommandBuffer::SetClearColor, CommandBuffer::SetClearDepth, and CommandBuffer::SetClearStencil.
*/
class LLGL_EXPORT CommandBuffer : public RenderSystemChild
{

    public:

        /* ----- Encoding ----- */

        /**
        \brief Begins with the encoding (also referred to as "recording") of this command buffer.
        \remarks All functions of the CommandBuffer interface must be used between a call to \c Begin and \c End, except for the following:
        - CommandBuffer::SetClearColor
        - CommandBuffer::SetClearDepth
        - CommandBuffer::SetClearStencil
        \see End
        \see RecordingFlags
        */
        virtual void Begin() = 0;

        /**
        \brief Ends the encoding (also referred to as "recording") of this command buffer.
        \see Begin
        \see CommandQueue::Submit(CommandBuffer&)
        */
        virtual void End() = 0;

        /* ----- Encoding ----- */

        /**
        \brief Updates the data of the specified buffer during encoding the command buffer.
        \param[in] dstBuffer Specifies the destination buffer whose data is to be updated.
        \param[in] dstOffset Specifies the destination offset (in bytes) at which the buffer is to be updated.
        This offset plus the data block size (i.e. <code>dstOffset + dataSize</code>) must be less than or equal to the size of the buffer.
        \param[in] data Raw pointer to the data with which the buffer is to be updated. This must not be null!
        \param[in] dataSize Specifies the size (in bytes) of the data block which is to be updated.
        This is limited to 2^16 = 65536 bytes, because it may be written to the command buffer itself before it is copied to the destination buffer (depending on the backend).
        \remarks To update buffers larger than 65536 bytes, use RenderSystem::WriteBuffer or RenderSystem::MapBuffer.
        It is recommended to call this outside of a render pass.
        Otherwise, LLGL needs to pause and resume the render pass for the Vulkan backend via a secondary render pass object.
        */
        virtual void UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize) = 0;

        /**
        \brief Encodes a buffer copy command for the specified buffer region.
        \param[in] dstBuffer Specifies the destination buffer whose data is to be updated.
        \param[in] dstOffset Specifies the destination offset (in bytes) at which the destination buffer is to be updated.
        This offset plus the size (i.e. <code>dstOffset + size</code>) must be less than or equal to the size of the destination buffer.
        \param[in] srcBuffer Specifies the source buffer whose data is to be read from.
        \param[in] srcOffset Specifies teh source offset (in bytes) at which the source buffer is to be read from.
        This offset plus the size (i.e. <code>srcOffset + size</code>) must be less than or equal to the size of the source buffer.
        \param[in] size Specifies the size of the buffer region to copy.
        \remarks It is recommended to call this outside of a render pass.
        Otherwise, LLGL needs to pause and resume the render pass for the Vulkan backend via a secondary render pass object.
        */
        virtual void CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size) = 0;

        #if 0
        virtual void FillBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, std::uint64_t size, std::uint32_t data) = 0;
        virtual void FillTexture(Texture& dstTexture, [...]) = 0;
        virtual void CopyTexture(Texture& dstTexture, Texture& srcTexture, [...]) = 0;
        virtual void BlitTexture(Texture& dstTexture, Texture& srcTexture, [...], Filter filter) = 0;
        #endif

        #if 0 // TODO: enable this
        /**
        \brief Generates all MIP-maps for the specified texture.
        \param[in,out] texture Specifies the texture whose MIP-maps are to be generated.
        \remarks To update the MIP levels outside of encoding a command buffer, use RenderSystem::GenerateMips.
        It is recommended to call this outside of a render pass.
        Otherwise, LLGL needs to pause and resume the render pass for the Vulkan backend via a secondary render pass object.
        \see GenerateMips(Texture&, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
        */
        virtual void GenerateMips(Texture& texture) = 0;
        #endif // /TODO

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
        \see RenderingLimits::maxViewports
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
        \note This state is guaranteed to be persistent and can be used outside of command buffer encoding.
        \see Clear
        */
        virtual void SetClearColor(const ColorRGBAf& color) = 0;

        /**
        \brief Sets the new value to clear the depth buffer with. By default 1.0.
        \note This state is guaranteed to be persistent and can be used outside of command buffer encoding.
        \see Clear
        */
        virtual void SetClearDepth(float depth) = 0;

        /**
        \brief Sets the new value to clear the stencil buffer. By default 0.
        \param[in] stencil Specifies the value to clear the stencil buffer.
        This value is masked with <code>2^m-1</code>, where \c m is the number of bits in the stencil buffer (e.g. <code>stencil & 0xFF</code> for an 8-bit stencil buffer).
        \note This state is guaranteed to be persistent and can be used outside of command buffer encoding.
        \see Clear
        */
        virtual void SetClearStencil(std::uint32_t stencil) = 0;

        /**
        \brief Clears the specified group of attachments of the active render target.
        \param[in] flags Specifies the clear buffer flags.
        This can be a bitwise OR combination of the ClearFlags enumeration entries.
        If this contains the ClearFlags::Color bit, all color attachments of the active render target are cleared with the color previously set by \c SetClearColor.
        \remarks To specify the clear values for each buffer type, use the respective <code>SetClear...</code> function.
        To clear only a specific render-target color buffer, use the \c ClearAttachments function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        For efficiency reasons, it is recommended to clear the render target attachments when a new render pass begins,
        i.e. the clear values of the \c BeginRenderPass function should be prefered over this function.
        For some render systems (e.g. Metal) this function forces the current render pass to stop and start again in order to clear the attachments.
        \see ClearFlags
        \see SetClearColor
        \see SetClearDepth
        \see SetClearStencil
        \see ClearAttachments
        \see BeginRenderPass
        */
        virtual void Clear(long flags) = 0;

        /**
        \brief Clears the specified attachments of the active render target.
        \param[in] numAttachments Specifies the number of attachments to clear.
        \param[in] attachments Pointer to the array of attachment clear commands. This must not be null!
        \remarks To clear all color buffers with the same value, use the \c Clear function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        For efficiency reasons, it is recommended to clear the render target attachments when a new render pass begins,
        i.e. the clear values of the \c BeginRenderPass function should be prefered over this function.
        For some render systems (e.g. Metal) this function forces the current render pass to stop and start again in order to clear the attachments.
        \see Clear
        \see BeginRenderPass
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
        where the depth attachment (i.e. RenderPassDescriptor::depthAttachment) and
        the stencil attachment (i.e. RenderPassDescriptor::stencilAttachment) are combined and appear as the last entry.
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
        \remarks
        The following commands can only appear inside a render pass section:
        - Drawing commands (i.e. <code>Draw</code>, <code>DrawInstanced</code>, <code>DrawIndexed</code>, and <code>DrawIndexedInstanced</code>).
        - Clear attachment commands (i.e. <code>Clear</code>, and <code>ClearAttachments</code>).
        \remarks
        The following commands can only appear outside a render pass section:
        - Dispatch compute commands (i.e. <code>Dispatch</code>).
        - Resource read/write from the RenderSystem (i.e. <code>WriteBuffer</code>, <code>MapBuffer</code> etc.).
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
        ) = 0;

        /**
        \brief Ends the current render pass.
        \see BeginRenderPass
        */
        virtual void EndRenderPass() = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Sets the active graphics pipeline state.
        \param[in] graphicsPipeline Specifies the graphics pipeline state to set.
        \remarks This will set all blending-, rasterizer-, depth-, stencil-, and shader states.
        A valid graphics pipeline must always be set before any drawing operation can be performed,
        and a graphics pipeline can only be set inside a render pass.
        \code
        // First set render target
        myCmdBuffer->BeginRenderPass(...);
        {
            // Then set graphics pipeline
            myCmdBuffer->SetGraphicsPipeline(...);

            // Then perform drawing operations
            myCmdBuffer->SetGraphicsResourceHeap(...);
            myCmdBuffer->Draw(...);
        }
        myCmdBuffer->EndRenderPass();
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

        #if 0//TODO: enable this
        /**
        \brief Sets the dynamic pipeline state for blending factors.
        \param[in] color Specifies the blending factors for each color component.
        The default value is <code>{ 1, 1, 1, 1 }</code>.
        \remarks This is only used for the following blending operations:
        - BlendOp::BlendFactor
        - BlendOp::InvBlendFactor
        */
        virtual void SetBlendFactor(const ColorRGBAf& color) = 0;
        #endif

        /* ----- Queries ----- */

        /**
        \brief Begins a query of the specified query heap.
        \param[in] queryHeap Specifies the query heap.
        \param[in] query Specifies the zero-based index of the query within the heap to begin with. By default 0.
        This must be in the half-open range [0, QueryHeapDescriptor::numQueries).
        \remarks The \c BeginQuery and \c EndQuery functions can be wrapped around any drawing and/or compute operation.
        This can be an occlusion query for instance, which determines how many fragments have passed the depth test.
        The result of a query can be retrieved by the command queue after this command buffer has been submitted.
        \see EndQuery
        \see RenderSystem::CreateQueryHeap
        \see CommandQueue::QueryResult
        */
        virtual void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) = 0;

        /**
        \brief Ends the specified query.
        \see BeginQuery
        */
        virtual void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) = 0;

        /**
        \brief Begins conditional rendering with the specified query object.
        \param[in] queryHeap Specifies the query heap.
        This query heap must have been created with the \c renderCondition member set to \c true.
        \param[in] query Specifies the zero-based index of the query within the heap which is to be used as render condition. By default 0.
        This must be in the half-open range <code>[0, QueryHeapDescriptor::numQueries)</code>.
        \param[in] mode Specifies the mode of the render condition.
        \remarks Here is a usage example:
        \code
        myCmdBuffer->BeginQuery(*myOcclusionQuery);
        // draw bounding box ...
        myCmdBuffer->EndQuery(*myOcclusionQuery);
        myCmdBuffer->BeginRenderCondition(*myOcclusionQuery, LLGL::RenderConditionMode::Wait);
        // draw actual object ...
        myCmdBuffer->EndRenderCondition();
        \endcode
        \see RenderSystem::CreateQueryHeap
        \see QueryHeapDescriptor::renderCondition
        */
        virtual void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) = 0;

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
        \note The parameter \c firstVertex modifies the vertex ID within the shader pipeline differently for \c SV_VertexID
        in HLSL and \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan), due to rendering API differences.
        The system value \c SV_VertexID in HLSL will always start with zero,
        but the system value \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan)
        will start with the value of \c firstVertex.
        */
        virtual void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) = 0;

        //! \see DrawIndexed(std::uint32_t, std::uint32_t, std::int32_t)
        virtual void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) = 0;

        /**
        \brief Draws the specified amount of primitives from the currently set vertex- and index buffers.
        \param[in] numIndices Specifies the number of indices to generate.
        \param[in] firstIndex Specifies the zero-based offset of the first index from the index buffer.
        \param[in] vertexOffset Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
        */
        virtual void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) = 0;

        //! \see DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
        virtual void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) = 0;

        /**
        \brief Draws the specified amount of instances of primitives from the currently set vertex buffer.
        \param[in] numVertices Specifies the number of vertices to generate.
        \param[in] firstVertex Specifies the zero-based offset of the first vertex from the vertex buffer.
        \param[in] numInstances Specifies the number of instances to generate.
        \param[in] firstInstance Specifies the zero-based offset of the first instance.
        \note The parameter \c firstVertex modifies the vertex ID within the shader pipeline differently for \c SV_VertexID
        in HLSL and \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan), due to rendering API differences.
        The system value \c SV_VertexID in HLSL will always start with zero,
        but the system value \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan)
        will start with the value of \c firstVertex.
        The same holds true for the parameter \c firstInstance and the system values \c SV_InstanceID in HLSL and \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan).
        \see RenderingFeatures::hasInstancing
        \see RenderingFeatures::hasOffsetInstancing
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
        \note The parameter \c firstInstance modifies the instance ID within the shader pipeline differently for \c SV_InstanceID
        in HLSL and \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan), due to rendering API differences.
        The system value \c SV_InstanceID in HLSL will always start with zero,
        but the system value \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan)
        will start with the value of \c firstInstance.
        \see RenderingFeatures::hasInstancing
        \see RenderingFeatures::hasOffsetInstancing
        */
        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose draw command arguments are taken from a buffer object.
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BufferFlags::IndirectBinding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DrawIndirectArguments
        \see RenderingFeatures::hasIndirectDrawing
        */
        virtual void DrawIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose draw command arguments are taken from a buffer object.
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BufferFlags::IndirectBinding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \param[in] numCommands Specifies the number of draw commands that are to be taken from the argument buffer.
        \param[in] stride Specifies the stride (in bytes) betweeen consecutive sets of arguments,
        which is commonly greater than or euqal to <code>sizeof(DrawIndirectArguments)</code>. This stride must be a multiple of 4.
        \remarks This is also known as a "multi draw command" which is only natively supported by OpenGL and Vulkan.
        For other rendering APIs, the recording of multiple draw commands is emulated with a simple loop, which is equivalent to the following example:
        \code
        while (numCommands-- > 0)
        {
            DrawIndirect(buffer, offset);
            offset += stride;
        }
        \endcode
        \see DrawIndirectArguments
        \see RenderingFeatures::hasIndirectDrawing
        */
        virtual void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose indexed draw command arguments are taken from a buffer object.
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BufferFlags::IndirectBinding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DrawIndexedIndirectArguments
        */
        virtual void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose indexed draw command arguments are taken from a buffer object.
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BufferFlags::IndirectBinding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \param[in] numCommands Specifies the number of draw commands that are to be taken from the argument buffer.
        \param[in] stride Specifies the stride (in bytes) betweeen consecutive sets of arguments,
        which is commonly greater than or euqal to <code>sizeof(DrawIndexedIndirectArguments)</code>. This stride must be a multiple of 4.
        \remarks This is also known as a "multi draw command" which is only natively supported by OpenGL and Vulkan.
        For other rendering APIs, the recording of multiple draw commands is emulated with a simple loop, which is equivalent to the following example:
        \code
        while (numCommands-- > 0)
        {
            DrawIndexedIndirect(buffer, offset);
            offset += stride;
        }
        \endcode
        \see DrawIndexedIndirectArguments
        */
        virtual void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) = 0;

        /* ----- Compute ----- */

        /**
        \brief Dispatches a compute command.
        \param[in] numWorkGroupsX Specifies the number of worker thread groups in the X-dimension.
        \param[in] numWorkGroupsY Specifies the number of worker thread groups in the Y-dimension.
        \param[in] numWorkGroupsZ Specifies the number of worker thread groups in the Z-dimension.
        \see SetComputePipeline
        \see RenderingLimits::maxComputeShaderWorkGroups
        */
        virtual void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) = 0;

        /**
        \brief Dispatches a compute command with an unknown amount of thread grounds.
        \param[in] buffer Specifies the buffer from which the dispatch command arguments are taken. This buffer must have been created with the BufferFlags::IndirectBinding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DispatchIndirectArguments
        */
        virtual void DispatchIndirect(Buffer& buffer, std::uint64_t offset) = 0;

    protected:

        CommandBuffer() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
