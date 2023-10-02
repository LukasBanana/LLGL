/*
 * VKCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCommandBuffer.h"
#include "VKCommandQueue.h"
#include "VKPhysicalDevice.h"
#include "VKSwapChain.h"
#include "VKTypes.h"
#include "Ext/VKExtensionRegistry.h"
#include "Ext/VKExtensions.h"
#include "RenderState/VKRenderPass.h"
#include "RenderState/VKGraphicsPSO.h"
#include "RenderState/VKComputePSO.h"
#include "RenderState/VKResourceHeap.h"
#include "RenderState/VKPredicateQueryHeap.h"
#include "Texture/VKSampler.h"
#include "Texture/VKTexture.h"
#include "Texture/VKRenderTarget.h"
#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"
#include "../CheckedCast.h"
#include "../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Constants.h>
#include <LLGL/TypeInfo.h>
#include <cstddef>

#include <LLGL/Backend/Vulkan/NativeHandle.h>


namespace LLGL
{


constexpr std::uint32_t VKCommandBuffer::maxNumCommandBuffers;

// Returns the maximum for a indirect multi draw command
static std::uint32_t GetMaxDrawIndirectCount(const VKPhysicalDevice& physicalDevice)
{
    if (physicalDevice.GetFeatures().multiDrawIndirect != VK_FALSE)
        return physicalDevice.GetProperties().limits.maxDrawIndirectCount;
    else
        return 1u;
}

VKCommandBuffer::VKCommandBuffer(
    const VKPhysicalDevice&         physicalDevice,
    VKDevice&                       device,
    VkQueue                         commandQueue,
    const QueueFamilyIndices&       queueFamilyIndices,
    const CommandBufferDescriptor&  desc)
:
    device_                 { device                                        },
    commandQueue_           { commandQueue                                  },
    commandPool_            { device, vkDestroyCommandPool                  },
    numCommandBuffers_      { VKCommandBuffer::GetNumVkCommandBuffers(desc) },
    queuePresentFamily_     { queueFamilyIndices.presentFamily              },
    maxDrawIndirectCount_   { GetMaxDrawIndirectCount(physicalDevice)       },
    recordingFenceArray_    { VKPtr<VkFence>{ device, vkDestroyFence },
                              VKPtr<VkFence>{ device, vkDestroyFence },
                              VKPtr<VkFence>{ device, vkDestroyFence }      },
    descriptorSetPoolArray_ { device.GetVkDevice().Get(),
                              device.GetVkDevice().Get(),
                              device.GetVkDevice().Get()                    }
{
    /* Translate creation flags */
    VkCommandBufferLevel bufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if ((desc.flags & CommandBufferFlags::ImmediateSubmit) != 0)
    {
        immediateSubmit_    = true;
        usageFlags_         = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    else
    {
        if ((desc.flags & CommandBufferFlags::Secondary) != 0)
        {
            bufferLevel = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            usageFlags_ |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        }
        if ((desc.flags & CommandBufferFlags::MultiSubmit) == 0)
            usageFlags_ |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    /* Create native command buffer objects */
    CreateVkCommandPool(queueFamilyIndices.graphicsFamily);
    CreateVkCommandBuffers(bufferLevel);
    CreateVkRecordingFences();

    /* Acquire first native command buffer */
    AcquireNextBuffer();
}

VKCommandBuffer::~VKCommandBuffer()
{
    vkFreeCommandBuffers(device_, commandPool_, numCommandBuffers_, commandBufferArray_);
}

/* ----- Encoding ----- */

void VKCommandBuffer::Begin()
{
    /* Use next internal VkCommandBuffer object to reduce latency */
    AcquireNextBuffer();

    /* Wait for fence before recording */
    vkWaitForFences(device_, 1, &recordingFence_, VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &recordingFence_);

    /* Begin recording of current command buffer */
    VkCommandBufferBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.flags             = usageFlags_;
        beginInfo.pInheritanceInfo  = nullptr;
    }
    VkResult result = vkBeginCommandBuffer(commandBuffer_, &beginInfo);
    VKThrowIfFailed(result, "failed to begin Vulkan command buffer");

    #if 0//TODO: optimize
    /* Reset all query pools that were in flight during last encoding */
    ResetQueryPoolsInFlight();
    #endif

    /* Store new record state */
    recordState_ = RecordState::OutsideRenderPass;
}

void VKCommandBuffer::End()
{
    /* End encoding of current command buffer */
    VkResult result = vkEndCommandBuffer(commandBuffer_);
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");

    /* Store new record state */
    recordState_ = RecordState::ReadyForSubmit;

    /* Execute command buffer right after encoding for immediate command buffers */
    if (IsImmediateCmdBuffer())
    {
        VkResult result = VKSubmitCommandBuffer(commandQueue_, commandBuffer_, GetQueueSubmitFence());
        VKThrowIfFailed(result, "failed to submit command buffer to Vulkan graphics queue");
    }

    ResetBindingStates();
}

void VKCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferVK = LLGL_CAST(VKCommandBuffer&, deferredCommandBuffer);
    VkCommandBuffer cmdBuffers[] = { cmdBufferVK.GetVkCommandBuffer() };
    vkCmdExecuteCommands(commandBuffer_, 1, cmdBuffers);
}

/* ----- Blitting ----- */

void VKCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);

    auto size   = static_cast<VkDeviceSize>(dataSize);
    auto offset = static_cast<VkDeviceSize>(dstOffset);

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        vkCmdUpdateBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, data);
        BufferPipelineBarrier(dstBufferVK.GetVkBuffer(), offset, size);
        ResumeRenderPass();
    }
    else
    {
        vkCmdUpdateBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, data);
        BufferPipelineBarrier(dstBufferVK.GetVkBuffer(), offset, size);
    }
}

void VKCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);
    auto& srcBufferVK = LLGL_CAST(VKBuffer&, srcBuffer);

    VkBufferCopy region;
    {
        region.srcOffset    = static_cast<VkDeviceSize>(srcOffset);
        region.dstOffset    = static_cast<VkDeviceSize>(dstOffset);
        region.size         = static_cast<VkDeviceSize>(size);
    }

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        vkCmdCopyBuffer(commandBuffer_, srcBufferVK.GetVkBuffer(), dstBufferVK.GetVkBuffer(), 1, &region);
        ResumeRenderPass();
    }
    else
        vkCmdCopyBuffer(commandBuffer_, srcBufferVK.GetVkBuffer(), dstBufferVK.GetVkBuffer(), 1, &region);
}

void VKCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);
    auto& srcTextureVK = LLGL_CAST(VKTexture&, srcTexture);

    VkBufferImageCopy region;
    {
        region.bufferOffset                     = dstOffset;
        region.bufferRowLength                  = rowStride;
        region.bufferImageHeight                = layerStride;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = srcRegion.subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = srcRegion.subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = srcRegion.subresource.numArrayLayers;
        region.imageOffset                      = VKTypes::ToVkOffset(srcRegion.offset);
        region.imageExtent                      = VKTypes::ToVkExtent(srcRegion.extent);
    }

    VkImageLayout oldLayout = srcTextureVK.TransitionImageLayout(device_, commandBuffer_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        device_.CopyImageToBuffer(commandBuffer_, srcTextureVK, dstBufferVK, region);
        ResumeRenderPass();
    }
    else
        device_.CopyImageToBuffer(commandBuffer_, srcTextureVK, dstBufferVK, region);

    srcTextureVK.TransitionImageLayout(device_, commandBuffer_, oldLayout);
}

void VKCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);

    /* Determine destination buffer range and ignore <dstOffset> if the whole buffer is meant to be filled */
    VkDeviceSize offset, size;
    if (fillSize == LLGL_WHOLE_SIZE)
    {
        offset  = 0;
        size    = VK_WHOLE_SIZE;
    }
    else
    {
        offset  = static_cast<VkDeviceSize>(dstOffset);
        size    = static_cast<VkDeviceSize>(fillSize);
    }

    /* Encode fill buffer command */
    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        vkCmdFillBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, value);
        ResumeRenderPass();
    }
    else
        vkCmdFillBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, value);
}

void VKCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureVK = LLGL_CAST(VKTexture&, dstTexture);
    auto& srcTextureVK = LLGL_CAST(VKTexture&, srcTexture);

    VkImageCopy region;
    {
        region.srcSubresource.aspectMask        = srcTextureVK.GetAspectFlags();
        region.srcSubresource.mipLevel          = srcLocation.mipLevel;
        region.srcSubresource.baseArrayLayer    = srcLocation.arrayLayer;
        region.srcSubresource.layerCount        = 1;
        region.srcOffset                        = VKTypes::ToVkOffset(srcLocation.offset);
        region.dstSubresource.aspectMask        = dstTextureVK.GetAspectFlags();
        region.dstSubresource.mipLevel          = dstLocation.mipLevel;
        region.dstSubresource.baseArrayLayer    = dstLocation.arrayLayer;
        region.dstSubresource.layerCount        = 1;
        region.dstOffset                        = VKTypes::ToVkOffset(dstLocation.offset);
        region.extent                           = VKTypes::ToVkExtent(extent);
    }

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        device_.CopyTexture(commandBuffer_, srcTextureVK, dstTextureVK, region);
        ResumeRenderPass();
    }
    else
        device_.CopyTexture(commandBuffer_, srcTextureVK, dstTextureVK, region);
}

void VKCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureVK = LLGL_CAST(VKTexture&, dstTexture);
    auto& srcBufferVK = LLGL_CAST(VKBuffer&, srcBuffer);

    VkBufferImageCopy region;
    {
        region.bufferOffset                     = srcOffset;
        region.bufferRowLength                  = rowStride;
        region.bufferImageHeight                = layerStride;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = dstRegion.subresource.baseMipLevel;
        region.imageSubresource.baseArrayLayer  = dstRegion.subresource.baseArrayLayer;
        region.imageSubresource.layerCount      = dstRegion.subresource.numArrayLayers;
        region.imageOffset                      = VKTypes::ToVkOffset(dstRegion.offset);
        region.imageExtent                      = VKTypes::ToVkExtent(dstRegion.extent);
    }

    VkImageLayout oldLayout = dstTextureVK.TransitionImageLayout(device_, commandBuffer_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        device_.CopyBufferToImage(commandBuffer_, srcBufferVK, dstTextureVK, region);
        ResumeRenderPass();
    }
    else
        device_.CopyBufferToImage(commandBuffer_, srcBufferVK, dstTextureVK, region);

    dstTextureVK.TransitionImageLayout(device_, commandBuffer_, oldLayout);
}

void VKCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    if (boundSwapChain_ == nullptr)
        return /*No bound framebuffer*/;

    if (dstRegion.extent.depth != 1 ||
        dstRegion.offset.x < 0      ||
        dstRegion.offset.y < 0      ||
        dstRegion.offset.z < 0)
    {
        return /*Out of bounds*/;
    }

    auto& dstTextureVK = LLGL_CAST(VKTexture&, dstTexture);

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        boundSwapChain_->CopyImage(
            device_,
            commandBuffer_,
            dstTextureVK.GetVkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED, //TODO: use state management of image layouts
            dstRegion,
            currentColorBuffer_,
            srcOffset,
            dstTextureVK.GetVkFormat()
        );
        ResumeRenderPass();
    }
    else
    {
        boundSwapChain_->CopyImage(
            device_,
            commandBuffer_,
            dstTextureVK.GetVkImage(),
            VK_IMAGE_LAYOUT_UNDEFINED, //TODO: use state management of image layouts
            dstRegion,
            currentColorBuffer_,
            srcOffset,
            dstTextureVK.GetVkFormat()
        );
    }
}

void VKCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);
    device_.GenerateMips(
        commandBuffer_,
        textureVK.GetVkImage(),
        textureVK.GetVkFormat(),
        textureVK.GetVkExtent(),
        TextureSubresource{ 0, textureVK.GetNumArrayLayers(), 0, textureVK.GetNumMipLevels() }
    );
}

void VKCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);

    const auto maxNumMipLevels      = textureVK.GetNumMipLevels();
    const auto maxNumArrayLayers    = textureVK.GetNumArrayLayers();

    if (subresource.baseMipLevel   < maxNumMipLevels   && subresource.numMipLevels   > 0 &&
        subresource.baseArrayLayer < maxNumArrayLayers && subresource.numArrayLayers > 0)
    {
        device_.GenerateMips(
            commandBuffer_,
            textureVK.GetVkImage(),
            textureVK.GetVkFormat(),
            textureVK.GetVkExtent(),
            subresource
        );
    }
}

/* ----- Viewport and Scissor ----- */

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    /* Convert viewport to VkViewport type */
    VkViewport viewportVK;
    VKTypes::Convert(viewportVK, viewport);
    vkCmdSetViewport(commandBuffer_, 0, 1, &viewportVK);
}

void VKCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    VkViewport viewportsVK[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

    /* Convert viewport to VkViewport types */
    numViewports = std::min(numViewports, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
    for (std::uint32_t i = 0; i < numViewports; ++i)
        VKTypes::Convert(viewportsVK[i], viewports[i]);

    vkCmdSetViewport(commandBuffer_, 0, numViewports, viewportsVK);
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    if (scissorEnabled_)
    {
        /* Convert scissor to VkRect2D type */
        VkRect2D scissorVK;
        VKTypes::Convert(scissorVK, scissor);
        vkCmdSetScissor(commandBuffer_, 0, 1, &scissorVK);
    }
}

void VKCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (scissorEnabled_)
    {
        /* Convert scissor to VkRect2D types */
        VkRect2D scissorsVK[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];

        numScissors = std::min(numScissors, LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS);
        for (std::uint32_t i = 0; i < numScissors; ++i)
            VKTypes::Convert(scissorsVK[i], scissors[i]);

        vkCmdSetScissor(commandBuffer_, 0, numScissors, scissorsVK);
    }
}

/* ----- Input Assembly ------ */

void VKCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    VkBuffer buffers[] = { bufferVK.GetVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer_, 0, 1, buffers, offsets);
}

void VKCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayVK = LLGL_CAST(VKBufferArray&, bufferArray);
    vkCmdBindVertexBuffers(
        commandBuffer_,
        0,
        static_cast<std::uint32_t>(bufferArrayVK.GetBuffers().size()),
        bufferArrayVK.GetBuffers().data(),
        bufferArrayVK.GetOffsets().data()
    );
}

void VKCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdBindIndexBuffer(commandBuffer_, bufferVK.GetVkBuffer(), 0, bufferVK.GetIndexType());
}

void VKCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdBindIndexBuffer(commandBuffer_, bufferVK.GetVkBuffer(), offset, VKTypes::ToVkIndexType(format));
}

/* ----- Resources ----- */

void VKCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    if (boundPipelineState_ == nullptr)
        return /*No PSO bound*/;

    /* Bind resource heap to pipeline bind point and insert resource barrier into command buffer */
    auto& resourceHeapVK = LLGL_CAST(VKResourceHeap&, resourceHeap);
    if (!(descriptorSet < resourceHeapVK.GetVkDescriptorSets().size()))
        return /*Descriptor set out of bounds*/;

    boundPipelineState_->BindHeapDescriptorSet(commandBuffer_, resourceHeapVK.GetVkDescriptorSets()[descriptorSet]);
    resourceHeapVK.SubmitPipelineBarrier(commandBuffer_, descriptorSet);
}

void VKCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (boundPipelineLayout_ != nullptr && descriptor < boundPipelineLayout_->GetLayoutDynamicBindings().size())
    {
        const auto& binding = boundPipelineLayout_->GetLayoutDynamicBindings()[descriptor];
        descriptorCache_->EmplaceDescriptor(resource, binding, descriptorSetWriter_);
    }
}

void VKCommandBuffer::ResetResourceSlots(
    const ResourceType  /*resourceType*/,
    std::uint32_t       /*firstSlot*/,
    std::uint32_t       /*numSlots*/,
    long                /*bindFlags*/,
    long                /*stageFlags*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void VKCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       swapBufferIndex)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        /* Get Vulkan swap-chain object */
        auto& swapChainVK = LLGL_CAST(VKSwapChain&, renderTarget);

        /* Store information about framebuffer attachments */
        boundSwapChain_                 = &swapChainVK;
        currentColorBuffer_             = swapChainVK.TranslateSwapIndex(swapBufferIndex);
        renderPass_                     = swapChainVK.GetSwapChainRenderPass().GetVkRenderPass();
        secondaryRenderPass_            = swapChainVK.GetSecondaryVkRenderPass();
        framebuffer_                    = swapChainVK.GetVkFramebuffer(currentColorBuffer_);
        framebufferRenderArea_.extent   = swapChainVK.GetVkExtent();
        numColorAttachments_            = swapChainVK.GetNumColorAttachments();
        hasDepthStencilAttachment_      = (swapChainVK.HasDepthAttachment() || swapChainVK.HasStencilAttachment());
    }
    else
    {
        /* Get Vulkan render target object and store its extent for subsequent commands */
        auto& renderTargetVK = LLGL_CAST(VKRenderTarget&, renderTarget);

        /* Store information about framebuffer attachments */
        renderPass_                     = renderTargetVK.GetVkRenderPass();
        secondaryRenderPass_            = renderTargetVK.GetSecondaryVkRenderPass();
        framebuffer_                    = renderTargetVK.GetVkFramebuffer();
        framebufferRenderArea_.extent   = renderTargetVK.GetVkExtent();
        numColorAttachments_            = renderTargetVK.GetNumColorAttachments();
        hasDepthStencilAttachment_      = (renderTargetVK.HasDepthAttachment() || renderTargetVK.HasStencilAttachment());
    }

    scissorRectInvalidated_ = true;

    /* Uninitialized stack memory for clear values */
    VkClearValue clearValuesVK[LLGL_MAX_NUM_COLOR_ATTACHMENTS * 2 + 1];
    std::uint32_t numClearValuesVK = 0;

    /* Get native render pass object either from RenderTarget or RenderPass interface */
    if (renderPass != nullptr)
    {
        /* Get native VkRenderPass object */
        auto renderPassVK = LLGL_CAST(const VKRenderPass*, renderPass);
        renderPass_ = renderPassVK->GetVkRenderPass();
        ConvertRenderPassClearValues(*renderPassVK, numClearValuesVK, clearValuesVK, numClearValues, clearValues);
    }

    /* Record begin of render pass */
    VkRenderPassBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.renderPass        = renderPass_;
        beginInfo.framebuffer       = framebuffer_;
        beginInfo.renderArea        = framebufferRenderArea_;
        beginInfo.clearValueCount   = numClearValuesVK;
        beginInfo.pClearValues      = clearValuesVK;
    }
    vkCmdBeginRenderPass(commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    /* Store new record state */
    recordState_ = RecordState::InsideRenderPass;
}

void VKCommandBuffer::EndRenderPass()
{
    /* Record and of render pass */
    vkCmdEndRenderPass(commandBuffer_);

    /* Reset render pass and framebuffer attributes */
    renderPass_     = VK_NULL_HANDLE;
    framebuffer_    = VK_NULL_HANDLE;

    /* Store new record state */
    recordState_ = RecordState::OutsideRenderPass;
}

static void ToVkClearColor(VkClearColorValue& dst, const float (&src)[4])
{
    dst.float32[0] = src[0];
    dst.float32[1] = src[1];
    dst.float32[2] = src[2];
    dst.float32[3] = src[3];
}

static VkImageAspectFlags GetDepthStencilAspectMask(long flags)
{
    VkImageAspectFlags aspectMask = 0;

    if ((flags & ClearFlags::Depth) != 0)
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if ((flags & ClearFlags::Stencil) != 0)
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    return aspectMask;
}

void VKCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    VkClearAttachment attachments[LLGL_MAX_NUM_ATTACHMENTS];

    std::uint32_t numAttachments = 0;

    /* Fill clear descriptors for color attachments */
    if ((flags & ClearFlags::Color) != 0)
    {
        VkClearColorValue clearColor;
        ToVkClearColor(clearColor, clearValue.color);

        numAttachments = std::min(numColorAttachments_, LLGL_MAX_NUM_COLOR_ATTACHMENTS);
        for_range(i, numAttachments)
        {
            auto& attachment = attachments[i];
            {
                attachment.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
                attachment.colorAttachment  = i;
                attachment.clearValue.color = clearColor;
            }
        }
    }

    /* Fill clear descriptor for depth-stencil attachment */
    if ((flags & ClearFlags::DepthStencil) != 0 && hasDepthStencilAttachment_)
    {
        auto& attachment = attachments[numAttachments++];
        {
            attachment.aspectMask                       = GetDepthStencilAspectMask(flags);
            attachment.colorAttachment                  = 0; // ignored
            attachment.clearValue.depthStencil.depth    = clearValue.depth;
            attachment.clearValue.depthStencil.stencil  = clearValue.stencil;
        }
    }

    /* Clear all framebuffer attachments */
    ClearFramebufferAttachments(numAttachments, attachments);
}

void VKCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    /* Convert clear attachment descriptors */
    VkClearAttachment attachmentsVK[LLGL_MAX_NUM_ATTACHMENTS];

    std::uint32_t numAttachmentsVK = 0;

    for_range(i, std::min(numAttachments, LLGL_MAX_NUM_ATTACHMENTS))
    {
        auto& dst = attachmentsVK[numAttachmentsVK];
        const auto& src = attachments[i];

        if ((src.flags & ClearFlags::Color) != 0)
        {
            /* Convert color clear command */
            dst.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            dst.colorAttachment = src.colorAttachment;
            ToVkClearColor(dst.clearValue.color, src.clearValue.color);
            ++numAttachmentsVK;
        }
        else if (hasDepthStencilAttachment_)
        {
            /* Convert depth-stencil clear command */
            dst.aspectMask      = 0;
            dst.colorAttachment = 0;

            if ((src.flags & ClearFlags::Depth) != 0)
            {
                dst.aspectMask                      |= VK_IMAGE_ASPECT_DEPTH_BIT;
                dst.clearValue.depthStencil.depth   = src.clearValue.depth;
            }
            if ((src.flags & ClearFlags::Stencil) != 0)
            {
                dst.aspectMask                      |= VK_IMAGE_ASPECT_STENCIL_BIT;
                dst.clearValue.depthStencil.stencil = src.clearValue.stencil;
            }

            ++numAttachmentsVK;
        }
    }

    ClearFramebufferAttachments(numAttachmentsVK, attachmentsVK);
}

/* ----- Pipeline States ----- */

void VKCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Bind native PSO */
    auto& pipelineStateVK = LLGL_CAST(VKPipelineState&, pipelineState);
    pipelineStateVK.BindPipelineAndStaticDescriptorSet(commandBuffer_);

    /* Handle special case for graphics PSOs */
    pipelineBindPoint_ = pipelineStateVK.GetBindPoint();
    if (pipelineBindPoint_ == VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        auto& graphicsPSO = LLGL_CAST(VKGraphicsPSO&, pipelineStateVK);

        /* Scissor rectangle must be updated (if scissor test is disabled) */
        scissorEnabled_ = graphicsPSO.IsScissorEnabled();
        if (!scissorEnabled_ && scissorRectInvalidated_ && graphicsPSO.HasDynamicScissor())
        {
            /* Set scissor to render target resolution */
            vkCmdSetScissor(commandBuffer_, 0, 1, &framebufferRenderArea_);

            /* Avoid scissor update with each graphics pipeline binding (as long as render pass does not change) */
            scissorRectInvalidated_ = false;
        }
    }

    /* Keep reference to bound piepline layout (can be null) */
    boundPipelineState_     = &pipelineStateVK;
    boundPipelineLayout_    = pipelineStateVK.GetPipelineLayout();

    /* Reset descriptor cache for dynamic resources */
    if (boundPipelineLayout_ != nullptr)
    {
        descriptorCache_ = boundPipelineLayout_->GetDescriptorCache();
        if (descriptorCache_ != nullptr)
        {
            descriptorCache_->Reset();
            descriptorSetWriter_.Reset(descriptorCache_->GetNumDescriptors());
        }
    }
    else
        descriptorCache_ = nullptr;
}

void VKCommandBuffer::SetBlendFactor(const float color[4])
{
    vkCmdSetBlendConstants(commandBuffer_, color);
}

void VKCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    vkCmdSetStencilReference(commandBuffer_, VKTypes::Map(stencilFace), reference);
}

void VKCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    if (boundPipelineState_ != nullptr)
        boundPipelineState_->PushConstants(commandBuffer_, first, reinterpret_cast<const char*>(data), dataSize);
}

/* ----- Queries ----- */

void VKCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapVK = LLGL_CAST(VKQueryHeap&, queryHeap);

    query *= queryHeapVK.GetGroupSize();

    if (queryHeapVK.GetType() == QueryType::TimeElapsed)
    {
        /* Record first timestamp */
        vkCmdWriteTimestamp(commandBuffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryHeapVK.GetVkQueryPool(), query);
    }
    else
    {
        /* Begin query section */
        vkCmdBeginQuery(commandBuffer_, queryHeapVK.GetVkQueryPool(), query, queryHeapVK.GetControlFlags());
    }

    if (queryHeapVK.HasPredicates())
    {
        /* Mark dirty range for predicates */
        auto& predicateQueryHeapVK = LLGL_CAST(VKPredicateQueryHeap&, queryHeapVK);
        predicateQueryHeapVK.MarkDirtyRange(query, 1);
    }
}

void VKCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapVK = LLGL_CAST(VKQueryHeap&, queryHeap);

    query *= queryHeapVK.GetGroupSize();

    if (queryHeapVK.GetType() == QueryType::TimeElapsed)
    {
        /* Record second timestamp */
        vkCmdWriteTimestamp(commandBuffer_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryHeapVK.GetVkQueryPool(), query + 1);
    }
    else
    {
        /* End query section */
        vkCmdEndQuery(commandBuffer_, queryHeapVK.GetVkQueryPool(), query);
    }

    #if 0//TEST
    AppendQueryPoolInFlight(&queryHeapVK);
    #endif
}

void VKCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    LLGL_ASSERT_VK_EXT(EXT_conditional_rendering);

    auto& queryHeapVK = LLGL_CAST(VKPredicateQueryHeap&, queryHeap);

    /* Flush dirty range before using predicate result buffer */
    if (queryHeapVK.InsideDirtyRange(query, 1))
    {
        if (IsInsideRenderPass())
        {
            PauseRenderPass();
            queryHeapVK.FlushDirtyRange(commandBuffer_);
            ResumeRenderPass();
        }
        else
            queryHeapVK.FlushDirtyRange(commandBuffer_);
    }

    /* Begin conditional rendering block */
    VkConditionalRenderingBeginInfoEXT beginInfo;
    {
        beginInfo.sType     = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
        beginInfo.pNext     = nullptr;
        beginInfo.buffer    = queryHeapVK.GetResultVkBuffer();
        beginInfo.offset    = query * sizeof(std::uint32_t);
        beginInfo.flags     = (mode >= RenderConditionMode::WaitInverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0);
    }
    vkCmdBeginConditionalRenderingEXT(commandBuffer_, &beginInfo);
}

void VKCommandBuffer::EndRenderCondition()
{
    /* Ensure "VK_EXT_conditional_rendering" is supported */
    LLGL_ASSERT_VK_EXT(EXT_conditional_rendering);

    /* End conditional rendering block */
    vkCmdEndConditionalRenderingEXT(commandBuffer_);
}

/* ----- Stream Output ------ */

#if 0
void VKCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    LLGL_ASSERT_VK_EXT(EXT_transform_feedback);

    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    VkBuffer buffers[] = { bufferVK.GetVkBuffer() };
    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize sizes[] = { bufferVK.GetSize() };

    vkCmdBindTransformFeedbackBuffersEXT(commandBuffer_, 0, 1, buffers, offsets, sizes);
}
#endif

void VKCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    LLGL_ASSERT_VK_EXT(EXT_transform_feedback);
    //TODO: bind buffers
    vkCmdBeginTransformFeedbackEXT(commandBuffer_, 0, 0, nullptr, nullptr);
}

void VKCommandBuffer::EndStreamOutput()
{
    LLGL_ASSERT_VK_EXT(EXT_transform_feedback);
    vkCmdEndTransformFeedbackEXT(commandBuffer_, 0, 0, nullptr, nullptr);
}

/* ----- Drawing ----- */

void VKCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    FlushDescriptorCache();
    vkCmdDraw(commandBuffer_, numVertices, 1, firstVertex, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    FlushDescriptorCache();
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushDescriptorCache();
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    FlushDescriptorCache();
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    FlushDescriptorCache();
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, firstInstance);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    FlushDescriptorCache();
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushDescriptorCache();
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    FlushDescriptorCache();
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void VKCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushDescriptorCache();
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdDrawIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, 1, 0);
}

void VKCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    FlushDescriptorCache();
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    if (maxDrawIndirectCount_ < numCommands)
    {
        /* Encode multiple indirect draw commands if limit is exceeded */
        while (numCommands > 0)
        {
            const std::uint32_t drawCount = (numCommands % maxDrawIndirectCount_);
            vkCmdDrawIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, drawCount, stride);
            numCommands -= drawCount;
            offset += stride;
        }
    }
    else
        vkCmdDrawIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, numCommands, stride);
}

void VKCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushDescriptorCache();
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdDrawIndexedIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, 1, 0);
}

void VKCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    FlushDescriptorCache();
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    if (maxDrawIndirectCount_ < numCommands)
    {
        /* Encode multiple indirect draw commands if limit is exceeded */
        while (numCommands > 0)
        {
            const std::uint32_t drawCount = (numCommands % maxDrawIndirectCount_);
            vkCmdDrawIndexedIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, drawCount, 0);
            numCommands -= drawCount;
            offset += stride;
        }
    }
    else
        vkCmdDrawIndexedIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, numCommands, stride);
}

/* ----- Compute ----- */

void VKCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    FlushDescriptorCache();
    vkCmdDispatch(commandBuffer_, numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void VKCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    FlushDescriptorCache();
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdDispatchIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset);
}

/* ----- Debugging ----- */

void VKCommandBuffer::PushDebugGroup(const char* name)
{
    if (HasExtension(VKExt::EXT_debug_marker))
    {
        VkDebugMarkerMarkerInfoEXT markerInfo;
        {
            markerInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            markerInfo.pNext        = nullptr;
            markerInfo.pMarkerName  = name;
            markerInfo.color[0]     = 0.0f;
            markerInfo.color[1]     = 0.0f;
            markerInfo.color[2]     = 0.0f;
            markerInfo.color[3]     = 0.0f;
        }
        vkCmdDebugMarkerBeginEXT(commandBuffer_, &markerInfo);
    }
}

void VKCommandBuffer::PopDebugGroup()
{
    if (HasExtension(VKExt::EXT_debug_marker))
        vkCmdDebugMarkerEndEXT(commandBuffer_);
}

/* ----- Extensions ----- */

void VKCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool VKCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Vulkan::CommandBufferNativeHandle))
    {
        auto* nativeHandleVK = reinterpret_cast<Vulkan::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleVK->commandBuffer = commandBuffer_;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void VKCommandBuffer::CreateVkCommandPool(std::uint32_t queueFamilyIndex)
{
    /* Create command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
    }
    VkResult result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");
}

void VKCommandBuffer::CreateVkCommandBuffers(VkCommandBufferLevel level)
{
    /* Allocate command buffers */
    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.commandPool           = commandPool_;
        allocInfo.level                 = level;
        allocInfo.commandBufferCount    = numCommandBuffers_;
    }
    VkResult result = vkAllocateCommandBuffers(device_, &allocInfo, commandBufferArray_);
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffers");
}

void VKCommandBuffer::CreateVkRecordingFences()
{
    /* Create all recording fences with their initial state being signaled */
    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    for_range(i, numCommandBuffers_)
    {
        /* Create fence for command buffer recording */
        VkResult result = vkCreateFence(device_, &createInfo, nullptr, recordingFenceArray_[i].ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan fence");
    }
}

void VKCommandBuffer::ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments)
{
    if (numAttachments > 0)
    {
        /* Clear framebuffer attachments at the entire image region */
        VkClearRect clearRect;
        {
            clearRect.rect              = framebufferRenderArea_;
            clearRect.baseArrayLayer    = 0;
            clearRect.layerCount        = 1;
        }
        vkCmdClearAttachments(commandBuffer_, numAttachments, attachments, 1, &clearRect);
    }
}

static void ToVkClearDepthStencil(VkClearDepthStencilValue& dst, float srcDepth, std::uint32_t srcStencil)
{
    dst.depth   = srcDepth;
    dst.stencil = srcStencil;
}

void VKCommandBuffer::ConvertRenderPassClearValues(
    const VKRenderPass& renderPass,
    std::uint32_t&      dstClearValuesCount,
    VkClearValue*       dstClearValues,
    std::uint32_t       srcClearValuesCount,
    const ClearValue*   srcClearValues)
{
    /* Fill array of clear values */
    dstClearValuesCount     = renderPass.GetNumClearValues();

    std::uint64_t   clearValuesMask     = renderPass.GetClearValuesMask();
    std::uint8_t    depthStencilIndex   = renderPass.GetDepthStencilIndex();
    const bool      hasMultiSampling    = (renderPass.GetSampleCountBits() > VK_SAMPLE_COUNT_1_BIT);

    const VkClearColorValue         defaultClearColor           = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    const VkClearDepthStencilValue  defaultClearDepthStencil    = { 1.0f, 0 };

    std::uint32_t srcIndex = 0;

    for_range(i, dstClearValuesCount)
    {
        /* Check if current attachment index requires a clear value */
        if (((clearValuesMask >> i) & 0x1ull) != 0)
        {
            /* Select destination Vulkan clear value */
            auto& dst = dstClearValues[i];

            if (srcIndex < srcClearValuesCount)
            {
                /* Set specified clear parameter */
                const auto& src = srcClearValues[srcIndex++];
                if (i == depthStencilIndex)
                    ToVkClearDepthStencil(dst.depthStencil, src.depth, src.stencil);
                else
                    ToVkClearColor(dst.color, src.color);
            }
            else
            {
                /* Set default clear parameters */
                if (i == depthStencilIndex)
                    dst.depthStencil = defaultClearDepthStencil;
                else
                    dst.color = defaultClearColor;
            }
        }
    }

    if (hasMultiSampling)
        dstClearValuesCount += renderPass.GetNumColorAttachments();
}

void VKCommandBuffer::PauseRenderPass()
{
    vkCmdEndRenderPass(commandBuffer_);
}

void VKCommandBuffer::ResumeRenderPass()
{
    /* Record begin of render pass */
    VkRenderPassBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.renderPass        = secondaryRenderPass_;
        beginInfo.framebuffer       = framebuffer_;
        beginInfo.renderArea        = framebufferRenderArea_;
        beginInfo.clearValueCount   = 0;
        beginInfo.pClearValues      = nullptr;
    }
    vkCmdBeginRenderPass(commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

bool VKCommandBuffer::IsInsideRenderPass() const
{
    return (recordState_ == RecordState::InsideRenderPass);
}

void VKCommandBuffer::BufferPipelineBarrier(
    VkBuffer                buffer,
    VkDeviceSize            offset,
    VkDeviceSize            size,
    VkAccessFlags           srcAccessMask,
    VkAccessFlags           dstAccessMask,
    VkPipelineStageFlags    srcStageMask,
    VkPipelineStageFlags    dstStageMask)
{
    VkBufferMemoryBarrier barrier;
    {
        barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext               = nullptr;
        barrier.srcAccessMask       = srcAccessMask;
        barrier.dstAccessMask       = dstAccessMask;
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        barrier.buffer              = buffer;
        barrier.offset              = offset;
        barrier.size                = size;
    }
    vkCmdPipelineBarrier(commandBuffer_, srcStageMask, dstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void VKCommandBuffer::FlushDescriptorCache()
{
    if (descriptorCache_ != nullptr && descriptorCache_->IsInvalidated())
    {
        VkDescriptorSet descriptorSet = descriptorCache_->FlushDescriptorSet(*descriptorSetPool_, descriptorSetWriter_);
        boundPipelineState_->BindDynamicDescriptorSet(commandBuffer_, descriptorSet);
    }
}

void VKCommandBuffer::AcquireNextBuffer()
{
    commandBufferIndex_ = (commandBufferIndex_ + 1) % numCommandBuffers_;
    commandBuffer_      = commandBufferArray_[commandBufferIndex_];
    recordingFence_     = recordingFenceArray_[commandBufferIndex_].Get();
    descriptorSetPool_  = &(descriptorSetPoolArray_[commandBufferIndex_]);
    descriptorSetPool_->Reset();
}

void VKCommandBuffer::ResetBindingStates()
{
    boundSwapChain_         = nullptr;
    boundPipelineLayout_    = nullptr;
    boundPipelineState_     = nullptr;
    descriptorCache_        = nullptr;
}

void VKCommandBuffer::ResetQueryPoolsInFlight()
{
    for_range(i, numQueryHeapsInFlight_)
    {
        auto& queryHeapVK = *(queryHeapsInFlight_[0]);
        vkCmdResetQueryPool(
            commandBuffer_,
            queryHeapVK.GetVkQueryPool(),
            0,
            queryHeapVK.GetNumQueries()
        );
    }
    numQueryHeapsInFlight_ = 0;
}

void VKCommandBuffer::AppendQueryPoolInFlight(VKQueryHeap* queryHeap)
{
    if (numQueryHeapsInFlight_ >= queryHeapsInFlight_.size())
        queryHeapsInFlight_.push_back(queryHeap);
    else
        queryHeapsInFlight_[numQueryHeapsInFlight_] = queryHeap;
    ++numQueryHeapsInFlight_;
}

std::uint32_t VKCommandBuffer::GetNumVkCommandBuffers(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & CommandBufferFlags::MultiSubmit) != 0)
        return 1u;
    else
        return std::max(1u, std::min(desc.numNativeBuffers, VKCommandBuffer::maxNumCommandBuffers));
}


} // /namespace LLGL



// ================================================================================
