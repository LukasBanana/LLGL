/*
 * VKRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Platform/Platform.h>
#include "VKRenderSystem.h"
#include "Ext/VKExtensionLoader.h"
#include "Ext/VKExtensions.h"
#include "Ext/VKExtensionRegistry.h"
#include "LLGL/Format.h"
#include "Memory/VKDeviceMemory.h"
#include "../RenderSystemUtils.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Vendor.h"
#include "../../Core/ImageUtils.h"
#include "VKCore.h"
#include "VKTypes.h"
#include "VKInitializers.h"
#include "Command/VKCommandBufferRegistry.h"
#include "Texture/VKImageUtils.h"
#include "RenderState/VKPredicateQueryHeap.h"
#include "RenderState/VKComputePSO.h"
#include "RenderState/VKPipelineLayoutPermutationPool.h"
#include "Shader/VKShaderModulePool.h"
#include "../../Platform/Debug.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Log.h>
#include <cstring>
#include <limits>

#include <LLGL/Backend/Vulkan/NativeHandle.h>


namespace LLGL
{


static bool IsDebugLayerEnabled(long flags)
{
    return ((flags & RenderSystemFlags::DebugDevice) != 0);
}

static bool IsDebugBreakOnErrorEnabled(long flags)
{
    constexpr long requiredFlags = (RenderSystemFlags::DebugDevice | RenderSystemFlags::DebugBreakOnError);
    return ((flags & requiredFlags) == requiredFlags);
}

VKRenderSystem::VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    instance_              { vkDestroyInstance                                        },
    isDebugLayerEnabled_   { LLGL::IsDebugLayerEnabled(renderSystemDesc.flags)        },
    isBreakOnErrorEnabled_ { LLGL::IsDebugBreakOnErrorEnabled(renderSystemDesc.flags) }
{
    /* Extract optional renderer configuration */
    auto* rendererConfigVK = GetRendererConfiguration<RendererConfigurationVulkan>(renderSystemDesc);

    constexpr long preferredDeviceMask = (RenderSystemFlags::PreferNVIDIA | RenderSystemFlags::PreferAMD | RenderSystemFlags::PreferIntel);
    const long preferredDeviceFlags = (renderSystemDesc.flags & preferredDeviceMask);

    QuerySupportedInstanceExtensions();

    if (auto* customNativeHandle = GetRendererNativeHandle<Vulkan::RenderSystemNativeHandle>(renderSystemDesc))
    {
        /* Store weak references to native handles */
        instance_ = VKPtr<VkInstance>{ customNativeHandle->instance };
        if (isDebugLayerEnabled_)
        {
            // The instance was created externally (e.g. by the OpenXR graphics binding); we can only
            // install the debug messenger if that creator enabled VK_EXT_debug_utils. Probe via the
            // procaddr instead of throwing, so a debug-less external instance degrades to "no callback"
            // rather than aborting startup.
            if (vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT") != nullptr)
                CreateDebugMessenger();
            else
                Log::Errorf(
                    "RenderSystemFlags::DebugDevice was requested but the externally-supplied Vulkan instance "
                    "lacks VK_EXT_debug_utils; no Vulkan debug messenger will be installed.\n"
                );
        }
        VKLoadInstanceExtensions(instance_, supportedInstanceExtensions_);
        if (!PickPhysicalDevice(preferredDeviceFlags, customNativeHandle->physicalDevice))
            return;
        CreateLogicalDevice(customNativeHandle->device);
    }
    else
    {
        /* Create Vulkan instance and device objects */
        CreateInstance(rendererConfigVK);
        if (isDebugLayerEnabled_)
            CreateDebugMessenger();
        VKLoadInstanceExtensions(instance_, supportedInstanceExtensions_);
        if (!PickPhysicalDevice(preferredDeviceFlags))
            return;
        CreateLogicalDevice();
    }

    /* Create default resources */
    VKPipelineLayout::CreateDefault(device_);

    /* Create device memory manager */
    deviceMemoryMngr_ = MakeUnique<VKDeviceMemoryManager>(
        device_,
        physicalDevice_.GetMemoryProperties(),
        (rendererConfigVK != nullptr ? rendererConfigVK->minDeviceMemoryAllocationSize : 1024*1024),
        (rendererConfigVK != nullptr ? rendererConfigVK->reduceDeviceMemoryFragmentation : false)
    );
}

VKRenderSystem::~VKRenderSystem()
{
    device_.WaitIdle();
    VKShaderModulePool::Get().Clear();
    VKPipelineLayoutPermutationPool::Get().Clear();
    VKPipelineLayout::ReleaseDefault();
}

/* ----- Swap-chain ----- */

SwapChain* VKRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<VKSwapChain>(
        instance_, physicalDevice_, device_, *deviceMemoryMngr_, device_.GetGraphicsQueue(), swapChainDesc, surface, GetRendererInfo()
    );
}

void VKRenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* VKRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* VKRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    return commandBuffers_.emplace<VKCommandBuffer>(
        physicalDevice_, device_, device_.GetGraphicsQueue(), *deviceMemoryMngr_, device_.GetQueueFamilyIndices(), commandBufferDesc
    );
}

void VKRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

static VkBufferUsageFlags GetStagingVkBufferUsageFlags(long /*cpuAccessFlags*/)
{
    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
}

Buffer* VKRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, static_cast<uint64_t>(std::numeric_limits<VkDeviceSize>::max()));

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    BuildVkBufferCreateInfo(
        stagingCreateInfo,
        static_cast<VkDeviceSize>(bufferDesc.size),
        GetStagingVkBufferUsageFlags(bufferDesc.cpuAccessFlags)
    );

    VKDeviceBuffer stagingBuffer = CreateStagingBufferAndInitialize(stagingCreateInfo, initialData, bufferDesc.size);

    /* Create primary buffer object */
    VKBuffer* bufferVK = buffers_.emplace<VKBuffer>(device_, bufferDesc);

    /* Allocate device memory */
    VKDeviceMemoryRegion* memoryRegion = deviceMemoryMngr_->Allocate(
        bufferVK->GetDeviceBuffer().GetRequirements(),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    bufferVK->BindMemoryRegion(device_, memoryRegion);

    /* Copy staging buffer into hardware buffer */
    device_.CopyBuffer(stagingBuffer.GetVkBuffer(), bufferVK->GetVkBuffer(), static_cast<VkDeviceSize>(bufferDesc.size));

    if (bufferDesc.cpuAccessFlags != 0 || (bufferDesc.miscFlags & MiscFlags::DynamicUsage) != 0)
    {
        /* Store ownership of staging buffer */
        bufferVK->TakeStagingBuffer(std::move(stagingBuffer));
    }
    else
    {
        /* Release staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }

    return bufferVK;
}

BufferArray* VKRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<VKBufferArray>(numBuffers, bufferArray);
}

void VKRenderSystem::Release(Buffer& buffer)
{
    /* Release device memory regions for primary buffer and internal staging buffer, then release buffer object */
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    bufferVK.GetDeviceBuffer().ReleaseMemoryRegion(*deviceMemoryMngr_);
    bufferVK.GetStagingDeviceBuffer().ReleaseMemoryRegion(*deviceMemoryMngr_);
    buffers_.erase(&buffer);
}

void VKRenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void VKRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    if (bufferVK.GetStagingVkBuffer() != VK_NULL_HANDLE)
    {
        /* Copy input data to staging buffer memory */
        device_.WriteBuffer(bufferVK.GetStagingDeviceBuffer(), data, dataSize, offset);

        /* Copy staging buffer into hardware buffer */
        device_.CopyBuffer(bufferVK.GetStagingVkBuffer(), bufferVK.GetVkBuffer(), dataSize, offset, offset);
    }
    else
    {
        /* Create staging buffer */
        VkBufferCreateInfo stagingCreateInfo;
        BuildVkBufferCreateInfo(
            stagingCreateInfo,
            dataSize,
            (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        );

        VKDeviceBuffer stagingBuffer = CreateStagingBufferAndInitialize(stagingCreateInfo, data, dataSize);

        /* Copy staging buffer into hardware buffer */
        device_.CopyBuffer(stagingBuffer.GetVkBuffer(), bufferVK.GetVkBuffer(), dataSize, 0, offset);

        /* Release device memory region of staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }
}

void VKRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    if (bufferVK.GetStagingVkBuffer() != VK_NULL_HANDLE)
    {
        /* Copy hardware buffer into staging buffer */
        device_.CopyBuffer(bufferVK.GetVkBuffer(), bufferVK.GetStagingVkBuffer(), dataSize, offset, offset);

        /* Copy staging buffer memory to output data */
        device_.ReadBuffer(bufferVK.GetStagingDeviceBuffer(), data, dataSize, offset);
    }
    else
    {
        /* Create staging buffer */
        VkBufferCreateInfo stagingCreateInfo;
        BuildVkBufferCreateInfo(
            stagingCreateInfo,
            dataSize,
            (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        );

        VKDeviceBuffer stagingBuffer = CreateStagingBuffer(stagingCreateInfo);

        /* Copy hardware buffer into staging buffer */
        device_.CopyBuffer(bufferVK.GetVkBuffer(), stagingBuffer.GetVkBuffer(), dataSize, offset, 0);

        /* Copy staging buffer memory to output data */
        device_.ReadBuffer(stagingBuffer, data, dataSize, 0);

        /* Release device memory region of staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }
}

void* VKRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    return bufferVK.Map(device_, access, 0, bufferVK.GetSize());
}

void* VKRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    return bufferVK.Map(device_, access, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(length));
}

void VKRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    bufferVK.Unmap(device_);
}

/* ----- Textures ----- */

// Tries to find an optimal initial VkImageLayout for the specified texture format and binding flags
static VkImageLayout FindOptimalInitialVkImageLayout(Format format, long bindFlags)
{
    if ((bindFlags & BindFlags::CopyDst) != 0)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    if ((bindFlags & BindFlags::CopySrc) != 0)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    if ((bindFlags & BindFlags::ColorAttachment) != 0)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if ((bindFlags & BindFlags::DepthStencilAttachment) != 0)
    {
        if (IsStencilFormat(format))
            return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        else
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    }
    if ((bindFlags & BindFlags::Sampled) != 0)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

Texture* VKRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    /* Determine size of image for staging buffer */
    const std::uint32_t imageSize       = NumMipTexels(textureDesc, 0);
    const std::size_t   initialDataSize = GetMemoryFootprint(textureDesc.format, imageSize);
    const std::uint32_t bytesPerPixel   = static_cast<std::uint32_t>(GetMemoryFootprint(textureDesc.format, 1));
    const auto&         formatAttribs   = GetFormatAttribs(textureDesc.format);
    const Extent3D      extent          = CalcTextureExtent(textureDesc.type, textureDesc.extent, textureDesc.arrayLayers);

    const bool isCompressed = ((formatAttribs.flags & FormatFlags::IsCompressed) != 0);

    /* Set up initial image data */
    const void* initialData = nullptr;
    DynamicByteArray intermediateData;

    std::uint32_t srcRowStride      = textureDesc.extent.width * bytesPerPixel;
    std::uint32_t srcLayerStride    = textureDesc.extent.height * srcRowStride;

    if (initialImage != nullptr)
    {
        const ImageView& srcImageView = *initialImage;

        /* Check if image data must be converted */
        if (!isCompressed)
        {
            const std::uint32_t srcBytesPerPixel        = static_cast<std::uint32_t>(GetMemoryFootprint(srcImageView.format, srcImageView.dataType, 1));
            const std::uint32_t srcDefaultRowStride     = (textureDesc.extent.width  * srcBytesPerPixel);
            const std::uint32_t srcDefaultLayerStride   = (textureDesc.extent.height * srcDefaultRowStride);

            srcRowStride    = std::max<std::uint32_t>(srcImageView.rowStride, srcDefaultRowStride);
            srcLayerStride  = std::max<std::uint32_t>(srcImageView.layerStride, srcDefaultLayerStride);

            /* Check if amount of padding memory is small enough to justify a larger GPU buffer upload */
            bool needsStrideConversion =
            (
                (srcImageView.rowStride   != 0 && srcImageView.rowStride   != srcDefaultRowStride) ||
                (srcImageView.layerStride != 0 && srcImageView.layerStride != srcDefaultLayerStride)
            );
            if (needsStrideConversion)
            {
                const std::size_t dataSizeWithPadding = srcImageView.dataSize / srcBytesPerPixel * bytesPerPixel;
                const bool isPaddingLessThan50Percent = (dataSizeWithPadding < initialDataSize + initialDataSize/2);

                const bool isStridePixelSizeAligned =
                (
                    srcImageView.rowStride > 0 &&
                    srcImageView.rowStride % bytesPerPixel == 0 &&
                    srcImageView.layerStride % srcImageView.rowStride == 0
                );

                if (isStridePixelSizeAligned && isPaddingLessThan50Percent)
                    needsStrideConversion = false;
            }

            if (srcImageView.format   != formatAttribs.format   ||
                srcImageView.dataType != formatAttribs.dataType ||
                needsStrideConversion)
            {
                /* Convert image format (will be null if no conversion is necessary) */
                intermediateData = ConvertImageBuffer(srcImageView, formatAttribs.format, formatAttribs.dataType, extent, LLGL_MAX_THREAD_COUNT);

                srcRowStride    = textureDesc.extent.width * bytesPerPixel;
                srcLayerStride  = textureDesc.extent.height * srcRowStride;
            }
        }

        if (intermediateData)
        {
            /* Validate that source image data was large enough so conversion is valid, then use temporary image as source for initial data */
            const std::size_t srcImageDataSize = GetMemoryFootprint(initialImage->format, initialImage->dataType, imageSize);
            LLGL_ASSERT(initialImage->dataSize >= srcImageDataSize);
            initialData = intermediateData.get();
        }
        else
        {
            /* Validate that image data is large enough, then use input data as source for initial data */
            LLGL_ASSERT(initialImage->dataSize >= initialDataSize);
            initialData = initialImage->data;
        }
    }
    else if ((textureDesc.miscFlags & MiscFlags::NoInitialData) == 0)
    {
        /* Allocate default image data */
        const auto& formatAttribs = GetFormatAttribs(textureDesc.format);
        if (formatAttribs.bitSize > 0 && (formatAttribs.flags & FormatFlags::IsCompressed) == 0)
            intermediateData = GenerateImageBuffer(formatAttribs.format, formatAttribs.dataType, imageSize, textureDesc.clearValue.color);
        else
            intermediateData = DynamicByteArray{ initialDataSize, UninitializeTag{} };

        initialData = intermediateData.get();
    }

    /* Create device texture */
    VKTexture* textureVK = textures_.emplace<VKTexture>(device_, *deviceMemoryMngr_, textureDesc);

    if (initialData != nullptr && !IsMultiSampleTexture(textureDesc.type))
    {
        /* Create staging buffer */
        VkBufferCreateInfo stagingCreateInfo;
        BuildVkBufferCreateInfo(
            stagingCreateInfo,
            initialDataSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );

        VKDeviceBuffer stagingBuffer =
        (
            isCompressed
                ? CreateStagingBufferAndInitialize(stagingCreateInfo, initialData, initialDataSize)
                : CreateTextureStagingBufferAndInitialize(stagingCreateInfo, initialData, initialDataSize, extent, srcRowStride, bytesPerPixel)
        );

        /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
        VkCommandBuffer cmdBuffer = AllocCommandBuffer();
        {
            const TextureSubresource subresource{ 0, textureVK->GetNumArrayLayers(), 0, textureVK->GetNumMipLevels() };

            textureVK->TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);

            /* Determine row length (in pixels) for image upload with padding */
            const std::uint32_t rowLength   = (bytesPerPixel > 0 ? srcRowStride / bytesPerPixel : 0);
            const std::uint32_t imageHeight = (srcRowStride > 0 ? srcLayerStride / srcRowStride : 0);

            context_.CopyBufferToImage(
                stagingBuffer.GetVkBuffer(),
                textureVK->GetVkImage(),
                textureVK->GetVkFormat(),
                VkOffset3D{ 0, 0, 0 },
                textureVK->GetVkExtent(),
                subresource,
                rowLength,
                imageHeight
            );

            /* Prepare image layout to be in its optimal state initially */
            if ((textureVK->GetUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
                textureVK->TransitionImageLayout(context_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);

            /* Generate MIP-maps if enabled */
            if (initialImage != nullptr && MustGenerateMipsOnCreate(textureDesc))
            {
                context_.GenerateMips(
                    textureVK->GetVkImage(),
                    textureVK->GetVkFormat(),
                    textureVK->GetVkExtent(),
                    subresource
                );
            }
        }
        FlushCommandBuffer(cmdBuffer);

        /* Release staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }
    else
    {
        /* Initialize image layout */
        const VkImageLayout initialLayout = FindOptimalInitialVkImageLayout(textureDesc.format, textureDesc.bindFlags);
        if (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            VkCommandBuffer cmdBuffer = AllocCommandBuffer();
            {
                textureVK->TransitionImageLayout(context_, initialLayout, true);
            }
            FlushCommandBuffer(cmdBuffer);
        }
    }

    /* Create primary image view for texture */
    textureVK->CreateInternalImageView(device_);

    return textureVK;
}

void VKRenderSystem::Release(Texture& texture)
{
    /* Release device memory region, then release texture object */
    auto& textureVK = LLGL_CAST(VKTexture&, texture);
    deviceMemoryMngr_->Release(textureVK.GetMemoryRegion());
    textures_.erase(&texture);
}

void VKRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);

    /* Determine size of image for staging buffer */
    const TextureSubresource&   subresource     = textureRegion.subresource;
  //const Offset3D              offset          = CalcTextureOffset(textureVK.GetType(), textureRegion.offset, subresource.baseArrayLayer);
    const Extent3D              extent          = CalcTextureExtent(textureVK.GetType(), textureRegion.extent, subresource.numArrayLayers);
    const Format                format          = textureVK.GetFormat();

    VkImage                     image           = textureVK.GetVkImage();
    const std::uint32_t         imageSize       = extent.width * extent.height * extent.depth;
    const void*                 imageData       = nullptr;
    const VkDeviceSize          imageDataSize   = static_cast<VkDeviceSize>(GetMemoryFootprint(format, imageSize));
    const std::uint32_t         bytesPerPixel   = static_cast<std::uint32_t>(GetMemoryFootprint(format, 1));

    /* Check if image data must be converted */
    DynamicByteArray intermediateData;

    std::uint32_t srcRowStride = srcImageView.rowStride > 0 ? srcImageView.rowStride : extent.width * bytesPerPixel;

    const auto& formatAttribs = GetFormatAttribs(format);
    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != srcImageView.format || formatAttribs.dataType != srcImageView.dataType))
    {
        /* Convert image format (will be null if no conversion is necessary) */
        intermediateData = ConvertImageBuffer(srcImageView, formatAttribs.format, formatAttribs.dataType, extent, LLGL_MAX_THREAD_COUNT);
        srcRowStride = extent.width * bytesPerPixel;
    }

    if (intermediateData)
    {
        /* Validate that source image data was large enough so conversion is valid, then use temporary buffer as source for initial data */
        const std::size_t srcImageDataSize = GetMemoryFootprint(srcImageView.format, srcImageView.dataType, imageSize);
        LLGL_ASSERT(srcImageView.dataSize >= srcImageDataSize);
        imageData = intermediateData.get();
    }
    else
    {
        /* Validate that image data is large enough, then use input data as source for initial data */
        LLGL_ASSERT(srcImageView.dataSize >= static_cast<std::size_t>(imageDataSize));
        imageData = srcImageView.data;
    }

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    BuildVkBufferCreateInfo(
        stagingCreateInfo,
        imageDataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT // <-- TODO: support read/write mapping //GetStagingVkBufferUsageFlags(bufferDesc.cpuAccessFlags)
    );

    VKDeviceBuffer stagingBuffer =
    (
        IsCompressedFormat(format)
            ? CreateStagingBufferAndInitialize(stagingCreateInfo, imageData, imageDataSize)
            : CreateTextureStagingBufferAndInitialize(stagingCreateInfo, imageData, imageDataSize, extent, srcRowStride, bytesPerPixel)
    );

    /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
    VkCommandBuffer cmdBuffer = AllocCommandBuffer();
    {
        VkImageLayout oldLayout = textureVK.TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource, true);

        /* Use input offset and extent (instead of transient dimensions) because copy operation takes subresource parameters into account */
        context_.CopyBufferToImage(
            stagingBuffer.GetVkBuffer(),
            image,
            textureVK.GetVkFormat(),
            VkOffset3D{ textureRegion.offset.x, textureRegion.offset.y, textureRegion.offset.z },
            VkExtent3D{ textureRegion.extent.width, textureRegion.extent.height, textureRegion.extent.depth },
            subresource
        );

        textureVK.TransitionImageLayout(context_, oldLayout, subresource, true);
    }
    FlushCommandBuffer(cmdBuffer);

    /* Release staging buffer */
    stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
}

void VKRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);

    /* Determine size of image for staging buffer */
    const TextureSubresource&   subresource     = textureRegion.subresource;
  //const Offset3D              offset          = CalcTextureOffset(textureVK.GetType(), textureRegion.offset, subresource.baseArrayLayer);
    const Extent3D              extent          = CalcTextureExtent(textureVK.GetType(), textureRegion.extent, subresource.numArrayLayers);
    const Format                format          = textureVK.GetFormat();
    const FormatAttributes&     formatAttribs   = GetFormatAttribs(format);
    const std::size_t           imageNumTexels  = extent.width * extent.height * extent.depth;

    VkDeviceSize stagingBufferSize      = 0;
    VkDeviceSize depthBufferOffset      = 0;
    VkDeviceSize depthBufferSize        = 0;
    VkDeviceSize stencilBufferOffset    = 0;
    VkDeviceSize stencilBufferSize      = 0;

    if (IsDepthAndStencilFormat(format))
    {
        depthBufferSize     = static_cast<VkDeviceSize>(GetMemoryFootprint(formatAttribs.format, formatAttribs.dataType, imageNumTexels));
        stencilBufferSize   = static_cast<VkDeviceSize>(GetMemoryFootprint(ImageFormat::Stencil, DataType::UInt8, imageNumTexels));

        stencilBufferOffset = 0;
        depthBufferOffset   = GetAlignedSize<VkDeviceSize>(stencilBufferSize, 4u);
        stagingBufferSize   = GetAlignedSize<VkDeviceSize>(depthBufferOffset + depthBufferSize, 4u);
    }
    else
    {
        stagingBufferSize = static_cast<VkDeviceSize>(GetMemoryFootprint(format, imageNumTexels));
    }

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    BuildVkBufferCreateInfo(stagingCreateInfo, stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VKDeviceBuffer stagingBuffer = CreateStagingBuffer(stagingCreateInfo);

    /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
    VkCommandBuffer cmdBuffer = AllocCommandBuffer();
    {
        VkImageLayout oldLayout = textureVK.TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource, true);

        /* Use input offset and extent (instead of transient dimensions) because copy operation takes subresource parameters into account */
        context_.CopyImageToBuffer(
            textureVK.GetVkImage(),
            stagingBuffer.GetVkBuffer(),
            textureVK.GetVkFormat(),
            VkOffset3D{ textureRegion.offset.x, textureRegion.offset.y, textureRegion.offset.z },
            VkExtent3D{ textureRegion.extent.width, textureRegion.extent.height, textureRegion.extent.depth },
            subresource,
            depthBufferOffset,
            stencilBufferOffset
        );

        textureVK.TransitionImageLayout(context_, oldLayout, subresource, true);
    }
    FlushCommandBuffer(cmdBuffer);

    /* Map staging buffer to CPU memory space */
    if (VKDeviceMemoryRegion* region = stagingBuffer.GetMemoryRegion())
    {
        /* Map buffer memory to host memory */
        VKDeviceMemory* deviceMemory = region->GetParentChunk();
        if (void* memory = deviceMemory->Map(device_, region->GetOffset(), stagingBufferSize))
        {
            /* Copy data to buffer object */
            if (IsDepthAndStencilFormat(format))
            {
                const std::uint8_t* memoryDepthStart = reinterpret_cast<const std::uint8_t*>(memory) + depthBufferOffset;
                const ImageView srcImageViewDepth{ formatAttribs.format, formatAttribs.dataType, memoryDepthStart, static_cast<std::size_t>(depthBufferSize) };
                ConvertImageBuffer(srcImageViewDepth, dstImageView, extent, /*LLGL_MAX_THREAD_COUNT*/0, true, ~0u, 0u);

                const std::uint8_t* memoryStencilStart = reinterpret_cast<const std::uint8_t*>(memory) + stencilBufferOffset;
                const ImageView srcImageViewStencil{ ImageFormat::Stencil, DataType::UInt8, memoryStencilStart, static_cast<std::size_t>(stencilBufferSize) };
                ConvertImageBuffer(srcImageViewStencil, dstImageView, extent, /*LLGL_MAX_THREAD_COUNT*/0, true, 0u, ~0u);
            }
            else
            {
                const ImageView srcImageView{ formatAttribs.format, formatAttribs.dataType, memory, static_cast<std::size_t>(stagingBufferSize) };
                ConvertImageBuffer(srcImageView, dstImageView, extent, LLGL_MAX_THREAD_COUNT, true);
            }
            deviceMemory->Unmap(device_);
        }
    }

    /* Release staging buffer */
    stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
}

/* ----- Sampler States ---- */

Sampler* VKRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<VKSampler>(device_, samplerDesc);
}

void VKRenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* VKRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<VKResourceHeap>(device_, resourceHeapDesc, initialResourceViews);
}

void VKRenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t VKRenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceHeap&, resourceHeap);
    return resourceHeapVK.WriteResourceViews(device_, firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* VKRenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<VKRenderPass>(device_, renderPassDesc);
}

void VKRenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* VKRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<VKRenderTarget>(device_, *deviceMemoryMngr_, renderTargetDesc);
}

void VKRenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* VKRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    RenderSystem::AssertCreateShader(shaderDesc);
    return shaders_.emplace<VKShader>(device_, shaderDesc);
}

void VKRenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* VKRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<VKPipelineLayout>(device_, pipelineLayoutDesc);
}

void VKRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* VKRenderSystem::CreatePipelineCache(const Blob& initialBlob)
{
    return pipelineCaches_.emplace<VKPipelineCache>(device_, initialBlob);
}

void VKRenderSystem::Release(PipelineCache& pipelineCache)
{
    pipelineCaches_.erase(&pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* VKRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<VKGraphicsPSO>(
        device_,
        (!swapChains_.empty() ? (*swapChains_.begin())->GetRenderPass() : nullptr),
        pipelineStateDesc,
        graphicsPipelineLimits_,
        pipelineCache
    );
}

PipelineState* VKRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<VKComputePSO>(device_, pipelineStateDesc, pipelineCache);
}

PipelineState* VKRenderSystem::CreatePipelineState(const MeshPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    #if VK_EXT_mesh_shader
    if (HasExtension(VKExt::EXT_mesh_shader))
    {
        return pipelineStates_.emplace<VKGraphicsPSO>(
            device_,
            (!swapChains_.empty() ? (*swapChains_.begin())->GetRenderPass() : nullptr),
            pipelineStateDesc,
            graphicsPipelineLimits_,
            pipelineCache
        );
    }
    else
    #endif // /VK_EXT_mesh_shader
    {
        return nullptr; // VK_EXT_mesh_shader not supported
    }
}

void VKRenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* VKRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    if (queryHeapDesc.renderCondition)
        return queryHeaps_.emplace<VKPredicateQueryHeap>(device_, *deviceMemoryMngr_, queryHeapDesc);
    else
        return queryHeaps_.emplace<VKQueryHeap>(device_, queryHeapDesc);
}

void VKRenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* VKRenderSystem::CreateFence()
{
    return fences_.emplace<VKFence>(device_);
}

void VKRenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool VKRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Vulkan::RenderSystemNativeHandle))
    {
        auto* nativeHandleVK = static_cast<Vulkan::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleVK->instance        = instance_.Get();
        nativeHandleVK->physicalDevice  = physicalDevice_.GetVkPhysicalDevice();
        nativeHandleVK->device          = device_.GetVkDevice();
        nativeHandleVK->queue           = device_.GetGraphicsQueue()->native;
        nativeHandleVK->queueFamily     = device_.GetQueueFamilyIndices().graphicsFamily;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void VKRenderSystem::QuerySupportedInstanceExtensions()
{
    /* Query instance extension properties */
    instanceExtensionProperties_ = VKQueryInstanceExtensionProperties();

    for (const VkExtensionProperties& prop : instanceExtensionProperties_)
    {
        if (VKIsInstanceExtensionEnabled(prop.extensionName, isDebugLayerEnabled_))
            supportedInstanceExtensions_.push_back(prop.extensionName);
    }
}

void VKRenderSystem::CreateInstance(const RendererConfigurationVulkan* config)
{
    /* Determine supported Vulkan API version */
    std::uint32_t instanceVersion = 0;
    vkEnumerateInstanceVersion(&instanceVersion);
    LLGL_ASSERT(instanceVersion >= VK_API_VERSION_1_0, "vkEnumerateInstanceVersion(instanceVersion = %u)", instanceVersion);

    /* Query instance layer properties */
    const std::vector<VkLayerProperties> layerProperties = VKQueryInstanceLayerProperties();
    std::vector<const char*> layerNames;

    for (const VkLayerProperties& prop : layerProperties)
    {
        if (IsLayerRequired(prop.layerName, config))
            layerNames.push_back(prop.layerName);
    }

    /* Setup Vulkan instance descriptor */
    VkInstanceCreateInfo instanceInfo = {};

    #if VK_KHR_portability_enumeration
    for (const char* extensionName : supportedInstanceExtensions_)
    {
        if (::strcmp(extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
        {
            instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            break;
        }
    }
    #endif

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    /* Specify application descriptor */
    VkApplicationInfo appInfo = {};
    {
        appInfo.sType       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion  = instanceVersion;
        if (config != nullptr)
        {
            appInfo.pApplicationName    = config->application.applicationName;
            appInfo.applicationVersion  = config->application.applicationVersion;
            appInfo.pEngineName         = config->application.engineName;
            appInfo.engineVersion       = config->application.engineVersion;
        }
    }
    instanceInfo.pApplicationInfo = (&appInfo);

    /* Specify layers to enable  */
    if (!layerNames.empty())
    {
        instanceInfo.enabledLayerCount          = static_cast<std::uint32_t>(layerNames.size());
        instanceInfo.ppEnabledLayerNames        = layerNames.data();
    }

    /* Specify extensions to enable */
    if (!supportedInstanceExtensions_.empty())
    {
        instanceInfo.enabledExtensionCount      = static_cast<std::uint32_t>(supportedInstanceExtensions_.size());
        instanceInfo.ppEnabledExtensionNames    = supportedInstanceExtensions_.data();
    }

    #if VK_EXT_validation_features

    #if LLGL_VK_ENABLE_GPU_ASSISTED_VALIDATION
    const VkValidationFeatureEnableEXT validationFeaturesEnabled[] =
    {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
        //VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        //VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    VkValidationFeaturesEXT validationFeatures = {};

    /* Enable GPU-assisted validation if debug layer is enabled and Vulkan 1.1 or later is supported */
    if (isDebugLayerEnabled_ && instanceVersion >= VK_API_VERSION_1_1)
    {
        validationFeatures.sType                            = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeatures.enabledValidationFeatureCount    = LLGL_ARRAY_LENGTH(validationFeaturesEnabled);
        validationFeatures.pEnabledValidationFeatures       = validationFeaturesEnabled;
        instanceInfo.pNext = &validationFeatures;
    }
    #endif

    #endif // /VK_EXT_validation_features

    /* Create Vulkan instance */
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan instance");
}

// Suppresses DebugBreakOnError for validation messages caused by known third-party OpenXR-runtime bugs that
// the application cannot fix. This handles only errors NOT tied to a command buffer (e.g. object creation);
// errors recorded into / submitted with a command buffer are attributed precisely by command-buffer ownership
// in ShouldBreakOnValidationError below, so they don't need per-VUID entries here. The message is still logged
// through DebugPuts so real problems remain visible. Remove entries once the runtimes ship a fix.
static bool IsKnownXrRuntimeNonCommandFalsePositive(const char* message)
{
    if (message == nullptr)
        return false;

    // SteamVR's own vkCreateImage with PREINITIALIZED + external-memory (its compositor scene/eye textures).
    // This is an object-creation error with no associated command buffer, so ownership can't attribute it.
    return (std::strstr(message, "VUID-VkImageCreateInfo-pNext-01443") != nullptr);
}

// Returns true if the named object belongs to the external XR runtime (e.g. SteamVR/OpenVR compositor) that
// shares LLGL's VkDevice. Used to attribute validation errors whose object list contains only a runtime image
// (and no command buffer) -- e.g. the runtime's own depth-image barriers with aspectMask=0 / wrong layouts.
static bool IsKnownExternalRuntimeObjectName(const char* objectName)
{
    if (objectName == nullptr)
        return false;
    static const char* kNames[] =
    {
        "CSxrSwapchainVulkanOpenVR", // SteamVR/OpenVR swap-chain images (shared with LLGL)
        "BlankEyeBuffer",            // SteamVR fallback texture submitted when the app provides no layer
        "Scene create Vulkan",       // SteamVR compositor scene/eye textures
    };
    for (const char* name : kNames)
    {
        if (std::strstr(objectName, name) != nullptr)
            return true;
    }
    return false;
}

// Decides whether a validation error should trigger the debug break. The OpenXR runtime compositor shares
// LLGL's VkDevice and records/submits its own command buffers on it, so the validation layer reports the
// runtime's errors to our callback too. We only break for errors LLGL is responsible for: those tied to a
// command buffer LLGL allocated. Errors tied only to command buffers we don't own (the runtime's) are skipped;
// errors not tied to any command buffer (e.g. object creation) break unless they are a known runtime false
// positive. This relies on the debug-utils object list, which the debug-report callback does not provide.
static bool ShouldBreakOnValidationError(const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    bool hasCommandBuffer = false;
#if LLGL_BUILD_XR_OPENXR
    bool anyNamedObject                 = false;
    bool allNamedObjectsAreRuntimeOwned = true;
#endif // /LLGL_BUILD_XR_OPENXR

    if (callbackData->pObjects != nullptr)
    {
        for (std::uint32_t i = 0; i < callbackData->objectCount; ++i)
        {
            const VkDebugUtilsObjectNameInfoEXT& obj = callbackData->pObjects[i];
            if (obj.objectType == VK_OBJECT_TYPE_COMMAND_BUFFER)
            {
                hasCommandBuffer = true;
                if (VKIsLLGLCommandBuffer(reinterpret_cast<VkCommandBuffer>(obj.objectHandle)))
                    return true; // LLGL recorded/submitted this command buffer -> our responsibility
            }
        #if LLGL_BUILD_XR_OPENXR
            if (obj.pObjectName != nullptr)
            {
                anyNamedObject = true;
                if (!IsKnownExternalRuntimeObjectName(obj.pObjectName))
                    allNamedObjectsAreRuntimeOwned = false;
            }
        #endif // /LLGL_BUILD_XR_OPENXR
        }
    }

    // The error references only command buffers LLGL does not own (e.g. an OpenXR runtime compositor's).
    if (hasCommandBuffer)
        return false;

#if LLGL_BUILD_XR_OPENXR
    // No (LLGL) command buffer in the object list. If every named object belongs to the external XR runtime --
    // e.g. a standalone barrier on the runtime's own swap-chain/scene image whose validation message omits the
    // recording command buffer -- the error is the runtime's, not LLGL's.
    if (anyNamedObject && allNamedObjectsAreRuntimeOwned)
        return false;

    // Object-creation and other command-less errors the runtime is known to produce (unnamed at the time).
    if (IsKnownXrRuntimeNonCommandFalsePositive(callbackData->pMessage))
        return false;
#endif // /LLGL_BUILD_XR_OPENXR

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugUtilsCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*                                       userData)
{
    if (callbackData == nullptr || callbackData->pMessage == nullptr)
        return VK_FALSE;

    DebugPuts(callbackData->pMessage);

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        VKRenderSystem* renderSystemVK = static_cast<VKRenderSystem*>(userData);
        if (renderSystemVK->IsBreakOnErrorEnabled() && ShouldBreakOnValidationError(callbackData))
            DebugBreakOnError();
    }

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   createInfo,
    const VkAllocationCallbacks*                allocator,
    VkDebugUtilsMessengerEXT*                   messenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
        return func(instance, createInfo, allocator, messenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance                      instance,
    VkDebugUtilsMessengerEXT        messenger,
    const VkAllocationCallbacks*    allocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
        func(instance, messenger, allocator);
}

void VKRenderSystem::CreateDebugMessenger()
{
    /*
    Enable command-buffer tracking so the messenger can attribute validation errors to LLGL by command-buffer
    ownership (see ShouldBreakOnValidationError). Done before any command buffers are allocated.
    */
    VKSetCommandBufferTrackingEnabled(true);

    /* Create debug-utils messenger (used over debug-report so the callback receives the labeled object list) */
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback  = VKDebugUtilsCallback;
        createInfo.pUserData        = this;
    }
    debugMessenger_ = VKPtr<VkDebugUtilsMessengerEXT>{ instance_, DestroyDebugUtilsMessengerEXT };
    auto result = CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, debugMessenger_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan debug utils messenger");
}

bool VKRenderSystem::PickPhysicalDevice(long preferredDeviceFlags, VkPhysicalDevice customPhysicalDevice)
{
    /* Pick physical device with Vulkan support */
    if (customPhysicalDevice != VK_NULL_HANDLE)
    {
        /* Load weak reference to custom native physical device */
        physicalDevice_.LoadPhysicalDeviceWeakRef(customPhysicalDevice);
    }
    else if (!physicalDevice_.PickPhysicalDevice(instance_, supportedInstanceExtensions_, preferredDeviceFlags))
    {
        GetMutableReport().Errorf("failed to find suitable Vulkan device");
        return false;
    }

    /* Store graphics pipeline limits for this physical device */
    physicalDevice_.QueryPipelineLimits(graphicsPipelineLimits_);

    return true;
}

void VKRenderSystem::CreateLogicalDevice(VkDevice customLogicalDevice)
{
    /* Create logical device with all supported physical device feature */
    device_ = physicalDevice_.CreateLogicalDevice(customLogicalDevice);

    /* Create command queue interface */
    commandQueue_ = MakeUnique<VKCommandQueue>(device_, device_.GetGraphicsQueue());

    /* Load Vulkan device extensions */
    VKLoadDeviceExtensions(device_, physicalDevice_.GetExtensionNames());
}

bool VKRenderSystem::IsLayerRequired(const char* name, const RendererConfigurationVulkan* config) const
{
    if (config != nullptr)
    {
        for (const char* layer : config->enabledLayers)
        {
            if (::strcmp(layer, name) == 0)
                return true;
        }
    }

    return (isDebugLayerEnabled_ && VKIsInstanceDebugLayer(name));
}

VKDeviceBuffer VKRenderSystem::CreateStagingBuffer(const VkBufferCreateInfo& createInfo)
{
    return VKDeviceBuffer
    {
        device_,
        createInfo,
        *deviceMemoryMngr_,
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };
}

VKDeviceBuffer VKRenderSystem::CreateStagingBufferAndInitialize(
    const VkBufferCreateInfo&   createInfo,
    const void*                 data,
    VkDeviceSize                dataSize)
{
    /* Allocate staging buffer */
    VKDeviceBuffer stagingBuffer = CreateStagingBuffer(createInfo);

    /* Copy initial data to buffer memory */
    if (data != nullptr && dataSize > 0)
        device_.WriteBuffer(stagingBuffer, data, dataSize);

    return stagingBuffer;
}

VKDeviceBuffer VKRenderSystem::CreateTextureStagingBufferAndInitialize(
    const VkBufferCreateInfo&   createInfo,
    const void*                 data,
    VkDeviceSize                dataSize,
    const Extent3D&             extent,
    std::uint32_t               srcRowStride,
    std::uint32_t               bpp)
{
    /* Allocate staging buffer */
    VKDeviceBuffer stagingBuffer = CreateStagingBuffer(createInfo);

    /* Copy initial data to buffer memory */
    if (data != nullptr && dataSize > 0)
    {
        if (VKDeviceMemoryRegion* region = stagingBuffer.GetMemoryRegion())
        {
            const std::uint32_t dstRowStride    = extent.width * bpp;
            const std::uint32_t dstLayerStride  = extent.height * dstRowStride;

            const std::uint32_t srcLayerStride  = extent.height * srcRowStride;

            /* Map buffer memory to host memory */
            VKDeviceMemory* deviceMemory = region->GetParentChunk();
            if (void* memory = deviceMemory->Map(device_, region->GetOffset(), dataSize))
            {
                const char* src = static_cast<const char*>(data);
                char* dst = static_cast<char*>(memory);

                BitBlit(extent, bpp, dst, dstRowStride, dstLayerStride, src, srcRowStride, srcLayerStride);

                deviceMemory->Unmap(device_);
            }
        }
    }

    return stagingBuffer;
}

VkCommandBuffer VKRenderSystem::AllocCommandBuffer(bool begin)
{
    VkCommandBuffer cmdBuffer = device_.AllocCommandBuffer(begin);
    context_.Reset(cmdBuffer);
    return cmdBuffer;
}

void VKRenderSystem::FlushCommandBuffer(VkCommandBuffer commandBuffer)
{
    device_.FlushCommandBuffer(commandBuffer);
}

bool VKRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
    {
        /* Query rendering information from selected physical device and store Vulkan extension names */
        physicalDevice_.QueryRendererInfo(*outInfo);
        const std::vector<const char*>& extensions = physicalDevice_.GetExtensionNames();
        outInfo->extensionNames = std::vector<UTF8String>(extensions.begin(), extensions.end());
    }
    if (outCaps != nullptr)
    {
        /* Query rendering capabilities from selected physical device */
        physicalDevice_.QueryRenderingCaps(*outCaps);
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
