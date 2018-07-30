/*
 * VKRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Platform/Platform.h>
#include "VKRenderSystem.h"
#include "Ext/VKExtensionLoader.h"
#include "Ext/VKExtensions.h"
#include "Memory/VKDeviceMemory.h"
#include "Buffer/VKIndexBuffer.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
#include "../GLCommon/GLTypes.h"
#include "VKCore.h"
#include "VKTypes.h"
#include "VKInitializers.h"
#include <LLGL/Log.h>


namespace LLGL
{


/* ----- Internal functions ----- */

static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugReportCallbackEXT* callback)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    if (func != nullptr)
        return func(instance, createInfo, allocator, callback);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* allocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    if (func != nullptr)
        func(instance, callback, allocator);
}

static VkBufferUsageFlags GetVkBufferUsageFlags(long bufferFlags)
{
    if ((bufferFlags & BufferFlags::MapReadAccess) != 0)
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    else
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
}

static VkBufferUsageFlags GetStagingVkBufferUsageFlags(long bufferFlags)
{
    if ((bufferFlags & BufferFlags::MapWriteAccess) != 0)
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    else
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
}


/* ----- Common ----- */

VKRenderSystem::VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    instance_            { vkDestroyInstance                        },
    debugReportCallback_ { instance_, DestroyDebugReportCallbackEXT }
{
    /* Extract optional renderer configuartion */
    const VulkanRendererConfiguration* rendererConfigVK= nullptr;

    if (renderSystemDesc.rendererConfig != nullptr && renderSystemDesc.rendererConfigSize > 0)
    {
        if (renderSystemDesc.rendererConfigSize == sizeof(VulkanRendererConfiguration))
            rendererConfigVK = reinterpret_cast<const VulkanRendererConfiguration*>(renderSystemDesc.rendererConfig);
        else
            throw std::invalid_argument("invalid renderer configuration structure (expected size of 'VulkanRendererConfiguration' structure)");
    }

    #ifdef LLGL_DEBUG
    debugLayerEnabled_ = true;
    #endif

    /* Create Vulkan instance and device objects */
    CreateInstance(rendererConfigVK != nullptr ? &(rendererConfigVK->application) : nullptr);
    LoadExtensions();
    PickPhysicalDevice();
    CreateLogicalDevice();

    /* Create default resources */
    CreateDefaultPipelineLayout();

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
}

/* ----- Render Context ----- */

RenderContext* VKRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(
        renderContexts_,
        MakeUnique<VKRenderContext>(instance_, physicalDevice_, device_, *deviceMemoryMngr_, desc, surface)
    );
}

void VKRenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* VKRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* VKRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    return TakeOwnership(
        commandBuffers_,
        MakeUnique<VKCommandBuffer>(device_, device_.GetVkQueue(), device_.GetQueueFamilyIndices(), desc)
    );
}

CommandBufferExt* VKRenderSystem::CreateCommandBufferExt(const CommandBufferDescriptor& /*desc*/)
{
    /* Extended command buffers are not spported */
    return nullptr;
}

void VKRenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

Buffer* VKRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    static const long g_stagingBufferRelatedFlags = (BufferFlags::MapReadWriteAccess | BufferFlags::DynamicUsage);

    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<VkDeviceSize>::max()));

    /* Create staging buffer */
    auto stagingCreateInfo = MakeVkBufferCreateInfo(
        static_cast<VkDeviceSize>(desc.size),
        GetStagingVkBufferUsageFlags(desc.flags)
    );

    auto stagingBuffer = CreateStagingBuffer(stagingCreateInfo, initialData, desc.size);

    /* Create device buffer */
    auto buffer = CreateGpuBuffer(desc, GetVkBufferUsageFlags(desc.flags));

    /* Allocate device memory */
    auto memoryRegion = deviceMemoryMngr_->Allocate(
        buffer->GetDeviceBuffer().GetRequirements(),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    buffer->BindMemoryRegion(device_, memoryRegion);

    /* Copy staging buffer into hardware buffer */
    device_.CopyBuffer(stagingBuffer.GetVkBuffer(), buffer->GetVkBuffer(), static_cast<VkDeviceSize>(desc.size));

    if ((desc.flags & g_stagingBufferRelatedFlags) != 0)
    {
        /* Store ownership of staging buffer */
        buffer->TakeStagingBuffer(std::move(stagingBuffer));
    }
    else
    {
        /* Release staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }

    return buffer;
}

BufferArray* VKRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    auto type = (*bufferArray)->GetType();
    return TakeOwnership(bufferArrays_, MakeUnique<VKBufferArray>(type, numBuffers, bufferArray));
}

void VKRenderSystem::Release(Buffer& buffer)
{
    /* Release device memory regions for primary buffer and internal staging buffer, then release buffer object */
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    bufferVK.GetDeviceBuffer().ReleaseMemoryRegion(*deviceMemoryMngr_);
    bufferVK.GetStagingDeviceBuffer().ReleaseMemoryRegion(*deviceMemoryMngr_);
    RemoveFromUniqueSet(buffers_, &buffer);
}

void VKRenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void VKRenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, dstBuffer);

    if (bufferVK.GetStagingVkBuffer() != VK_NULL_HANDLE)
    {
        /* Copy data to staging buffer memory */
        device_.WriteBuffer(bufferVK.GetStagingDeviceBuffer(), data, dataSize, dstOffset);

        /* Copy staging buffer into hardware buffer */
        device_.CopyBuffer(bufferVK.GetStagingVkBuffer(), bufferVK.GetVkBuffer(), dataSize, dstOffset, dstOffset);
    }
    else
    {
        /* Create staging buffer */
        auto stagingCreateInfo = MakeVkBufferCreateInfo(
            dataSize, (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        );

        auto stagingBuffer = CreateStagingBuffer(stagingCreateInfo, data, dataSize);

        /* Copy staging buffer into hardware buffer */
        device_.CopyBuffer(stagingBuffer.GetVkBuffer(), bufferVK.GetVkBuffer(), dataSize, 0, dstOffset);

        /* Release device memory region of staging buffer */
        stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);
    }
}

void* VKRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    if (auto stagingBuffer = bufferVK.GetStagingVkBuffer())
    {
        /* Copy GPU local buffer into staging buffer for read accces */
        if (access != CPUAccess::WriteOnly && access != CPUAccess::WriteDiscard)
            device_.CopyBuffer(bufferVK.GetVkBuffer(), stagingBuffer, bufferVK.GetSize());

        /* Map staging buffer */
        return bufferVK.Map(device_, access);
    }

    return nullptr;
}

void VKRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    if (auto stagingBuffer = bufferVK.GetStagingVkBuffer())
    {
        /* Unmap staging buffer */
        bufferVK.Unmap(device_);

        /* Copy staging buffer into GPU local buffer for write access */
        if (bufferVK.GetMappingCPUAccess() != CPUAccess::ReadOnly)
            device_.CopyBuffer(stagingBuffer, bufferVK.GetVkBuffer(), bufferVK.GetSize());
    }
}

/* ----- Textures ----- */

// Returns the extent for the specified texture dimensionality (used for the dimension of 'VK_IMAGE_TYPE_1D/ 2D/ 3D')
static VkExtent3D GetTextureVkExtent(const TextureDescriptor& desc)
{
    switch (desc.type)
    {
        case TextureType::Texture1D:        /*pass*/
        case TextureType::Texture1DArray:   return { desc.extent.width, 1u, 1u };
        case TextureType::Texture2D:        /*pass*/
        case TextureType::Texture2DArray:   /*pass*/
        case TextureType::TextureCube:      /*pass*/
        case TextureType::TextureCubeArray: /*pass*/
        case TextureType::Texture2DMS:      /*pass*/
        case TextureType::Texture2DMSArray: return { desc.extent.width, desc.extent.height, 1u };
        case TextureType::Texture3D:        return { desc.extent.width, desc.extent.height, desc.extent.depth };
    }
    throw std::invalid_argument("cannot determine texture extent for unknown texture type");
}

static std::uint32_t GetTextureLayertCount(const TextureDescriptor& desc)
{
    if (IsArrayTexture(desc.type))
        return desc.arrayLayers;
    else
        return 1;
}

Texture* VKRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    const auto& cfg = GetConfiguration();

    /* Determine size of image for staging buffer */
    const auto imageSize        = TextureSize(textureDesc);
    const auto initialDataSize  = static_cast<VkDeviceSize>(TextureBufferSize(textureDesc.format, imageSize));

    /* Set up initial image data */
    const void* initialData = nullptr;
    ByteBuffer tempImageBuffer;

    if (imageDesc)
    {
        /* Check if image data must be converted */
        ImageFormat dstFormat   = ImageFormat::RGBA;
        DataType    dstDataType = DataType::Int8;

        if (FindSuitableImageFormat(textureDesc.format, dstFormat, dstDataType))
        {
            /* Convert image format (will be null if no conversion is necessary) */
            tempImageBuffer = ConvertImageBuffer(*imageDesc, dstFormat, dstDataType, cfg.threadCount);
        }

        if (tempImageBuffer)
        {
            /*
            Validate that source image data was large enough so conversion is valid,
            then use temporary image buffer as source for initial data
            */
            const auto srcImageDataSize = imageSize * ImageFormatSize(imageDesc->format) * DataTypeSize(imageDesc->dataType);
            AssertImageDataSize(imageDesc->dataSize, static_cast<std::size_t>(srcImageDataSize));
            initialData = tempImageBuffer.get();
        }
        else
        {
            /*
            Validate that image data is large enough,
            then use input data as source for initial data
            */
            AssertImageDataSize(imageDesc->dataSize, static_cast<std::size_t>(initialDataSize));
            initialData = imageDesc->data;
        }
    }
    else if (cfg.imageInitialization.enabled)
    {
        /* Allocate default image data */
        ImageFormat imageFormat = ImageFormat::RGBA;
        DataType imageDataType = DataType::Float64;

        if (FindSuitableImageFormat(textureDesc.format, imageFormat, imageDataType))
        {
            const ColorRGBAd fillColor { cfg.imageInitialization.clearValue.color.Cast<double>() };
            tempImageBuffer = GenerateImageBuffer(imageFormat, imageDataType, imageSize, fillColor);
        }
        else
            tempImageBuffer = GenerateEmptyByteBuffer(static_cast<std::size_t>(initialDataSize));

        initialData = tempImageBuffer.get();
    }

    /* Create staging buffer */
    auto stagingCreateInfo = MakeVkBufferCreateInfo(
        initialDataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT  // <-- TODO: support read/write mapping //GetStagingVkBufferUsageFlags(desc.flags)
    );

    auto stagingBuffer = CreateStagingBuffer(stagingCreateInfo, initialData, initialDataSize);

    /* Create device texture */
    auto textureVK      = MakeUnique<VKTexture>(device_, *deviceMemoryMngr_, textureDesc);

    auto image          = textureVK->GetVkImage();
    auto mipLevels      = textureVK->GetNumMipLevels();
    auto arrayLayers    = textureVK->GetNumArrayLayers();

    /* Copy staging buffer into hardware texture, then transfer image into sampling-ready state */
    auto formatVK = VKTypes::Map(textureDesc.format);

    auto cmdBuffer = device_.AllocCommandBuffer();
    {
        device_.TransitionImageLayout(
            cmdBuffer,
            image,
            formatVK,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            mipLevels,
            arrayLayers
        );

        device_.CopyBufferToImage(
            cmdBuffer,
            stagingBuffer.GetVkBuffer(),
            image,
            GetTextureVkExtent(textureDesc),
            GetTextureLayertCount(textureDesc)
        );

        device_.TransitionImageLayout(
            cmdBuffer,
            image,
            formatVK,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            mipLevels,
            arrayLayers
        );
    }
    device_.FlushCommandBuffer(cmdBuffer);

    /* Release staging buffer */
    stagingBuffer.ReleaseMemoryRegion(*deviceMemoryMngr_);

    /* Create image view for texture */
    textureVK->CreateInternalImageView(device_);

    return TakeOwnership(textures_, std::move(textureVK));
}

void VKRenderSystem::Release(Texture& texture)
{
    /* Release device memory region, then release texture object */
    auto& textureVK = LLGL_CAST(VKTexture&, texture);
    deviceMemoryMngr_->Release(textureVK.GetMemoryRegion());
    RemoveFromUniqueSet(textures_, &texture);
}

void VKRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    //todo
}

void VKRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    //todo
}

void VKRenderSystem::GenerateMips(Texture& texture)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);
    auto cmdBuffer = device_.AllocCommandBuffer();
    {
        device_.GenerateMips(
            cmdBuffer,
            textureVK.GetVkImage(),
            textureVK.GetVkExtent(),
            0,
            textureVK.GetNumMipLevels(),
            0,
            textureVK.GetNumArrayLayers()
        );
    }
    device_.FlushCommandBuffer(cmdBuffer);
}

void VKRenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    auto& textureVK = LLGL_CAST(VKTexture&, texture);

    const auto maxNumMipLevels      = textureVK.GetNumMipLevels();
    const auto maxNumArrayLayers    = std::uint32_t(1u); //TODO...

    if (baseMipLevel < maxNumMipLevels && baseArrayLayer < maxNumArrayLayers && numMipLevels > 0 && numArrayLayers > 0)
    {
        auto cmdBuffer = device_.AllocCommandBuffer();
        {
            device_.GenerateMips(
                cmdBuffer,
                textureVK.GetVkImage(),
                textureVK.GetVkExtent(),
                baseMipLevel,
                std::min(numMipLevels, maxNumMipLevels - baseMipLevel),
                baseArrayLayer,
                std::min(numArrayLayers, maxNumArrayLayers - baseArrayLayer)
            );
        }
        device_.FlushCommandBuffer(cmdBuffer);
    }
}

/* ----- Sampler States ---- */

Sampler* VKRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<VKSampler>(device_, desc));
}

void VKRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* VKRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<VKResourceHeap>(device_, desc));
}

void VKRenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* VKRenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    AssertCreateRenderPass(desc);
    return TakeOwnership(renderPasses_, MakeUnique<VKRenderPass>(device_, desc));
}

void VKRenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* VKRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    AssertCreateRenderTarget(desc);
    return TakeOwnership(renderTargets_, MakeUnique<VKRenderTarget>(device_, *deviceMemoryMngr_, desc));
}

void VKRenderSystem::Release(RenderTarget& renderTarget)
{
    /* Release device memory region, then release texture object */
    auto& renderTargetVL = LLGL_CAST(VKRenderTarget&, renderTarget);
    renderTargetVL.ReleaseDeviceMemoryResources(*deviceMemoryMngr_);
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* VKRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    AssertCreateShader(desc);
    return TakeOwnership(shaders_, MakeUnique<VKShader>(device_, desc));
}

ShaderProgram* VKRenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    AssertCreateShaderProgram(desc);
    return TakeOwnership(shaderPrograms_, MakeUnique<VKShaderProgram>(desc));
}

void VKRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void VKRenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* VKRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<VKPipelineLayout>(device_, desc));
}

void VKRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* VKRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(
        graphicsPipelines_,
        MakeUnique<VKGraphicsPipeline>(
            device_,
            defaultPipelineLayout_,
            (!renderContexts_.empty() ? (*renderContexts_.begin())->GetRenderPass() : nullptr),
            desc,
            gfxPipelineLimits_
        )
    );
}

ComputePipeline* VKRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return TakeOwnership(computePipelines_, MakeUnique<VKComputePipeline>(device_, desc, defaultPipelineLayout_));
}

void VKRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void VKRenderSystem::Release(ComputePipeline& computePipeline)
{
    RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

QueryHeap* VKRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<VKQueryHeap>(device_, desc));
}

void VKRenderSystem::Release(QueryHeap& queryHeap)
{
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* VKRenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<VKFence>(device_));
}

void VKRenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
}


/*
 * ======= Private: =======
 */

void VKRenderSystem::CreateInstance(const ApplicationDescriptor* applicationDesc)
{
    /* Initialize application descriptor */
    VkApplicationInfo appInfo;

    appInfo.sType                   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext                   = nullptr;

    if (applicationDesc)
    {
        appInfo.pApplicationName    = applicationDesc->applicationName.c_str();
        appInfo.applicationVersion  = applicationDesc->applicationVersion;
        appInfo.pEngineName         = applicationDesc->engineName.c_str();
        appInfo.engineVersion       = applicationDesc->engineVersion;
    }
    else
    {
        appInfo.pApplicationName    = nullptr;
        appInfo.applicationVersion  = 0;
        appInfo.pEngineName         = nullptr;
        appInfo.engineVersion       = 0;
    }

    appInfo.apiVersion              = VK_API_VERSION_1_0;

    /* Query instance layer properties */
    auto layerProperties = VKQueryInstanceLayerProperties();
    std::vector<const char*> layerNames;

    for (const auto& prop : layerProperties)
    {
        if (IsLayerRequired(prop.layerName))
            layerNames.push_back(prop.layerName);
    }

    /* Query instance extension properties */
    auto extensionProperties = VKQueryInstanceExtensionProperties();
    std::vector<const char*> extensionNames;

    for (const auto& prop : extensionProperties)
    {
        if (IsExtensionRequired(prop.extensionName))
            extensionNames.push_back(prop.extensionName);
    }

    /* Setup Vulkan instance descriptor */
    VkInstanceCreateInfo instanceInfo;

    instanceInfo.sType                          = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext                          = nullptr;
    instanceInfo.flags                          = 0;
    instanceInfo.pApplicationInfo               = (&appInfo);

    if (layerNames.empty())
    {
        instanceInfo.enabledLayerCount          = 0;
        instanceInfo.ppEnabledLayerNames        = nullptr;
    }
    else
    {
        instanceInfo.enabledLayerCount          = static_cast<std::uint32_t>(layerNames.size());
        instanceInfo.ppEnabledLayerNames        = layerNames.data();
    }

    if (extensionNames.empty())
    {
        instanceInfo.enabledExtensionCount      = 0;
        instanceInfo.ppEnabledExtensionNames    = nullptr;
    }
    else
    {
        instanceInfo.enabledExtensionCount      = static_cast<std::uint32_t>(extensionNames.size());
        instanceInfo.ppEnabledExtensionNames    = extensionNames.data();
    }

    /* Create Vulkan instance */
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan instance");

    if (debugLayerEnabled_)
        CreateDebugReportCallback();
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
    //auto renderSystemVK = reinterpret_cast<VKRenderSystem*>(userData);
    Log::StdErr() << message << std::endl;
    return VK_FALSE;
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
    auto result = CreateDebugReportCallbackEXT(instance_, &createInfo, nullptr, debugReportCallback_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan debug report callback");
}

void VKRenderSystem::LoadExtensions()
{
    LoadAllExtensions(instance_);
}

void VKRenderSystem::PickPhysicalDevice()
{
    /* Pick physical device with Vulkan support */
    if (!physicalDevice_.PickPhysicalDevice(instance_))
        throw std::runtime_error("failed to find physical device with Vulkan support");

    /* Query and store rendering capabilities */
    RendererInfo info;
    RenderingCapabilities caps;

    physicalDevice_.QueryDeviceProperties(info, caps, gfxPipelineLimits_);

    SetRendererInfo(info);
    SetRenderingCaps(caps);
}

void VKRenderSystem::CreateLogicalDevice()
{
    /* Create logical device with all supported physical device feature */
    device_ = physicalDevice_.CreateLogicalDevice();

    /* Create command queue interface */
    commandQueue_ = MakeUnique<VKCommandQueue>(device_, device_.GetVkQueue());
}

void VKRenderSystem::CreateDefaultPipelineLayout()
{
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    {
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    }
    auto result = vkCreatePipelineLayout(device_, &layoutCreateInfo, nullptr, defaultPipelineLayout_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan default pipeline layout");
}

bool VKRenderSystem::IsLayerRequired(const std::string& name) const
{
    //TODO: make this statically optional
    #ifdef LLGL_DEBUG
    if (name == "VK_LAYER_LUNARG_core_validation")
        return true;
    #endif
    return false;
}

bool VKRenderSystem::IsExtensionRequired(const std::string& name) const
{
    return
    (
        name == VK_KHR_SURFACE_EXTENSION_NAME
        #ifdef LLGL_OS_WIN32
        || name == VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        #endif
        #ifdef LLGL_OS_LINUX
        || name == VK_KHR_XLIB_SURFACE_EXTENSION_NAME
        #endif
        || (debugLayerEnabled_ && name == VK_EXT_DEBUG_REPORT_EXTENSION_NAME)
    );
}

VKBuffer* VKRenderSystem::CreateGpuBuffer(const BufferDescriptor& desc, VkBufferUsageFlags usage)
{
    /* Create hardware buffer */
    VkBufferCreateInfo createInfo;

    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            InitVkBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKBuffer>(BufferType::Vertex, device_, createInfo));
        }
        break;

        case BufferType::Index:
        {
            InitVkBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKIndexBuffer>(device_, createInfo, desc.indexBuffer.format));
        }
        break;

        case BufferType::Constant:
        {
            InitVkBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKBuffer>(BufferType::Constant, device_, createInfo));
        }
        break;

        case BufferType::Storage:
        {
            InitVkBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKBuffer>(BufferType::Storage, device_, createInfo));
        }
        break;

        case BufferType::StreamOutput:
        {
            throw std::runtime_error("stream output buffer not supported by Vulkan renderer");
        }
        break;

        default:
        break;
    }
    return nullptr;
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

VKDeviceBuffer VKRenderSystem::CreateStagingBuffer(
    const VkBufferCreateInfo&   createInfo,
    const void*                 initialData,
    VkDeviceSize                initialDataSize)
{
    /* Allocate staging buffer */
    auto stagingBuffer = CreateStagingBuffer(createInfo);

    /* Copy initial data to buffer memory */
    if (initialData != nullptr && initialDataSize > 0)
        device_.WriteBuffer(stagingBuffer, initialData, initialDataSize);

    return stagingBuffer;
}


} // /namespace LLGL



// ================================================================================
