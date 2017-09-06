/*
 * VKRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Platform/Platform.h>
#include "VKRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
#include "../GLCommon/GLTypes.h"
#include "VKCore.h"


namespace LLGL
{


/* ----- Common ----- */

VKRenderSystem::VKRenderSystem() :
    instance_ { vkDestroyInstance }
{
    //TODO: get application descriptor from client programmer
    ApplicationDescriptor appDesc;
    appDesc.applicationName     = "LLGL-App";
    appDesc.applicationVersion  = 1;
    appDesc.engineName          = "LLGL";
    appDesc.engineVersion       = 1;

    CreateInstance(appDesc);
    PickPhysicalDevice();
    QueryDeviceProperties();
}

VKRenderSystem::~VKRenderSystem()
{
}

/* ----- Render Context ----- */

RenderContext* VKRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(renderContexts_, MakeUnique<VKRenderContext>(instance_, desc, surface));
}

void VKRenderSystem::Release(RenderContext& renderContext)
{
    //todo
}

/* ----- Command buffers ----- */

CommandBuffer* VKRenderSystem::CreateCommandBuffer()
{
    return nullptr;//todo
}

void VKRenderSystem::Release(CommandBuffer& commandBuffer)
{
    //todo
}

/* ----- Buffers ------ */

Buffer* VKRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    return nullptr;//todo
}

BufferArray* VKRenderSystem::CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    return nullptr;//todo
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

TextureArray* VKRenderSystem::CreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
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

TextureDescriptor VKRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    return {};//todo
}

void VKRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    //todo
}

void VKRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
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
    return nullptr;//todo
}

SamplerArray* VKRenderSystem::CreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(Sampler& sampler)
{
    //todo
}

void VKRenderSystem::Release(SamplerArray& samplerArray)
{
    //todo
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
    return nullptr;//todo
}

ShaderProgram* VKRenderSystem::CreateShaderProgram()
{
    return nullptr;//todo
}

void VKRenderSystem::Release(Shader& shader)
{
    //todo
}

void VKRenderSystem::Release(ShaderProgram& shaderProgram)
{
    //todo
}

/* ----- Pipeline States ----- */

GraphicsPipeline* VKRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return nullptr;//todo
}

ComputePipeline* VKRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    //todo
}

void VKRenderSystem::Release(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

Query* VKRenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return nullptr;//todo
}

void VKRenderSystem::Release(Query& query)
{
    //todo
}


/*
 * ======= Private: =======
 */

void VKRenderSystem::CreateInstance(const ApplicationDescriptor& appDesc)
{
    /* Setup application descriptor */
    VkApplicationInfo appInfo;
    
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext               = nullptr;
    appInfo.pApplicationName    = appDesc.applicationName.c_str();
    appInfo.applicationVersion  = appDesc.applicationVersion;
    appInfo.pEngineName         = appDesc.engineName.c_str();
    appInfo.engineVersion       = appDesc.engineVersion;
    appInfo.apiVersion          = VK_API_VERSION_1_0;

    /* Query instance layer properties */
    auto layerProperties = QueryInstanceLayerProperties();
    std::vector<const char*> layerNames;

    for (const auto& prop : layerProperties)
    {
        if (IsLayerRequired(prop.layerName))
            layerNames.push_back(prop.layerName);
    }

    /* Query instance extension properties */
    auto extensionProperties = QueryInstanceExtensionProperties();
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
        instanceInfo.enabledLayerCount          = static_cast<uint32_t>(layerNames.size());
        instanceInfo.ppEnabledLayerNames        = layerNames.data();
    }

    if (extensionNames.empty())
    {
        instanceInfo.enabledExtensionCount      = 0;
        instanceInfo.ppEnabledExtensionNames    = nullptr;
    }
    else
    {
        instanceInfo.enabledExtensionCount      = static_cast<uint32_t>(extensionNames.size());
        instanceInfo.ppEnabledExtensionNames    = extensionNames.data();
    }

    /* Create Vulkan instance */
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan instance");
}

void VKRenderSystem::PickPhysicalDevice()
{
    /* Query all physical devices and pick suitable */
    auto devices = QueryPhysicalDevices();

    for (const auto& dev : devices)
    {
        if (IsPhysicalDeviceSuitable(dev))
        {
            physicalDevice_ = dev;
            break;
        }
    }
}

std::vector<VkLayerProperties> VKRenderSystem::QueryInstanceLayerProperties()
{
    uint32_t propertyCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance layer properties");

    std::vector<VkLayerProperties> properties(propertyCount);
    result = vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance layer properties");

    return properties;
}

std::vector<VkExtensionProperties> VKRenderSystem::QueryInstanceExtensionProperties()
{
    uint32_t propertyCount = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance extension properties");

    std::vector<VkExtensionProperties> properties(propertyCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance extension properties");

    return properties;
}

std::vector<VkPhysicalDevice> VKRenderSystem::QueryPhysicalDevices()
{
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan physical devices");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    VKThrowIfFailed(result, "failed to query Vulkan physical devices");

    return devices;
}

std::vector<VkQueueFamilyProperties> VKRenderSystem::QueryQueueFamilyProperties(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

void VKRenderSystem::QueryDeviceProperties()
{
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

    caps.screenOrigin                   = ScreenOrigin::UpperLeft;
    caps.clippingRange                  = ClippingRange::ZeroToOne;
    caps.shadingLanguage                = ShadingLanguage::SPIRV_100;
    caps.hasRenderTargets               = true;
    caps.has3DTextures                  = true;
    caps.hasCubeTextures                = true;
    caps.hasTextureArrays               = true;
    caps.hasCubeTextureArrays           = true;
    caps.hasMultiSampleTextures         = true;
    caps.hasSamplers                    = true;
    caps.hasConstantBuffers             = true;
    caps.hasStorageBuffers              = true;
    caps.hasUniforms                    = true;
    caps.hasGeometryShaders             = true;
    caps.hasTessellationShaders         = true;
    caps.hasComputeShaders              = true;
    caps.hasInstancing                  = true;
    caps.hasOffsetInstancing            = true;
    caps.hasViewportArrays              = true;
    caps.hasConservativeRasterization   = true;
    caps.hasStreamOutputs               = true;
    caps.maxNumTextureArrayLayers       = limits.maxImageArrayLayers;
    caps.maxNumRenderTargetAttachments  = static_cast<std::uint32_t>(limits.framebufferColorSampleCounts);
    caps.maxConstantBufferSize          = 0; //???
    caps.maxPatchVertices               = limits.maxTessellationPatchSize;
    caps.max1DTextureSize               = limits.maxImageDimension1D;
    caps.max2DTextureSize               = limits.maxImageDimension2D;
    caps.max3DTextureSize               = limits.maxImageDimension3D;
    caps.maxCubeTextureSize             = limits.maxImageDimensionCube;
    caps.maxAnisotropy                  = static_cast<int>(limits.maxSamplerAnisotropy);
    caps.maxNumComputeShaderWorkGroups  = { limits.maxComputeWorkGroupCount[0], limits.maxComputeWorkGroupCount[1], limits.maxComputeWorkGroupCount[2] };
    caps.maxComputeShaderWorkGroupSize  = { limits.maxComputeWorkGroupSize[0], limits.maxComputeWorkGroupSize[1], limits.maxComputeWorkGroupSize[2] };

    SetRenderingCaps(caps);
}

VKRenderSystem::QueueFamilyIndices VKRenderSystem::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, const VkQueueFlags flags)
{
    QueueFamilyIndices indices;

    auto queueFamilies = QueryQueueFamilyProperties(device);

    int i = 0;
    for (const auto& family : queueFamilies)
    {
        /* Get graphics family index */
        if (family.queueCount > 0 && (family.queueFlags & flags) != 0)
            indices.graphicsFamily = i;

        /* Get present family index */
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (family.queueCount > 0 && presentSupport)
            indices.presentFamily = i;

        /* Check if queue family is complete */
        if (indices.Complete())
            break;

        ++i;
    }

    return indices;
}

bool VKRenderSystem::IsLayerRequired(const std::string& name) const
{
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
    );
}

bool VKRenderSystem::IsPhysicalDeviceSuitable(VkPhysicalDevice device) const
{
    /*VkPhysicalDeviceFeatures features;
    InitMemory(features);
    vkGetPhysicalDeviceFeatures(device, &features);*/

    //auto indices = FindQueueFamilies(device, VK_QUEUE_GRAPHICS_BIT);


    return true;
}


} // /namespace LLGL



// ================================================================================
