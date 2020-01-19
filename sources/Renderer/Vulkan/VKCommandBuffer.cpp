/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKPhysicalDevice.h"
#include "VKRenderContext.h"
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
#include <LLGL/StaticLimits.h>
#include <cstddef>


namespace LLGL
{


static const std::uint32_t g_maxNumViewportsPerBatch = 16;

// Returns the maximum for a indirect multi draw command
static std::uint32_t GetMaxDrawIndirectCount(const VKPhysicalDevice& physicalDevice)
{
    if (physicalDevice.GetFeatures().multiDrawIndirect != VK_FALSE)
        return physicalDevice.GetProperties().limits.maxDrawIndirectCount;
    else
        return 1u;
}

// Returns the number of native command buffers for the specified descriptor
static std::uint32_t GetNumVkCommandBuffers(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & CommandBufferFlags::MultiSubmit) != 0)
        return 1u;
    else
        return std::max(1u, desc.numNativeBuffers);
}

VKCommandBuffer::VKCommandBuffer(
    const VKPhysicalDevice&         physicalDevice,
    VKDevice&                       device,
    VkQueue                         graphicsQueue,
    const QueueFamilyIndices&       queueFamilyIndices,
    const CommandBufferDescriptor&  desc)
:
    device_               { device                                  },
    commandPool_          { device, vkDestroyCommandPool            },
    queuePresentFamily_   { queueFamilyIndices.presentFamily        },
    maxDrawIndirectCount_ { GetMaxDrawIndirectCount(physicalDevice) }
{
    /* Translate creation flags */
    if ((desc.flags & CommandBufferFlags::DeferredSubmit) != 0)
    {
        usageFlags_     = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        bufferLevel_    = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    }
    else if ((desc.flags & CommandBufferFlags::MultiSubmit) != 0)
        usageFlags_ = 0;

    /* Determine number of internal command buffers */
    const auto bufferCount = GetNumVkCommandBuffers(desc);

    /* Create native command buffer objects */
    CreateCommandPool(queueFamilyIndices.graphicsFamily);
    CreateCommandBuffers(bufferCount);
    CreateRecordingFences(graphicsQueue, bufferCount);

    /* Acquire first native command buffer */
    AcquireNextBuffer();
}

VKCommandBuffer::~VKCommandBuffer()
{
    vkFreeCommandBuffers(
        device_,
        commandPool_,
        static_cast<std::uint32_t>(commandBufferList_.size()),
        commandBufferList_.data()
    );
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
    auto result = vkBeginCommandBuffer(commandBuffer_, &beginInfo);
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
    auto result = vkEndCommandBuffer(commandBuffer_);
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");

    /* Store new record state */
    recordState_ = RecordState::ReadyForSubmit;
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
        ResumeRenderPass();
    }
    else
        vkCmdUpdateBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, data);
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

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        device_.CopyImageToBuffer(commandBuffer_, srcTextureVK, dstBufferVK, region);
        ResumeRenderPass();
    }
    else
        device_.CopyImageToBuffer(commandBuffer_, srcTextureVK, dstBufferVK, region);
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
    if (fillSize == Constants::wholeSize)
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

    if (IsInsideRenderPass())
    {
        PauseRenderPass();
        device_.CopyBufferToImage(commandBuffer_, srcBufferVK, dstTextureVK, region);
        ResumeRenderPass();
    }
    else
        device_.CopyBufferToImage(commandBuffer_, srcBufferVK, dstTextureVK, region);
}

void VKCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);
    device_.GenerateMips(
        commandBuffer_,
        textureVK.GetVkImage(),
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
            textureVK.GetVkExtent(),
            subresource
        );
    }
}

/* ----- Viewport and Scissor ----- */

#if 0//TODO: currently unused
// Check if VkViewport and Viewport structures can be safely reinterpret-casted
static constexpr bool IsCompatibleToVkViewport()
{
    return
    (
        sizeof(VkViewport)             == sizeof(Viewport)             &&
        offsetof(VkViewport, x       ) == offsetof(Viewport, x       ) &&
        offsetof(VkViewport, y       ) == offsetof(Viewport, y       ) &&
        offsetof(VkViewport, width   ) == offsetof(Viewport, width   ) &&
        offsetof(VkViewport, height  ) == offsetof(Viewport, height  ) &&
        offsetof(VkViewport, minDepth) == offsetof(Viewport, minDepth) &&
        offsetof(VkViewport, maxDepth) == offsetof(Viewport, maxDepth)
    );
}
#endif

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    #if 0
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport type */
        vkCmdSetViewport(commandBuffer_, 0, 1, reinterpret_cast<const VkViewport*>(&viewport));
    }
    else
    #endif
    {
        /* Convert viewport to VkViewport type */
        VkViewport viewportVK;
        VKTypes::Convert(viewportVK, viewport);
        vkCmdSetViewport(commandBuffer_, 0, 1, &viewportVK);
    }
}

void VKCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    #if 0
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport types */
        vkCmdSetViewport(commandBuffer_, 0, numViewports, reinterpret_cast<const VkViewport*>(viewports));
    }
    else
    #endif
    {
        VkViewport viewportsVK[g_maxNumViewportsPerBatch];

        for (std::uint32_t i = 0, first = 0, count = 0; i < numViewports; numViewports -= count)
        {
            /* Convert viewport to VkViewport types */
            count = std::min(numViewports, g_maxNumViewportsPerBatch);

            for (first = i; i < first + count; ++i)
                VKTypes::Convert(viewportsVK[i - first], viewports[i]);

            vkCmdSetViewport(commandBuffer_, first, count, viewportsVK);
        }
    }
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    if (scissorEnabled_)
    {
        VkRect2D scissorVK;
        VKTypes::Convert(scissorVK, scissor);
        vkCmdSetScissor(commandBuffer_, 0, 1, &scissorVK);
    }
}

void VKCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (scissorEnabled_)
    {
        VkRect2D scissorsVK[g_maxNumViewportsPerBatch];

        for (std::uint32_t i = 0, first = 0, count = 0; i < numScissors; numScissors -= count)
        {
            /* Convert viewport to VkViewport types */
            count = std::min(numScissors, g_maxNumViewportsPerBatch);

            for (first = i; i < first + count; ++i)
                VKTypes::Convert(scissorsVK[i - first], scissors[i]);

            vkCmdSetScissor(commandBuffer_, first, count, scissorsVK);
        }
    }
}

/* ----- Clear ----- */

static void ToVkClearColor(VkClearColorValue& dst, const ColorRGBAf& src)
{
    dst.float32[0] = src.r;
    dst.float32[1] = src.g;
    dst.float32[2] = src.b;
    dst.float32[3] = src.a;
}

void VKCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    ToVkClearColor(clearColor_, color);
}

void VKCommandBuffer::SetClearDepth(float depth)
{
    clearDepthStencil_.depth = depth;
}

void VKCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearDepthStencil_.stencil = stencil;
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

void VKCommandBuffer::Clear(long flags)
{
    VkClearAttachment attachments[LLGL_MAX_NUM_ATTACHMENTS];

    std::uint32_t numAttachments = 0;

    /* Fill clear descriptors for color attachments */
    if ((flags & ClearFlags::Color) != 0)
    {
        numAttachments = std::min(numColorAttachments_, LLGL_MAX_NUM_COLOR_ATTACHMENTS);
        for (std::uint32_t i = 0; i < numAttachments; ++i)
        {
            auto& attachment = attachments[i];
            {
                attachment.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
                attachment.colorAttachment  = i;
                attachment.clearValue.color = clearColor_;
            }
        }
    }

    /* Fill clear descriptor for depth-stencil attachment */
    if ((flags & ClearFlags::DepthStencil) != 0 && hasDSVAttachment_)
    {
        auto& attachment = attachments[numAttachments++];
        {
            attachment.aspectMask               = GetDepthStencilAspectMask(flags);
            attachment.colorAttachment          = 0; // ignored
            attachment.clearValue.depthStencil  = clearDepthStencil_;
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

    for (std::uint32_t i = 0, n = std::min(numAttachments, LLGL_MAX_NUM_ATTACHMENTS); i < n; ++i)
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
        else if (hasDSVAttachment_)
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

//private
void VKCommandBuffer::BindResourceHeap(VKResourceHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t firstSet)
{
    const VkDescriptorSet descriptorSets[1] = { resourceHeapVK.GetVkDescriptorSets()[firstSet] };
    vkCmdBindDescriptorSets(
        commandBuffer_,
        bindingPoint,
        resourceHeapVK.GetVkPipelineLayout(),   // Pipeline lauyout
        0,                                      // First set in SPIR-V (always 0 atm.)
        1,                                      // Number of descriptor sets (always 1 atm.)
        descriptorSets,                         // Descriptor sets
        0,                                      // No dynamic offsets
        nullptr
    );
}

void VKCommandBuffer::SetResourceHeap(
    ResourceHeap&           resourceHeap,
    std::uint32_t           firstSet,
    const PipelineBindPoint bindPoint)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceHeap&, resourceHeap);

    /* Bind resource heap to pipelines */
    if (bindPoint == PipelineBindPoint::Undefined)
    {
        if (resourceHeapVK.GetBindPoint() == VK_PIPELINE_BIND_POINT_MAX_ENUM)
        {
            BindResourceHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_GRAPHICS, firstSet);
            BindResourceHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_COMPUTE, firstSet);
        }
        else
            BindResourceHeap(resourceHeapVK, resourceHeapVK.GetBindPoint(), firstSet);
    }
    else
        BindResourceHeap(resourceHeapVK, VKTypes::Map(bindPoint), firstSet);

    /* Insert resource barrier into command buffer */
    resourceHeapVK.InsertPipelineBarrier(commandBuffer_);
}

void VKCommandBuffer::SetResource(
    Resource&       /*resource*/,
    std::uint32_t   /*slot*/,
    long            /*bindFlags*/,
    long            /*stageFlags*/)
{
    // dummy
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
    const ClearValue*   clearValues)
{
    if (renderTarget.IsRenderContext())
    {
        /* Get Vulkan render context object */
        auto& renderContextVK = LLGL_CAST(VKRenderContext&, renderTarget);

        /* Store information about framebuffer attachments */
        renderPass_             = renderContextVK.GetSwapChainRenderPass().GetVkRenderPass();
        secondaryRenderPass_    = renderContextVK.GetSecondaryVkRenderPass();
        framebuffer_            = renderContextVK.GetVkFramebuffer();
        framebufferExtent_      = renderContextVK.GetVkExtent();
        numColorAttachments_    = renderContextVK.GetNumColorAttachments();
        hasDSVAttachment_       = (renderContextVK.HasDepthAttachment() || renderContextVK.HasStencilAttachment());
    }
    else
    {
        /* Get Vulkan render target object and store its extent for subsequent commands */
        auto& renderTargetVK = LLGL_CAST(VKRenderTarget&, renderTarget);

        /* Store information about framebuffer attachments */
        renderPass_             = renderTargetVK.GetVkRenderPass();
        secondaryRenderPass_    = renderTargetVK.GetSecondaryVkRenderPass();
        framebuffer_            = renderTargetVK.GetVkFramebuffer();
        framebufferExtent_      = renderTargetVK.GetVkExtent();
        numColorAttachments_    = renderTargetVK.GetNumColorAttachments();
        hasDSVAttachment_       = (renderTargetVK.HasDepthAttachment() || renderTargetVK.HasStencilAttachment());
    }

    scissorRectInvalidated_ = true;

    /* Declare array or clear values */
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
        beginInfo.renderArea.offset = { 0, 0 };
        beginInfo.renderArea.extent = framebufferExtent_;
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


/* ----- Pipeline States ----- */

void VKCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    /* Bind native PSO */
    auto& pipelineStateVK = LLGL_CAST(VKPipelineState&, pipelineState);
    vkCmdBindPipeline(commandBuffer_, pipelineStateVK.GetBindPoint(), pipelineStateVK.GetVkPipeline());

    /* Handle special case for graphics PSOs */
    if (pipelineStateVK.GetBindPoint() == VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        auto& graphicsPSO = LLGL_CAST(VKGraphicsPSO&, pipelineStateVK);

        /* Scissor rectangle must be updated (if scissor test is disabled) */
        scissorEnabled_ = graphicsPSO.IsScissorEnabled();
        if (!scissorEnabled_ && scissorRectInvalidated_ && graphicsPSO.HasDynamicScissor())
        {
            /* Set scissor to render target resolution */
            VkRect2D scissorRect;
            {
                scissorRect.offset = { 0, 0 };
                scissorRect.extent = framebufferExtent_;
            }
            vkCmdSetScissor(commandBuffer_, 0, 1, &scissorRect);

            /* Avoid scissor update with each graphics pipeline binding (as long as render pass does not change) */
            scissorRectInvalidated_ = false;
        }
    }
}

void VKCommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    vkCmdSetBlendConstants(commandBuffer_, color.Ptr());
}

void VKCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    vkCmdSetStencilReference(commandBuffer_, VKTypes::Map(stencilFace), reference);
}

void VKCommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    VKCommandBuffer::SetUniforms(location, 1, data, dataSize);
}

void VKCommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    //TODO
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
    LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_conditional_rendering, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

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
    LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_conditional_rendering, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

    /* End conditional rendering block */
    vkCmdEndConditionalRenderingEXT(commandBuffer_);
}

/* ----- Stream Output ------ */

#if 0
void VKCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_transform_feedback, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    VkBuffer buffers[] = { bufferVK.GetVkBuffer() };
    VkDeviceSize offsets[] = { 0 };
    VkDeviceSize sizes[] = { bufferVK.GetSize() };

    vkCmdBindTransformFeedbackBuffersEXT(commandBuffer_, 0, 1, buffers, offsets, sizes);
}
#endif

void VKCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_transform_feedback, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    //TODO: bind buffers
    vkCmdBeginTransformFeedbackEXT(commandBuffer_, 0, 0, nullptr, nullptr);
}

void VKCommandBuffer::EndStreamOutput()
{
    LLGL_ASSERT_VK_EXTENSION(VKExt::EXT_transform_feedback, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    vkCmdEndTransformFeedbackEXT(commandBuffer_, 0, 0, nullptr, nullptr);
}

/* ----- Drawing ----- */

void VKCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    vkCmdDraw(commandBuffer_, numVertices, 1, firstVertex, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, firstInstance);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void VKCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdDrawIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, 1, 0);
}

void VKCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    if (maxDrawIndirectCount_ < numCommands)
    {
        while (numCommands > 0)
        {
            std::uint32_t drawCount = (numCommands % maxDrawIndirectCount_);
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
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdDrawIndexedIndirect(commandBuffer_, bufferVK.GetVkBuffer(), offset, 1, 0);
}

void VKCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    if (maxDrawIndirectCount_ < numCommands)
    {
        while (numCommands > 0)
        {
            std::uint32_t drawCount = (numCommands % maxDrawIndirectCount_);
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
    vkCmdDispatch(commandBuffer_, numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void VKCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
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

void VKCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}


/*
 * ======= Private: =======
 */

void VKCommandBuffer::CreateCommandPool(std::uint32_t queueFamilyIndex)
{
    /* Create command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
    }
    auto result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");
}

void VKCommandBuffer::CreateCommandBuffers(std::uint32_t bufferCount)
{
    /* Allocate command buffers */
    commandBufferList_.resize(bufferCount);

    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.commandPool           = commandPool_;
        allocInfo.level                 = bufferLevel_;
        allocInfo.commandBufferCount    = bufferCount;
    }
    auto result = vkAllocateCommandBuffers(device_, &allocInfo, commandBufferList_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffers");
}

void VKCommandBuffer::CreateRecordingFences(VkQueue graphicsQueue, std::uint32_t numFences)
{
    recordingFenceList_.reserve(numFences);

    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
    }

    for (std::uint32_t i = 0; i < numFences; ++i)
    {
        VKPtr<VkFence> fence{ device_, vkDestroyFence };
        {
            /* Create fence for command buffer recording */
            auto result = vkCreateFence(device_, &createInfo, nullptr, fence.ReleaseAndGetAddressOf());
            VKThrowIfFailed(result, "failed to create Vulkan fence");

            /* Initial fence signal */
            vkQueueSubmit(graphicsQueue, 0, nullptr, fence);
        }
        recordingFenceList_.emplace_back(std::move(fence));
    }
}

void VKCommandBuffer::ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments)
{
    if (numAttachments > 0)
    {
        /* Clear framebuffer attachments at the entire image region */
        VkClearRect clearRect;
        {
            clearRect.rect.offset.x     = 0;
            clearRect.rect.offset.y     = 0;
            clearRect.rect.extent       = framebufferExtent_;
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
    auto clearValuesMask    = renderPass.GetClearValuesMask();
    auto depthStencilIndex  = renderPass.GetDepthStencilIndex();
    bool multiSampleEnabled = (renderPass.GetSampleCountBits() > VK_SAMPLE_COUNT_1_BIT);

    std::uint32_t dstIndex = 0, srcIndex = 0;

    for (std::uint32_t i = 0; i < dstClearValuesCount; ++i)
    {
        /* Check if current attachment index requires a clear value */
        if (((clearValuesMask >> i) & 0x1ull) != 0)
        {
            /* Select destination Vulkan clear value */
            if (i == depthStencilIndex || !multiSampleEnabled)
                dstIndex = i;
            else
                dstIndex = (renderPass.GetNumColorAttachments() + 1u + i);

            auto& dst = dstClearValues[dstIndex];

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
                /* Set global clear parameters (from SetClearColor, SetClearDepth, SetClearStencil) */
                if (i == depthStencilIndex)
                    dst.depthStencil = clearDepthStencil_;
                else
                    dst.color = clearColor_;
            }
        }
    }

    if (multiSampleEnabled)
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
        beginInfo.renderArea.offset = { 0, 0 };
        beginInfo.renderArea.extent = framebufferExtent_;
        beginInfo.clearValueCount   = 0;
        beginInfo.pClearValues      = nullptr;
    }
    vkCmdBeginRenderPass(commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

bool VKCommandBuffer::IsInsideRenderPass() const
{
    return (recordState_ == RecordState::InsideRenderPass);
}

void VKCommandBuffer::AcquireNextBuffer()
{
    commandBufferIndex_ = (commandBufferIndex_ + 1) % commandBufferList_.size();
    commandBuffer_      = commandBufferList_[commandBufferIndex_];
    recordingFence_     = recordingFenceList_[commandBufferIndex_].Get();
}

void VKCommandBuffer::ResetQueryPoolsInFlight()
{
    for (std::size_t i = 0; i < numQueryHeapsInFlight_; ++i)
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


} // /namespace LLGL



// ================================================================================
