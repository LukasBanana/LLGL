/*
 * CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMMAND_BUFFER_H
#define LLGL_COMMAND_BUFFER_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/RenderSystemFlags.h>

#include <LLGL/Buffer.h>
#include <LLGL/BufferArray.h>
#include <LLGL/ResourceHeap.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Constants.h>

#include <LLGL/RenderPass.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/Shader.h>
#include <LLGL/PipelineState.h>
#include <LLGL/QueryHeap.h>

#include <cstdint>


namespace LLGL
{


class SwapChain;

/**
\brief Command buffer interface used for storing and encoding GPU commands.
\remarks This is the main interface to encode graphics, compute, and blit commands to be submitted to the GPU.
All states that can be changed with a setter function are not persistent across several encoding sections.
Before any command can be encoded, the command buffer must be set into encode mode, which is done by the CommandBuffer::Begin function.
\see RenderSystem::CreateCommandBuffer
*/
class LLGL_EXPORT CommandBuffer : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::CommandBuffer );

    public:

        /* ----- Encoding ----- */

        /**
        \brief Begins with the encoding (also referred to as "recording") of this command buffer.
        \remarks All functions of the CommandBuffer interface must be used between a call to \c Begin and \c End.
        This function also resets all previously encoded commands.
        \see End
        */
        virtual void Begin() = 0;

        /**
        \brief Ends the encoding (also referred to as "recording") of this command buffer.
        \remarks After this call, the command buffer can be submitted to the CommandQueue or executed by a primary command buffer.
        \see Begin
        \see Execute
        \see CommandQueue::Submit(CommandBuffer&)
        */
        virtual void End() = 0;

        /**
        \brief Executes the specified deferred command buffer.
        \param[in] deferredCommandBuffer Specifies the deferred command buffer which is meant to be executed.
        This command buffer must have been created with the CommandBufferFlags::Secondary flag.
        \remarks This function can only be used by primary command buffers, i.e. command buffers that have not been created with the flag CommandBufferFlags::Secondary.
        \see CommandBufferFlags
        \todo Incomplete for: D3D12, Vulkan, Metal.
        */
        virtual void Execute(CommandBuffer& deferredCommandBuffer) = 0;

        /* ----- Blitting ----- */

        /**
        \brief Updates the data of the specified buffer during encoding the command buffer.

        \param[in] dstBuffer Specifies the destination buffer whose data is to be updated.

        \param[in] dstOffset Specifies the destination offset (in bytes) at which the buffer is to be updated.
        This offset plus the data block size (i.e. <code>dstOffset + dataSize</code>) must be less than or equal to the size of the buffer.

        \param[in] data Raw pointer to the data with which the buffer is to be updated. This <b>must not</b> be null!

        \param[in] dataSize Specifies the size (in bytes) of the data block which is to be updated.
        This is limited to 2^16 = 65536 bytes, because it may be written to the command buffer itself before it is copied to the destination buffer (depending on the backend).

        \remarks To update buffers larger than 65536 bytes, use RenderSystem::WriteBuffer or RenderSystem::MapBuffer.
        For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.
        */
        virtual void UpdateBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            const void*     data,
            std::uint16_t   dataSize
        ) = 0;

        /**
        \brief Encodes a buffer copy command for the specified buffer region.

        \param[in,out] dstBuffer Specifies the destination buffer whose data is to be updated.

        \param[in] dstOffset Specifies the destination offset (in bytes) at which the destination buffer is to be updated.
        This offset plus the size (i.e. <code>dstOffset + size</code>) must be less than or equal to the size of the destination buffer.

        \param[in] srcBuffer Specifies the source buffer whose data is to be read from.

        \param[in] srcOffset Specifies the source offset (in bytes) at which the source buffer is to be read from.
        This offset plus the size (i.e. <code>srcOffset + size</code>) must be less than or equal to the size of the source buffer.

        \param[in] size Specifies the size of the buffer region to copy.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.
        */
        virtual void CopyBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            Buffer&         srcBuffer,
            std::uint64_t   srcOffset,
            std::uint64_t   size
        ) = 0;

        /**
        \brief Encodes a buffer copy command that blits data from a source texture.

        \param[in,out] dstBuffer Specifies the destination buffer whose data is to be updated.
        This buffer must have been created with the binding flag BindFlags::CopyDst.

        \param[in] dstOffset Specifies the destination offset (in bytes) at which the source buffer is to be updated. This \b must be a multiple of 4.

        \param[in] srcTexture Specifies the source texture whose data is to be read from.
        This texture must have been created with the binding flag BindFlags::CopySrc and
        its format <b>must not</b> be compressed (see FormatFlags::IsCompressed) or packed (see FormatFlags::IsPacked).

        \param[in] srcRegion Specifies the source region where the texture is to be read from.
        Note that the \c numMipLevels attribute of this parameter \b must be 1.

        \param[in] rowStride Specifies an optional stride (in bytes) per row in the destination buffer. By default 0.

        \param[in] layerStride Specifies an optional stride (in bytes) per layer in the destination buffer.
        This \b must be a multiple of \c rowStride. If \c rowStride is zero, then \c layerStride must also be zero. By default 0.

        \remarks This is called "copy buffer from texture" instead of "copy texture to buffer"
        to be uniform with the notation <code>buffer := texture</code>, or <code>memcpy(destination, source, size)</code>.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.

        \remarks Further performance penalties can be introduced if \c rowStride is not aligned to the respective rendering API restrictions:
        - Direct3D 12: \c rowStride \b should be a multiple of 256.
        - Metal: \c rowStride \b should be less than or equal to 32767 multiplied by the source texture's format size.

        \remarks If \c rowStride is 0, the source data is considered to be tightly packed for each array layer and the required alignment is managed automatically.
        \remarks If \c rowStride is not 0, it \b must be greater than or equal to the size (in bytes) of each row in the texture region with respect to the texture's format.
        \remarks The same rules of \c rowStride also apply to \c layerStride.

        \see CopyTextureFromBuffer
        \see GetMemoryFootprint
        \see Texture::GetSubresourceFootprint
        */
        virtual void CopyBufferFromTexture(
            Buffer&                 dstBuffer,
            std::uint64_t           dstOffset,
            Texture&                srcTexture,
            const TextureRegion&    srcRegion,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) = 0;

        /**
        \brief Fills the destination buffer with copies of the specified 32-bit value.

        \param[in,out] dstBuffer Specifies the destination buffer whose data is to be updated.
        This buffer must have been created with the binding flag BindFlags::CopyDst.
        This command works with all kinds of buffers, but for performance reasons it is recommended to create this buffer with the binding flag BindFlags::Storage.
        Otherwise, an intermediate buffer might be created and copied by LLGL.

        \param[in] dstOffset Specifies the destination offset (in bytes) at which the destination buffer is to be updated.

        \param[in] value Specifies the 32-bit value to fill the buffer with.

        \param[in] fillSize Specifies the fill size (in bytes) of the buffer region. This \b must be a multiple of 4. By default Constants::wholeSize.
        If this is equal to \c Constants::wholeSize, \c dstOffset is ignored and the entire buffer will be filled.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.
        */
        virtual void FillBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            std::uint32_t   value,
            std::uint64_t   fillSize    = Constants::wholeSize
        ) = 0;

        /**
        \brief Encodes a texture copy command for the specified texture regions.

        \param[in,out] dstTexture Specifies the destination texture whose data is to be updated.

        \param[in] dstLocation Specifies the destination location, including MIP-map level and offset.
        Its offset plus the extent (i.e. <code>dstLocation.offset + extent</code>) must be less than or equal to the size of the destination texture.

        \param[in] srcTexture Specifies the source texture whose data is to be read from.

        \param[in] srcLocation Specifies the source location, including MIP-map level and offset.
        Its offset plus the extent (i.e. <code>srcLocation.offset + extent</code>) must be less than or equal to the size of the source texture.

        \param[in] extent Specifies the extent of the texture region to copy (see TextureDescriptor::extent).
        For this function, the extent also includes the array layers, i.e. \c y component for 1D arrays (TextureType::Texture1DArray),
        and \c z component for 2D and cube arrays (TextureType::Texture2DArray and TextureType::TextureCubeArray).

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.
        */
        virtual void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
        ) = 0;

        /**
        \brief Encodes a texture copy command that blits data from a source buffer.

        \param[in,out] dstTexture Specifies the destination texture whose data is to be updated.
        This texture must have been created with the binding flag BindFlags::CopyDst and
        its format <b>must not</b> be compressed (see FormatFlags::IsCompressed) or packed (see FormatFlags::IsPacked).

        \param[in] dstRegion Specifies the destination region where the texture is to be updated.
        Note that the \c numMipLevels attribute of this parameter \b must be 1.

        \param[in] srcBuffer Specifies the source buffer whose data is to be read from.
        This buffer must have been created with the binding flag BindFlags::CopySrc.

        \param[in] srcOffset Specifies the source offset (in bytes) at which the source buffer is to be read from. This \b must be a multiple of 4.

        \param[in] rowStride Specifies an optional stride (in bytes) per row in the source buffer. By default 0.

        \param[in] layerStride Specifies an optional stride (in bytes) per layer in the source buffer.
        This \b must be a multiple of \c rowStride. If \c rowStride is zero, then \c layerStride must also be zero. By default 0.

        \remarks This is called "copy texture from buffer" instead of "copy buffer to texture"
        to be uniform with the notation <code>texture := buffer</code>, or <code>memcpy(destination, source, size)</code>.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.

        \remarks Further performance penalties can be introduced if \c rowStride is not aligned to the respective rendering API restrictions:
        - Direct3D 12: \c rowStride \b should be a multiple of 256.
        - Metal: \c rowStride \b should be less than or equal to 32767 multiplied by the destination texture's format size.

        \remarks If \c rowStride is 0, the source data is considered to be tightly packed for each array layer and the required alignment is managed automatically.
        \remarks If \c rowStride is not 0, it \b must be greater than or equal to the size (in bytes) of each row in the texture region with respect to the texture's format.
        \remarks The same rules of \c rowStride also apply to \c layerStride.

        \see CopyBufferFromTexture
        \see GetMemoryFootprint
        \see Texture::GetSubresourceFootprint
        */
        virtual void CopyTextureFromBuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            Buffer&                 srcBuffer,
            std::uint64_t           srcOffset,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) = 0;

        /**
        \brief Encodes a texture copy command that blits data from the current framebuffer.

        \param[in,out] dstTexture Specifies the destination texture whose data is to be updated.
        This texture must have been created with the binding flag BindFlags::CopyDst and
        its format <b>must not</b> be compressed (see FormatFlags::IsCompressed) or packed (see FormatFlags::IsPacked).
        If the current framebuffer is multi-sampled, this texture \e can be either a multi-sampled texture with the \e same sample count as the current framebuffer
        or a single-sampled texture in which case the resource will be automatically resolved.
        If the current framebuffer is single-sampled, this texture \b must be single-sampled as well.

        \param[in] dstRegion Specifies the destination region where the texture is to be updated.
        Note that the \c subresource.numMipLevels, \c subresource.numArrayLayers, and \c extent.depth attributes of this parameter \b must be 1.

        \param[in] srcOffset Specifies the source offset at which the framebuffer is to be read from.
        If the source offset plus the destination dimension is larger the framebuffer's resolution, the behavior is undefined.

        \remarks For performance reasons, it is recommended to render a scene into a RenderTarget instead of copying the framebuffer into a texture.
        This command merely simplifies the process of capturing the framebuffer mid-scene without having to interrupt a render pass or creating an intermediate render target.

        \remarks This function is only supported for SwapChain framebuffers, not for common render targets. This functionality might be added in the future.

        \todo Add support for common render targets.

        \see RenderTarget::GetResolution
        */
        virtual void CopyTextureFromFramebuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            const Offset2D&         srcOffset
        ) = 0;

        /**
        \brief Generates all MIP-maps for the specified texture.

        \param[in,out] texture Specifies the texture whose MIP-maps are to be generated.
        This texture must have been created with the binding flags BindFlags::Sampled and BindFlags::ColorAttachment.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.

        \see GenerateMips(Texture&, const TextureSubresource&)
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /**
        \brief Generates a range of MIP-maps for the specified texture.

        \param[in,out] texture Specifies the texture whose MIP-maps are to be generated.
        This texture must have been created with the binding flags BindFlags::Sampled and BindFlags::ColorAttachment.

        \param[in] subresource Specifies the texture subresource, i.e. the range of MIP-maps that are to be updated.

        \remarks For performance reasons, it is recommended to encode this command outside of a render pass.
        Otherwise, render pass interruptions might be inserted by LLGL.

        \remarks This function guarantees to generate only the MIP-maps in the specified range (specified by \c subresource).
        However, this function \e may introduce a performance penalty compared to generating the full MIP chain if texture views are not natively supported by the backend.
        It is therefore recommended to use this function only if the range of MIP-maps is significantly smaller than the entire MIP chain,
        e.g. only a single slice of a large 2D array texture, and use the primary \c GenerateMips function otherwise.

        \see GenerateMips(Texture&)
        \see RenderingFeatures::hasTextureViews
        */
        virtual void GenerateMips(Texture& texture, const TextureSubresource& subresource) = 0;

        /* ----- Viewport and Scissor ----- */

        /**
        \brief Sets a single viewport.
        \remarks Similar to SetViewports but only a single viewport is set.
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c viewports being empty. Otherwise, the behavior is undefined.
        \see SetViewports
        \see GraphicsPipelineDescriptor::viewports
        */
        virtual void SetViewport(const Viewport& viewport) = 0;

        /**
        \brief Sets an array of viewports.
        \param[in] numViewports Specifies the number of viewports to set. Most render system have a limit of 16 viewports.
        \param[in] viewports Pointer to the array of viewports. This <b>must not</b> be null!
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c viewports being empty. Otherwise, the behavior is undefined.
        \see GraphicsPipelineDescriptor::viewports
        \see RenderingLimits::maxViewports
        */
        virtual void SetViewports(std::uint32_t numViewports, const Viewport* viewports) = 0;

        /**
        \brief Sets a single scissor rectangle.
        \remarks Similar to SetScissors but only a single scissor rectangle is set.
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c scissors being empty. Otherwise, the behavior is undefined.
        \see SetScissors
        \see GraphicsPipelineDescriptor::scissors
        */
        virtual void SetScissor(const Scissor& scissor) = 0;

        /**
        \brief Sets an array of scissor rectangles, but only if the scissor test was enabled in the previously set graphics pipeline (otherwise, this function has no effect).
        \param[in] numScissors Specifies the number of scissor rectangles to set.
        \param[in] scissors Pointer to the array of scissor rectangles. This <b>must not</b> be null!
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c scissors being empty. Otherwise, the behavior is undefined.
        \see GraphicsPipelineDescriptor::scissors
        \see RasterizerDescriptor::scissorTestEnabled
        */
        virtual void SetScissors(std::uint32_t numScissors, const Scissor* scissors) = 0;

        /* ----- Input Assembly ------ */

        /**
        \brief Sets the specified vertex buffer for subsequent drawing operations.
        \param[in] buffer Specifies the vertex buffer to set. This buffer must have been created with the binding flag BindFlags::VertexBuffer and its content <b>must not</b> be uninitialized.
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
        \param[in] buffer Specifies the index buffer to set. This buffer must have been created with the binding flag BindFlags::IndexBuffer and its content <b>must not</b> be uninitialized.
        For this version of \c SetIndexBuffer, the index buffer must also be created with \c indexFormat set to either Format::R16UInt or Format::R32UInt.
        \remarks An index buffer is only required for any \c DrawIndexed or \c DrawIndexedInstanced draw call.
        \see RenderSystem::CreateBuffer
        \see RenderSystem::WriteBuffer
        \see DrawIndexed
        \see DrawIndexedInstanced
        \see BufferDescriptor::indexFormat
        */
        virtual void SetIndexBuffer(Buffer& buffer) = 0;

        /**
        \brief Sets the active index buffer for subsequent drawing operations with a dynamic format and optional buffer offset.
        \param[in] buffer Specifies the index buffer to set. This buffer must have been created with the binding flag BindFlags::IndexBuffer and its content <b>must not</b> be uninitialized.
        \param[in] format Specifies the format of each index in the buffer. This must be either Format::R16UInt or Format::R32UInt.
        \param[in] offset Specifies an optional offset (in bytes) where to start reading the index buffer. By default 0.
        This has the same effect as setting the \c firstIndex argument in any \c DrawIndexed or \c DrawIndexedInstanced function, except that this offset is byte aligned.
        \remarks The alternative version of this function uses merely the index format that was specified when the buffer was created.
        \see BufferDescriptor::IndexBuffer::format
        \see SetIndexBuffer(Buffer&)
        */
        virtual void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) = 0;

        /* ----- Resources ----- */

        /**
        \brief Binds the specified resource heap to the respective pipeline.
        \param[in] resourceHeap Specifies the resource heap that contains all shader resources that will be bound to the shader pipeline.
        \param[in] descriptorSet Specifies the zero-based index of the set of resource descriptors.
        This \b must be in the half-open range <code>[0, ResourceHeap::GetNumDescriptorSets)</code>. By default 0.
        \remarks Any previous heap resource bindings are invalid after this call.
        \see ResourceHeap::GetNumDescriptorSets
        \see PipelineLayoutDescriptor::heapBindings
        */
        virtual void SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet = 0) = 0;

        /**
        \brief Binds the specified resource as root parameter to the respective pipeline.
        \param[in] descriptor Specifies the zero-based index of the descriptor in the currently bound pipeline layout.
        This \b must be in the half-open range <code>[0, PipelineLayout::GetNumBindings)</code>.
        \param[in] resource Specifies the resource that is to be bound to the shader pipeline.
        \see PipelineLayoutDescriptor::bindings
        */
        virtual void SetResource(std::uint32_t descriptor, Resource& resource) = 0;

        /**
        \brief Resets the binding slots for the specified resources.

        \param[in] resourceType Specifies the type of resources to unbind.

        \param[in] firstSlot Specifies the first binding slot beginning with zero.
        This must be zero for the following resource types: ResourceType::IndexBuffer, ResourceType::StreamOutputBuffer.

        \param[in] numSlots Specifies the number of binding slots to reset. If this is zero, the function has no effect.

        \param[in] bindFlags Specifies which kind of binding slots to reset.
        This can be a bitwise OR combinations of the BindFlags entries.
        To reset a vertex buffer slot for instance, it must contain the BindFlags::VertexBuffer flag.

        \param[in] stageFlags Specifies which shader stages are affected.
        This can be a bitwise OR combination of the StageFlags entries. By default StageFlags::AllStages.

        \remarks This should be called when a resource is currently bound as shader output and will be bound as shader input for the next draw or compute commands.
        \remarks If direct resource binding is not supported by the render system, this function has no effect.

        \note Only supported with: OpenGL, Direct3D 11, Metal.
        \see BindFlags
        \see StageFlags
        */
        virtual void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) = 0;

        /* ----- Render Passes ----- */

        /**
        \brief Begins with a new render pass.

        \param[in] renderTarget Specifies the render target in which the subsequent draw operations will be stored.
        If this is a swap-chain and SwapChain::Present is called on such swap-chain before this command buffer is submitted to the command queue,
        use \c swapBufferIndex parameter to render into a specific swap buffer and then select the correct command buffer
        via SwapChain::GetCurrentSwapIndex when submitting to the command queue.

        \param[in] renderPass Specifies an optional render pass object. If this is null, the default render pass for the specified render target will be used.
        This render pass object must be compatible with the render pass object the specified render target was created with.
        Note that the default render pass will ignore the previous framebuffer content (see AttachmentLoadOp::Undefined), i.e. such a render pass section should fill the entire framebuffer.

        \param[in] numClearValues Specifies the number of clear values that are specified in the \c clearValues parameter.
        This \em should be greater than or equal to the number of render pass attachments whose load operation (i.e. AttachmentFormatDescriptor::loadOp) is set to AttachmentLoadOp::Clear.
        Otherwise, the following default values are used: (0, 0, 0, 0) for color, 1 for depth, 0 for stencil.

        \param[in] clearValues Optional pointer to the array of clear values.
        If \c numClearValues is not zero, this must be a valid pointer to an array of at least \c numClearValues entries.
        Each entry in the array is used to clear the attachment whose load operation is set to AttachmentLoadOp::Clear,
        where the depth attachment (i.e. RenderPassDescriptor::depthAttachment) and
        the stencil attachment (i.e. RenderPassDescriptor::stencilAttachment) are combined and appear as the last entry.

        \param[in] swapBufferIndex Optional index into what swap-chain buffer the render pass is meant to be rendered.
        If this is Constants::currentSwapIndex, the current buffer in the swap-chain is used.
        Otherwise, this should be the current value returned by SwapChain::GetCurrentSwapIndex.
        This parameter is ignored for regular render targets, i.e. if \c renderTarget is \e not a SwapChain.

        \remarks This function starts a new render pass section and must be ended with the \c EndRenderPass function.
        \remarks The following example shows how to use a render pass to clear a render target with two color attachments and a depth-stencil attachment:
        \code
        LLGL::ClearValue myClearValues[3];

        // Set clear values for color attachments 0 and 1
        myClearValues[0].color = { 1, 0, 0, 1 };
        myClearValues[1].color = { 0, 1, 0, 1 };

        // Set clear values for depth-stencil attachment
        myClearValues[2].depth   = 1.0f;
        myClearValues[2].stencil = 0;

        // Begin render pass and clear render target
        myCmdBuffer->BeginRenderPass(*myRenderTarget, *myRenderPass, 3, myClearValues);
        {
            // Draw scene ...
        }
        myCmdBuffer->EndRenderPass();
        \endcode

        \remarks
        The following commands \b must only be used \b inside a render pass section:
        - Drawing commands (i.e. \c Draw, \c DrawInstanced, \c DrawIndexed, \c DrawIndexedInstanced, \c DrawIndirect and \c DrawIndexedIndirect).
        - Clear attachment commands (i.e. \c Clear and \c ClearAttachments).
        - Query block (i.e. \c BeginQuery and \c EndQuery).
        - Conditional render block (i.e. \c BeginRenderCondition and \c EndRenderCondition).
        - Stream-output block (i.e. \c BeginStreamOutput and \c EndStreamOutput).

        \remarks
        The following commands \b must only be used \b outside a render pass section:
        - Dispatch compute commands (i.e. \c Dispatch and \c DispatchIndirect).

        \remarks
        The following commands \em can be used both inside and outside a render pass section but are \em recommended
        to be used only \b outside a render pass section to avoid potential performace penalties:
        - Copy commands (i.e. \c UpdateBuffer, \c CopyBuffer*, and \c CopyTexture*).
        - MIP-map generation commands (i.e. \c GenerateMips).

        \see RenderSystem::CreateRenderPass
        \see RenderSystem::CreateRenderTarget
        \see RenderTargetDescriptor::renderPass
        \see AttachmentFormatDescriptor::loadOp
        \see SwapChain::GetCurrentSwapIndex
        \see EndRenderPass
        */
        virtual void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr,
            std::uint32_t       swapBufferIndex = Constants::currentSwapIndex
        ) = 0;

        /**
        \brief Ends the current render pass.
        \see BeginRenderPass
        */
        virtual void EndRenderPass() = 0;

        /**
        \brief Clears the specified group of attachments of the active render target.

        \param[in] flags Specifies the clear buffer flags.
        This can be a bitwise OR combination of the ClearFlags enumeration entries.
        If this contains the ClearFlags::Color bit, all color attachments of the active render target are cleared with the color specified by \c clearValue.

        \param[in] clearValue Specifies the value to which the attachments will be cleared.

        \remarks To specify the clear values for each buffer type, use the respective <code>SetClear...</code> function.
        To clear only a specific render-target color buffer, use the \c ClearAttachments function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        For efficiency reasons, it is recommended to clear the render target attachments when a new render pass begins,
        i.e. the clear values of the \c BeginRenderPass function should be prefered over this function.
        For some render systems (e.g. Metal) this function forces the current render pass to stop and start again in order to clear the attachments.

        \see ClearFlags
        \see ClearAttachments
        \see BeginRenderPass
        */
        virtual void Clear(long flags, const ClearValue& clearValue = {}) = 0;

        /**
        \brief Clears the specified attachments of the active render target.

        \param[in] numAttachments Specifies the number of attachments to clear.

        \param[in] attachments Pointer to the array of attachment clear commands. This <b>must not</b> be null!

        \remarks To clear all color buffers with the same value, use the \c Clear function.
        Clearing a depth-stencil attachment while the active render target has no depth-stencil buffer is allowed but has no effect.
        For efficiency reasons, it is recommended to clear the render target attachments when a new render pass begins,
        i.e. the clear values of the \c BeginRenderPass function should be prefered over this function.
        For some render systems (e.g. Metal) this function forces the current render pass to stop and start again in order to clear the attachments.

        \see Clear
        \see BeginRenderPass
        */
        virtual void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Sets the active graphics or compute pipeline state.

        \param[in] pipelineState Specifies the pipeline state which is to be bound for subsequent draw or compute commands.

        \remarks A <b>graphics pipeline state</b> will set all blending-, rasterizer-, depth-, stencil-, and shader states.
        A valid graphics pipeline state must always be set before any drawing operation can be performed,
        and a graphics pipeline state \b can be set \b inside and \b outside a render pass section.

        \remarks A <b>compute pipeline state</b> will set shader states for dispatch compute commands.
        A valid compute pipeline state must always be set before any dispatch compute operation cam ne performed,
        and a compute pipeline state \b must be set \b outside a render pass section.
        \code
        // Set compute pipeline state and perform compute commands
        myCmdBuffer->SetPipelineState(*myComputePipeline);
        myCmdBuffer->Dispatch(...);

        // Start render pass section
        myCmdBuffer->BeginRenderPass(...);
        {
            // Set graphics pipeline state and perform drawing operations
            myCmdBuffer->SetPipelineState(*myGraphicsPipeline);
            myCmdBuffer->Draw(...);
        }
        myCmdBuffer->EndRenderPass();
        \endcode

        \see RenderSystem::CreatePipelineState
        */
        virtual void SetPipelineState(PipelineState& pipelineState) = 0;

        /**
        \brief Sets the dynamic pipeline state for blending factors.
        \param[in] color Specifies the blending factors for each color component as an array of four floating-point numbers. The default value is (1, 1, 1, 1).
        \remarks This is only used for the following blending operations:
        - BlendOp::BlendFactor
        - BlendOp::InvBlendFactor
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c blendFactorDynamic set to true. Otherwise, the behavior is undefined.
        \see BlendDescriptor::blendFactorDynamic
        */
        virtual void SetBlendFactor(const float color[4]) = 0;

        /**
        \brief Sets the dynamic pipeline state for stencil reference values.
        \param[in] reference Specifies the reference value.
        \param[in] stencilFace Specifies the faces that will be affected by this reference value.
        For Direct3D renderers, this must be StencilFace::FrontAndBack, which is the default value.
        \remarks This must only be used if the currently bound graphics pipeline state was created with \c referenceDynamic set to true. Otherwise, the behavior is undefined.
        \see StencilDescriptor::referenceDynamic
        */
        virtual void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) = 0;

        /**
        \brief Sets the value of a certain number of shader uniforms (aka. push constant/ shader constants) in the currently bound PSO.

        \param[in] first Specifies the zero-based index of the first uniform that are to be updated.
        This \b must be in the half-open range <code>[0, PipelineLayout::GetNumUniforms)</code>.
        The number of uniforms that are to be updated is determined by the size of the data. See \c dataSize parameter for more details.

        \param[in] data Raw pointer to the data that is to be copied to the uniform.

        \param[in] dataSize Specifies the size (in bytes) of the input buffer \c data.
        This \b must be a multiple of 4 since 32-bits are the smallest granularity to update shader uniforms.
        This parameter also determines the number of uniforms that are to be updated.

        \remarks This function must only be called after a pipeline state object (PSO) has been bound.

        \see PipelineLayoutDescriptor::uniforms
        \see SetPipelineState
        */
        virtual void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize) = 0;

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

        /* ----- Stream Output ------ */

        /**
        \brief Begins a stream-output section for subsequent draw calls.

        \param[in] numBuffers Specifies the number of stream-output buffers. This must be in the range <code>[1, RenderingLimits::maxStreamOutputs]</code>.

        \param[in] buffers Array to the stream-output buffers. This must be a valid pointer to an array of \c numBuffers buffer objects.
        Each of these buffers must have been created with the binding flag BindFlags::StreamOutputBuffer.

        \remarks This must only be called if a graphics pipeline is currently bound.

        \see EndStreamOutput
        \see SetPipelineState
        \see RenderingFeatures::hasStreamOutputs
        \see RenderingLimits::maxStreamOutputs
        */
        virtual void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers) = 0;

        /**
        \brief Ends the current stream-output.
        \see BeginStreamOutput
        */
        virtual void EndStreamOutput() = 0;

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
        \note For the Metal renderer, the parameter \c vertexOffset is ignored when tessellation is enabled.
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

        \note For the Metal renderer, the parameter \c vertexOffset is ignored when tessellation is enabled.

        \see RenderingFeatures::hasInstancing
        \see RenderingFeatures::hasOffsetInstancing
        */
        virtual void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose draw command arguments are taken from a buffer object.
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DrawIndirectArguments
        \see RenderingFeatures::hasIndirectDrawing
        */
        virtual void DrawIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose draw command arguments are taken from a buffer object.

        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
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
        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DrawIndexedIndirectArguments
        */
        virtual void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        /**
        \brief Draws an unknown amount of instances of primitives whose indexed draw command arguments are taken from a buffer object.

        \param[in] buffer Specifies the buffer from which the draw command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
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
        \see SetPipelineState
        \see RenderingLimits::maxComputeShaderWorkGroups
        */
        virtual void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) = 0;

        /**
        \brief Dispatches a compute command with an unknown amount of thread grounds.
        \param[in] buffer Specifies the buffer from which the dispatch command arguments are taken. This buffer must have been created with the BindFlags::IndirectBuffer binding flag.
        \param[in] offset Specifies an offset within the argument buffer from which the arguments are to be taken. This offset must be a multiple of 4.
        \see DispatchIndirectArguments
        */
        virtual void DispatchIndirect(Buffer& buffer, std::uint64_t offset) = 0;

        /* ----- Debugging ----- */

        /**
        \brief Pushes the specified name onto a stack of group strings that is used for debug reports.

        \param[in] name Pointer to a null terminated string that specifies the name. This <b>must not</b> be null!

        \remarks
        Here is a usage example:
        \code
        myCmdBuffer->PushDebugGroup("Shadow Map Pass");
        myCmdBuffer->BeginRenderPass(...);
        // render shadow map ...
        myCmdBuffer->EndRenderPass();
        myCmdBuffer->PopDebugGroup();

        myCmdBuffer->PushDebugGroup("Final Scene Pass");
        myCmdBuffer->BeginRenderPass(...);
        // render final scene ...
        myCmdBuffer->EndRenderPass();
        myCmdBuffer->PopDebugGroup();
        \endcode

        \note Only supported in debug mode or when the debug layer is enabled. Otherwise, the function has no effect.

        \see PopDebugGroup
        \see RenderSystemChild::SetName
        */
        virtual void PushDebugGroup(const char* name) = 0;

        //! \see PushDebugGroup
        virtual void PopDebugGroup() = 0;

        /* ----- Extensions ----- */

        /**
        \brief Performs a native command that is backend specific.

        \param[out] nativeCommand Raw pointer to the backend specific structure to store the native command.
        Optain the respective structure from <code>#include <LLGL/Backend/BACKEND/NativeCommand.h></code>
        where \c BACKEND must be either \c Direct3D12, \c Direct3D11, \c Vulkan, \c Metal, or \c OpenGL.

        \param[in] nativeCommandSize Specifies the size (in bytes) of the native command structure for robustness.
        This must be <code>sizeof(STRUCT)</code> where \c STRUCT is the respective backend specific structure such as \c LLGL::Metal::NativeCommand.

        \remarks This must only be used on an immediate command buffer, i.e. those that have been created with the CommandBufferFlags::ImmediateSubmit flag.
        \remarks This can be used to work around several differences between the low-level graphics APIs, e.g. for internal buffer binding slots.
        Here is a usage example:
        \code
        LLGL::Metal::NativeCommand myMetalCommand;
        myMetalCommand.type                  = LLGL::Metal::NativeCommandType::TessFactorBuffer;
        myMetalCommand.tessFactorBuffer.slot = 1;
        myCmdBuffer->DoNativeCommand(&myMetalCommand, sizeof(myMetalCommand));
        \endcode

        \see Direct3D12::NativeCommand
        \see Direct3D11::NativeCommand
        \see Vulkan::NativeCommand
        \see Metal::NativeCommand
        \see OpenGL::NativeCommand
        */
        virtual void DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize) = 0;

        /**
        \brief Returns the native command buffer handle.

        \param[out] nativeHandle Raw pointer to the backend specific structure to store the native handle.
        Optain the respective structure from <code>#include <LLGL/Backend/BACKEND/NativeHandle.h></code>
        where \c BACKEND must be either \c Direct3D12, \c Direct3D11, \c Vulkan, or \c Metal.
        OpenGL does not have a native handle as it uses the current platform specific GL context.

        \param[in] nativeHandleSize Specifies the size (in bytes) of the native handle structure for robustness.
        This must be <code>sizeof(STRUCT)</code> where \c STRUCT is the respective backend specific structure such as \c LLGL::Direct3D12::CommandBufferNativeHandle.

        \return True if the native handle was successfully retrieved. Otherwise, \c nativeHandleSize specifies an incompatible structure size.

        \remarks This must only be used on an immediate command buffer, i.e. those that have been created with the CommandBufferFlags::ImmediateSubmit flag.
        \remarks For the Direct3D backends, all retrieved COM pointers will be incremented and the user is responsible for releasing those pointers,
        i.e. a call to \c IUnknown::Release is required to each of the objects returned by this function.
        \remarks For the Metal backend, all retrieved \c NSObject instances will have their retain counter incremented and the user is responsible for releasing those objects,
        i.e. a call to <code>-(oneway void)release</code> is required to each of the objects returned by this function.
        \remarks For backends that do not support this function, the return value is false unless \c nativeHandle is null or \c nativeHandleSize is 0.
        \remarks Example for obtaining the native handle of a Direct3D12 render system:
        \code
        #include <LLGL/Backend/Direct3D12/NativeHandle.h>
        //...
        LLGL::Direct3D12::CommandBufferNativeHandle d3dNativeHandle;
        myCmdBuffer->GetNativeHandle(&d3dNativeHandle, sizeof(d3dNativeHandle));
        ID3D12GraphicsCommandList* d3dCommandList = d3dNativeHandle.commandList;
        ...
        d3dCommandList->Release();
        \endcode

        \note Only supported with: Direct3D 12, Direct3D 11, Vulkan, Metal.

        \see Direct3D12::CommandBufferNativeHandle
        \see Direct3D11::CommandBufferNativeHandle
        \see Vulkan::CommandBufferNativeHandle
        \see Metal::CommandBufferNativeHandle
        */
        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) = 0;

    protected:

        CommandBuffer() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
