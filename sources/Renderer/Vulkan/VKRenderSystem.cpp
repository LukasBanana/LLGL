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
#include "Memory/VKDeviceMemory.h"
#include "../RenderSystemUtils.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Vendor.h"
#include "VKCore.h"
#include "VKTypes.h"
#include "VKInitializers.h"
#include "RenderState/VKPredicateQueryHeap.h"
#include "RenderState/VKComputePSO.h"
#include "Shader/VKShaderModulePool.h"
#include "../../Platform/Debug.h"
#include <LLGL/ImageFlags.h>
#include <limits>

#include <LLGL/Backend/Vulkan/NativeHandle.h>


namespace LLGL
{


VKRenderSystem::VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    instance_          { vkDestroyInstance                                                },
    debugLayerEnabled_ { ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0) }
{
    /* Extract optional renderer configuartion */
    auto* rendererConfigVK = GetRendererConfiguration<RendererConfigurationVulkan>(renderSystemDesc);

    constexpr long preferredDeviceMask = (RenderSystemFlags::PreferNVIDIA | RenderSystemFlags::PreferAMD | RenderSystemFlags::PreferIntel);
    const long preferredDeviceFlags = (renderSystemDesc.flags & preferredDeviceMask);

    if (auto* customNativeHandle = GetRendererNativeHandle<Vulkan::RenderSystemNativeHandle>(renderSystemDesc))
    {
        /* Store weak references to native handles */
        instance_ = VKPtr<VkInstance>{ customNativeHandle->instance };
        if (debugLayerEnabled_)
            CreateDebugReportCallback();
        VKLoadInstanceExtensions(instance_);
        if (!PickPhysicalDevice(preferredDeviceFlags, customNativeHandle->physicalDevice))
            return;
        CreateLogicalDevice(customNativeHandle->device);
    }
    else
    {
        /* Create Vulkan instance and device objects */
        CreateInstance(rendererConfigVK);
        if (debugLayerEnabled_)
            CreateDebugReportCallback();
        VKLoadInstanceExtensions(instance_);
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
    VKPipelineLayout::ReleaseDefault();
}

/* ----- Swap-chain ----- */

SwapChain* VKRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<VKSwapChain>(instance_, physicalDevice_, device_, *deviceMemoryMngr_, swapChainDesc, surface, GetRendererInfo());
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
    return commandBuffers_.emplace<VKCommandBuffer>(physicalDevice_, device_, device_.GetVkQueue(), device_.GetQueueFamilyIndices(), commandBufferDesc);
}

void VKRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

static VkBufferUsageFlags GetStagingVkBufferUsageFlags(long cpuAccessFlags)
{
    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    else
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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

    /* Set up initial image data */
    const void* initialData = nullptr;
    DynamicByteArray intermediateData;

    if (initialImage != nullptr)
    {
        /* Check if image data must be converted */
        const auto& formatAttribs = GetFormatAttribs(textureDesc.format);
        if (formatAttribs.bitSize > 0 && (formatAttribs.flags & FormatFlags::IsCompressed) == 0)
        {
            /* Convert image format (will be null if no conversion is necessary) */
            intermediateData = ConvertImageBuffer(*initialImage, formatAttribs.format, formatAttribs.dataType, LLGL_MAX_THREAD_COUNT);
        }

        if (intermediateData)
        {
            /*
            Validate that source image data was large enough so conversion is valid,
            then use temporary image buffer as source for initial data
            */
            const std::size_t srcImageDataSize = GetMemoryFootprint(initialImage->format, initialImage->dataType, imageSize);
            RenderSystem::AssertImageDataSize(initialImage->dataSize, srcImageDataSize);
            initialData = intermediateData.get();
        }
        else
        {
            /*
            Validate that image data is large enough,
            then use input data as source for initial data
            */
            RenderSystem::AssertImageDataSize(initialImage->dataSize, initialDataSize);
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

    if (initialData != nullptr)
    {
        /* Create staging buffer */
        VkBufferCreateInfo stagingCreateInfo;
        BuildVkBufferCreateInfo(
            stagingCreateInfo,
            initialDataSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );

        VKDeviceBuffer stagingBuffer = CreateStagingBufferAndInitialize(stagingCreateInfo, initialData, initialDataSize);

        /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
        VkCommandBuffer cmdBuffer = AllocCommandBuffer();
        {
            const TextureSubresource subresource{ 0, textureVK->GetNumArrayLayers(), 0, textureVK->GetNumMipLevels() };

            textureVK->TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);

            context_.CopyBufferToImage(
                stagingBuffer.GetVkBuffer(),
                textureVK->GetVkImage(),
                textureVK->GetVkFormat(),
                VkOffset3D{ 0, 0, 0 },
                textureVK->GetVkExtent(),
                subresource
            );

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
    const Offset3D&             offset          = textureRegion.offset;
    const Extent3D&             extent          = textureRegion.extent;
    const TextureSubresource&   subresource     = textureRegion.subresource;
    const Format                format          = VKTypes::Unmap(textureVK.GetVkFormat());

    VkImage                     image           = textureVK.GetVkImage();
    const std::uint32_t         imageSize       = extent.width * extent.height * extent.depth * subresource.numArrayLayers;
    const void*                 imageData       = nullptr;
    const VkDeviceSize          imageDataSize   = static_cast<VkDeviceSize>(GetMemoryFootprint(format, imageSize));

    /* Check if image data must be converted */
    DynamicByteArray intermediateData;

    const auto& formatAttribs = GetFormatAttribs(format);
    if (formatAttribs.bitSize > 0 && (formatAttribs.flags & FormatFlags::IsCompressed) == 0)
    {
        /* Convert image format (will be null if no conversion is necessary) */
        intermediateData = ConvertImageBuffer(srcImageView, formatAttribs.format, formatAttribs.dataType, LLGL_MAX_THREAD_COUNT);
    }

    if (intermediateData)
    {
        /*
        Validate that source image data was large enough so conversion is valid,
        then use temporary image buffer as source for initial data
        */
        const std::size_t srcImageDataSize = GetMemoryFootprint(srcImageView.format, srcImageView.dataType, imageSize);
        RenderSystem::AssertImageDataSize(srcImageView.dataSize, srcImageDataSize);
        imageData = intermediateData.get();
    }
    else
    {
        /*
        Validate that image data is large enough,
        then use input data as source for initial data
        */
        RenderSystem::AssertImageDataSize(srcImageView.dataSize, static_cast<std::size_t>(imageDataSize));
        imageData = srcImageView.data;
    }

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    BuildVkBufferCreateInfo(
        stagingCreateInfo,
        imageDataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT // <-- TODO: support read/write mapping //GetStagingVkBufferUsageFlags(bufferDesc.cpuAccessFlags)
    );

    VKDeviceBuffer stagingBuffer = CreateStagingBufferAndInitialize(stagingCreateInfo, imageData, imageDataSize);

    /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
    VkCommandBuffer cmdBuffer = AllocCommandBuffer();
    {
        VkImageLayout oldLayout = textureVK.TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource, true);

        context_.CopyBufferToImage(
            stagingBuffer.GetVkBuffer(),
            image,
            textureVK.GetVkFormat(),
            VkOffset3D{ offset.x, offset.y, offset.z },
            VkExtent3D{ extent.width, extent.height, extent.depth },
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
    const Offset3D              offset          = CalcTextureOffset(textureVK.GetType(), textureRegion.offset);
    const Extent3D              extent          = CalcTextureExtent(textureVK.GetType(), textureRegion.extent);
    const TextureSubresource&   subresource     = textureRegion.subresource;
    const Format                format          = VKTypes::Unmap(textureVK.GetVkFormat());
    const FormatAttributes&     formatAttribs   = GetFormatAttribs(format);

    VkImage                     image           = textureVK.GetVkImage();
    const std::uint32_t         imageSize       = extent.width * extent.height * extent.depth * subresource.numArrayLayers;
    const VkDeviceSize          imageDataSize   = static_cast<VkDeviceSize>(GetMemoryFootprint(format, imageSize));

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    BuildVkBufferCreateInfo(stagingCreateInfo, imageDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VKDeviceBuffer stagingBuffer = CreateStagingBuffer(stagingCreateInfo);

    /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
    VkCommandBuffer cmdBuffer = AllocCommandBuffer();
    {
        VkImageLayout oldLayout = textureVK.TransitionImageLayout(context_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource, true);

        context_.CopyImageToBuffer(
            image,
            stagingBuffer.GetVkBuffer(),
            textureVK.GetVkFormat(),
            VkOffset3D{ offset.x, offset.y, offset.z },
            VkExtent3D{ extent.width, extent.height, extent.depth },
            subresource
        );

        textureVK.TransitionImageLayout(context_, oldLayout, subresource, true);
    }
    FlushCommandBuffer(cmdBuffer);

    /* Map staging buffer to CPU memory space */
    if (VKDeviceMemoryRegion* region = stagingBuffer.GetMemoryRegion())
    {
        /* Map buffer memory to host memory */
        VKDeviceMemory* deviceMemory = region->GetParentChunk();
        if (void* memory = deviceMemory->Map(device_, region->GetOffset(), imageDataSize))
        {
            /* Copy data to buffer object */
            const ImageView srcImageView{ formatAttribs.format, formatAttribs.dataType, memory, static_cast<std::size_t>(imageDataSize) };
            RenderSystem::CopyTextureImageData(dstImageView, srcImageView, imageSize, extent.width);
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
        auto* nativeHandleVK = reinterpret_cast<Vulkan::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleVK->instance        = instance_.Get();
        nativeHandleVK->physicalDevice  = physicalDevice_.GetVkPhysicalDevice();
        nativeHandleVK->device          = device_.GetVkDevice();
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

#ifndef VK_LAYER_KHRONOS_VALIDATION_NAME
#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"
#endif

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

    /* Query instance extension properties */
    const std::vector<VkExtensionProperties> extensionProperties = VKQueryInstanceExtensionProperties();
    std::vector<const char*> extensionNames;

    auto IsVKExtSupportIncluded = [this](VKExtSupport extSupport)
    {
        return
        (
            extSupport == VKExtSupport::Required ||
            extSupport == VKExtSupport::Optional ||
            (this->debugLayerEnabled_ && extSupport == VKExtSupport::DebugOnly)
        );
    };

    for (const VkExtensionProperties& prop : extensionProperties)
    {
        const VKExtSupport extSupport = GetVulkanInstanceExtensionSupport(prop.extensionName);
        if (IsVKExtSupportIncluded(extSupport))
            extensionNames.push_back(prop.extensionName);
    }

    /* Setup Vulkan instance descriptor */
    VkInstanceCreateInfo instanceInfo = {};

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
    if (!extensionNames.empty())
    {
        instanceInfo.enabledExtensionCount      = static_cast<std::uint32_t>(extensionNames.size());
        instanceInfo.ppEnabledExtensionNames    = extensionNames.data();
    }

    #ifdef VK_EXT_validation_features

    const VkValidationFeatureEnableEXT validationFeaturesEnabled[] =
    {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
        //VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        //VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    VkValidationFeaturesEXT validationFeatures = {};

    /* Enable GPU-assisted validation if debug layer is enabled and Vulkan 1.1 or later is supported */
    if (debugLayerEnabled_ && instanceVersion >= VK_API_VERSION_1_1)
    {
        validationFeatures.sType                            = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeatures.enabledValidationFeatureCount    = LLGL_ARRAY_LENGTH(validationFeaturesEnabled);
        validationFeatures.pEnabledValidationFeatures       = validationFeaturesEnabled;
        instanceInfo.pNext = &validationFeatures;
    }

    #endif // /VK_EXT_validation_features

    /* Create Vulkan instance */
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan instance");
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 layerPrefix,
    const char*                 message,
    void*                       userData)
{
    DebugPuts(message);
    return VK_FALSE;
}

static VkResult CreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   createInfo,
    const VkAllocationCallbacks*                allocator,
    VkDebugReportCallbackEXT*                   callback)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    if (func != nullptr)
        return func(instance, createInfo, allocator, callback);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugReportCallbackEXT(
    VkInstance                      instance,
    VkDebugReportCallbackEXT        callback,
    const VkAllocationCallbacks*    allocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    if (func != nullptr)
        func(instance, callback, allocator);
}

void VKRenderSystem::CreateDebugReportCallback()
{
    /* Initialize flags */
    VkDebugReportFlagsEXT flags = 0;

    //flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
    //flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
    //flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    /* Create report callback */
    VkDebugReportCallbackCreateInfoEXT createInfo;
    {
        createInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext        = nullptr;
        createInfo.flags        = flags;
        createInfo.pfnCallback  = VKDebugCallback;
        createInfo.pUserData    = reinterpret_cast<void*>(this);
    }
    debugReportCallback_ = VKPtr<VkDebugReportCallbackEXT>{ instance_, DestroyDebugReportCallbackEXT };
    auto result = CreateDebugReportCallbackEXT(instance_, &createInfo, nullptr, debugReportCallback_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan debug report callback");
}

bool VKRenderSystem::PickPhysicalDevice(long preferredDeviceFlags, VkPhysicalDevice customPhysicalDevice)
{
    /* Pick physical device with Vulkan support */
    if (customPhysicalDevice != VK_NULL_HANDLE)
    {
        /* Load weak reference to custom native physical device */
        physicalDevice_.LoadPhysicalDeviceWeakRef(customPhysicalDevice);
    }
    else if (!physicalDevice_.PickPhysicalDevice(instance_, preferredDeviceFlags))
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
    commandQueue_ = MakeUnique<VKCommandQueue>(device_, device_.GetVkQueue());

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

    if (debugLayerEnabled_)
    {
        if (::strcmp(name, VK_LAYER_KHRONOS_VALIDATION_NAME) == 0)
            return true;
    }

    return false;
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
