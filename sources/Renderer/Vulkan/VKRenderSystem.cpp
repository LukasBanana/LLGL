/*
 * VKRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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
#include <LLGL/Log.h>

#define TEST_VULKAN_MEMORY_MNGR
#ifdef TEST_VULKAN_MEMORY_MNGR
#   include <iostream>
#endif


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

static void FillBufferCreateInfo(VkBufferCreateInfo& createInfo, VkDeviceSize size, VkBufferUsageFlags usage)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.size                     = size;
    createInfo.usage                    = usage;
    createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount    = 0;
    createInfo.pQueueFamilyIndices      = nullptr;
}


/* ----- Common ----- */

static const std::vector<const char*> g_deviceExtensions
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VKRenderSystem::VKRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    instance_            { vkDestroyInstance                        },
    device_              { vkDestroyDevice                          },
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

    if (!PickPhysicalDevice())
        throw std::runtime_error("failed to find physical device with Vulkan support");

    QueryDeviceProperties();
    CreateLogicalDevice();
    CreateStagingCommandResources();

    /* Create device memory manager */
    deviceMemoryMngr_ = MakeUnique<VKDeviceMemoryManager>(
        device_,
        memoryProperties_,
        (rendererConfigVK != nullptr ? rendererConfigVK->minDeviceMemoryAllocationSize : 1024*1024),
        (rendererConfigVK != nullptr ? rendererConfigVK->reduceDeviceMemoryFragmentation : false)
    );

    #if defined TEST_VULKAN_MEMORY_MNGR && 0

    auto& mngr = *deviceMemoryMngr_;

    std::uint32_t typeBits = 1665;
    VkDeviceSize alignment = 1;
    
    auto reg0 = mngr.Allocate(6, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto reg1 = mngr.Allocate(7, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto reg2 = mngr.Allocate(12, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto reg3 = mngr.Allocate(5, 16, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto reg4 = mngr.Allocate(5, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mngr.PrintBlocks(std::cout, "Allocate: 6, 7, 12, 5 (alignment 16), 5");
    std::cout << std::endl;

    mngr.Release(reg1);
    mngr.PrintBlocks(std::cout, "Release second allocation (7)");
    std::cout << std::endl;

    reg1 = mngr.Allocate(3, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mngr.PrintBlocks(std::cout, "Allocate: 3");
    std::cout << std::endl;

    auto reg5 = mngr.Allocate(4, alignment, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mngr.PrintBlocks(std::cout, "Allocate: 4");
    std::cout << std::endl;

    mngr.Release(reg1);
    mngr.PrintBlocks(std::cout, "Release previous 3");
    std::cout << std::endl;

    mngr.Release(reg2);
    mngr.PrintBlocks(std::cout, "Release previous 12");
    std::cout << std::endl;

    mngr.Release(reg5);
    mngr.Release(reg4);
    mngr.PrintBlocks(std::cout, "Release previous 4, 5");
    std::cout << std::endl;

    reg5 = mngr.Allocate(9, 8, typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mngr.PrintBlocks(std::cout, "Allocate: 9 with alignment 8");
    std::cout << std::endl;

    #endif
}

VKRenderSystem::~VKRenderSystem()
{
    /* Release resource and wait until device becomes idle */
    ReleaseStagingCommandResources();
    vkDeviceWaitIdle(device_);
}

/* ----- Render Context ----- */

RenderContext* VKRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(renderContexts_, MakeUnique<VKRenderContext>(instance_, physicalDevice_, device_, desc, surface));
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

CommandBuffer* VKRenderSystem::CreateCommandBuffer()
{
    auto mainContext = renderContexts_.begin()->get();
    return TakeOwnership(
        commandBuffers_,
        MakeUnique<VKCommandBuffer>(device_, mainContext->GetSwapChainSize(), queueFamilyIndices_)
    );
}

void VKRenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

Buffer* VKRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<VkDeviceSize>::max()));

    /* Create staging buffer */
    VkBufferCreateInfo stagingCreateInfo;
    FillBufferCreateInfo(stagingCreateInfo, static_cast<VkDeviceSize>(desc.size), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VKBufferWithRequirements stagingBuffer { device_ };
    stagingBuffer.Create(device_, stagingCreateInfo);

    /* Allocate statging device memory */
    auto memoryRegionStaging = deviceMemoryMngr_->Allocate(
        stagingBuffer.requirements.size,
        stagingBuffer.requirements.alignment,
        stagingBuffer.requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    memoryRegionStaging->BindBuffer(device_, stagingBuffer.buffer);

    /* Copy initial data to buffer memory */
    if (initialData != nullptr)
    {
        auto stagingDeviceMemory = memoryRegionStaging->GetParentChunk();

        if (auto memory = stagingDeviceMemory->Map(device_, memoryRegionStaging->GetOffset(), static_cast<VkDeviceSize>(desc.size)))
        {
            ::memcpy(memory, initialData, static_cast<size_t>(desc.size));
            stagingDeviceMemory->Unmap(device_);
        }
    }

    /* Create device buffer */
    auto buffer = CreateHardwareBuffer(desc, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    
    /* Allocate device memory */
    const auto& requirements = buffer->GetRequirements();

    auto memoryRegion = deviceMemoryMngr_->Allocate(
        requirements.size,
        requirements.alignment,
        requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    buffer->BindToMemory(device_, memoryRegion);

    /* Copy staging buffer into hardware buffer */
    CopyBuffer(stagingBuffer.buffer, buffer->GetVkBuffer(), static_cast<VkDeviceSize>(desc.size));

    if ((desc.flags & BufferFlags::MapReadWriteAccess) != 0)
    {
        /* Store ownership of staging buffer */
        buffer->TakeStagingBuffer(std::move(stagingBuffer), memoryRegionStaging);
    }
    else
    {
        /* Release staging buffer */
        deviceMemoryMngr_->Release(memoryRegionStaging);
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
    //todo
}

void VKRenderSystem::Release(BufferArray& bufferArray)
{
    //todo
}

void VKRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void* VKRenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    return nullptr;//todo
}

void VKRenderSystem::UnmapBuffer(Buffer& buffer)
{
    //todo
}

/* ----- Textures ----- */

Texture* VKRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    return nullptr;//todo
}

TextureArray* VKRenderSystem::CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(Texture& texture)
{
    //todo
}

void VKRenderSystem::Release(TextureArray& textureArray)
{
    //todo
}

void VKRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    //todo
}

void VKRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, ImageFormat imageFormat, DataType dataType, void* data, std::size_t dataSize)
{
    //todo
}

void VKRenderSystem::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ---- */

Sampler* VKRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<VKSampler>(device_, desc));
}

SamplerArray* VKRenderSystem::CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray)
{
    return TakeOwnership(samplerArrays_, MakeUnique<VKSamplerArray>(numSamplers, samplerArray));
}

void VKRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

void VKRenderSystem::Release(SamplerArray& samplerArray)
{
    RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Resource Views ----- */

ResourceViewHeap* VKRenderSystem::CreateResourceViewHeap(const ResourceViewHeapDescriptor& desc)
{
    return TakeOwnership(resourceViewHeaps_, MakeUnique<VKResourceViewHeap>(device_, desc));
}

void VKRenderSystem::Release(ResourceViewHeap& resourceViewHeap)
{
    RemoveFromUniqueSet(resourceViewHeaps_, &resourceViewHeap);
}

/* ----- Render Targets ----- */

RenderTarget* VKRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(RenderTarget& renderTarget)
{
    //todo
}

/* ----- Shader ----- */

Shader* VKRenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<VKShader>(device_, type));
}

ShaderProgram* VKRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<VKShaderProgram>());
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
    if (renderContexts_.empty())
        throw std::runtime_error("cannot create graphics pipeline without a render context");

    auto renderContext = renderContexts_.begin()->get();
    auto renderPassVK = renderContext->GetSwapChainRenderPass().Get();

    return TakeOwnership(graphicsPipelines_, MakeUnique<VKGraphicsPipeline>(device_, renderPassVK, desc, renderContext->GetSwapChainExtent()));
}

ComputePipeline* VKRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void VKRenderSystem::Release(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

Query* VKRenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return TakeOwnership(queries_, MakeUnique<VKQuery>(device_, desc));
}

void VKRenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
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
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t messageCode,
    const char* layerPrefix, const char* message, void* userData)
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

bool VKRenderSystem::PickPhysicalDevice()
{
    /* Query all physical devices and pick suitable */
    auto devices = VKQueryPhysicalDevices(instance_);

    for (const auto& dev : devices)
    {
        if (IsPhysicalDeviceSuitable(dev))
        {
            physicalDevice_ = dev;
            return true;
        }
    }

    return false;
}

void VKRenderSystem::QueryDeviceProperties()
{
    /* Query physical device features and memory propertiers */
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
    vkGetPhysicalDeviceFeatures(physicalDevice_, &features_);

    /* Query properties of selected physical device */
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

    /* Map properties to output renderer info */
    RendererInfo info;

    info.rendererName           = "Vulkan " + VKApiVersionToString(properties.apiVersion);
    info.deviceName             = std::string(properties.deviceName);
    info.vendorName             = GetVendorByID(properties.vendorID);
    info.shadingLanguageName    = "SPIR-V";

    SetRendererInfo(info);

    /* Map limits to output rendering capabilites */
    const auto& limits = properties.limits;

    RenderingCaps caps;
    {
        caps.screenOrigin                       = ScreenOrigin::UpperLeft;
        caps.clippingRange                      = ClippingRange::ZeroToOne;
        caps.shadingLanguages                   = { ShadingLanguage::SPIRV, ShadingLanguage::SPIRV_100 };
        caps.hasRenderTargets                   = true;
        caps.has3DTextures                      = true;
        caps.hasCubeTextures                    = true;
        caps.hasTextureArrays                   = true;
        caps.hasCubeTextureArrays               = (features_.imageCubeArray != VK_FALSE);
        caps.hasMultiSampleTextures             = true;
        caps.hasSamplers                        = true;
        caps.hasConstantBuffers                 = true;
        caps.hasStorageBuffers                  = true;
        caps.hasUniforms                        = true;
        caps.hasGeometryShaders                 = (features_.geometryShader != VK_FALSE);
        caps.hasTessellationShaders             = (features_.tessellationShader != VK_FALSE);
        caps.hasComputeShaders                  = true;
        caps.hasInstancing                      = true;
        caps.hasOffsetInstancing                = true;
        caps.hasViewportArrays                  = (features_.multiViewport != VK_FALSE);
        caps.hasConservativeRasterization       = false;
        caps.hasStreamOutputs                   = true;
        caps.maxNumTextureArrayLayers           = limits.maxImageArrayLayers;
        caps.maxNumRenderTargetAttachments      = static_cast<std::uint32_t>(limits.framebufferColorSampleCounts);
        caps.maxConstantBufferSize              = 0; //???
        caps.maxPatchVertices                   = limits.maxTessellationPatchSize;
        caps.max1DTextureSize                   = limits.maxImageDimension1D;
        caps.max2DTextureSize                   = limits.maxImageDimension2D;
        caps.max3DTextureSize                   = limits.maxImageDimension3D;
        caps.maxCubeTextureSize                 = limits.maxImageDimensionCube;
        caps.maxAnisotropy                      = static_cast<std::uint32_t>(limits.maxSamplerAnisotropy);
        caps.maxNumComputeShaderWorkGroups[0]   = limits.maxComputeWorkGroupCount[0];
        caps.maxNumComputeShaderWorkGroups[1]   = limits.maxComputeWorkGroupCount[1];
        caps.maxNumComputeShaderWorkGroups[2]   = limits.maxComputeWorkGroupCount[2];
        caps.maxComputeShaderWorkGroupSize[0]   = limits.maxComputeWorkGroupSize[0];
        caps.maxComputeShaderWorkGroupSize[1]   = limits.maxComputeWorkGroupSize[1];
        caps.maxComputeShaderWorkGroupSize[2]   = limits.maxComputeWorkGroupSize[2];
        caps.maxNumViewports                    = limits.maxViewports;
        caps.maxViewportSize[0]                 = limits.maxViewportDimensions[0];
        caps.maxViewportSize[1]                 = limits.maxViewportDimensions[1];
    }
    SetRenderingCaps(caps);
}

// Device-only layers are deprecated -> set 'enabledLayerCount' and 'ppEnabledLayerNames' members to zero during device creation.
// see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#extended-functionality-device-layer-deprecation
void VKRenderSystem::CreateLogicalDevice()
{
    /* Initialize queue create description */
    queueFamilyIndices_ = VKFindQueueFamilies(physicalDevice_, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = { queueFamilyIndices_.graphicsFamily, queueFamilyIndices_.presentFamily };

    float queuePriority = 1.0f;
    for (auto family : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo info;
        {
            info.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.pNext              = nullptr;
            info.flags              = 0;
            info.queueFamilyIndex   = family;
            info.queueCount         = 1;
            info.pQueuePriorities   = &queuePriority;
        }
        queueCreateInfos.push_back(info);
    }

    /* Create logical device */
    VkDeviceCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.queueCreateInfoCount     = static_cast<std::uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos        = queueCreateInfos.data();
        createInfo.enabledLayerCount        = 0;
        createInfo.ppEnabledLayerNames      = nullptr;
        createInfo.enabledExtensionCount    = static_cast<std::uint32_t>(g_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames  = g_deviceExtensions.data();
        createInfo.pEnabledFeatures         = &features_;
    }
    VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, device_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan logical device");

    /* Query device graphics queue */
    vkGetDeviceQueue(device_, queueFamilyIndices_.graphicsFamily, 0, &graphicsQueue_);

    /* Create command queue interface */
    commandQueue_ = MakeUnique<VKCommandQueue>(device_, graphicsQueue_);
}

void VKRenderSystem::CreateStagingCommandResources()
{
    /* Create staging command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndices_.graphicsFamily;
    }
    auto result = vkCreateCommandPool(device_, &createInfo, nullptr, stagingCommandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool for staging buffers");

    /* Allocate staging command buffer */
    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.commandPool           = stagingCommandPool_;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount    = 1;
    }
    result = vkAllocateCommandBuffers(device_, &allocInfo, &stagingCommandBuffer_);
    VKThrowIfFailed(result, "failed to create Vulkan command buffer for staging buffers");
}

void VKRenderSystem::ReleaseStagingCommandResources()
{
    /* Release staging command buffer */
    vkFreeCommandBuffers(device_, stagingCommandPool_, 1, &stagingCommandBuffer_);
    stagingCommandBuffer_ = VK_NULL_HANDLE;
}

bool VKRenderSystem::IsLayerRequired(const std::string& name) const
{
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

bool VKRenderSystem::IsPhysicalDeviceSuitable(VkPhysicalDevice device) const
{
    if (CheckDeviceExtensionSupport(device, g_deviceExtensions))
    {
        //TODO...
        return true;
    }
    return false;
}

bool VKRenderSystem::CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensionNames) const
{
    /* Check if device supports all required extensions */
    auto availableExtensions = VKQueryDeviceExtensionProperties(device);

    std::set<std::string> requiredExtensions(extensionNames.begin(), extensionNames.end());

    for (const auto& ext : availableExtensions)
        requiredExtensions.erase(ext.extensionName);

    return requiredExtensions.empty();
}

std::uint32_t VKRenderSystem::FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const
{
    return VKFindMemoryType(memoryProperties_, memoryTypeBits, properties);
}

VKBuffer* VKRenderSystem::CreateHardwareBuffer(const BufferDescriptor& desc, VkBufferUsageFlags usage)
{
    /* Create hardware buffer */
    VkBufferCreateInfo createInfo;

    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            FillBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKBuffer>(BufferType::Vertex, device_, createInfo));
        }
        break;

        case BufferType::Index:
        {
            FillBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKIndexBuffer>(device_, createInfo, desc.indexBuffer.format));
        }
        break;

        case BufferType::Constant:
        {
            FillBufferCreateInfo(createInfo, static_cast<VkDeviceSize>(desc.size), (usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
            return TakeOwnership(buffers_, MakeUnique<VKBuffer>(BufferType::Constant, device_, createInfo));
        }
        break;

        default:
        break;
    }
    return nullptr;
}

void VKRenderSystem::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    /* Begin command buffer record */
    VkCommandBufferBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo  = nullptr;
    }
    auto result = vkBeginCommandBuffer(stagingCommandBuffer_, &beginInfo);
    VKThrowIfFailed(result, "failed to begin recording Vulkan command buffer");

    /* Record copy command */
    VkBufferCopy copyRegion;
    {
        copyRegion.srcOffset    = 0;
        copyRegion.dstOffset    = 0;
        copyRegion.size         = size;
    }
    vkCmdCopyBuffer(stagingCommandBuffer_, srcBuffer, dstBuffer, 1, &copyRegion);

    /* End command buffer record */
    vkEndCommandBuffer(stagingCommandBuffer_);

    /* Submit command buffer to queue */
    VkSubmitInfo submitInfo;
    {
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = 0;
        submitInfo.pWaitSemaphores      = nullptr;
        submitInfo.pWaitDstStageMask    = nullptr;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = (&stagingCommandBuffer_);
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores    = nullptr;
    }
    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue_);
}


} // /namespace LLGL



// ================================================================================
