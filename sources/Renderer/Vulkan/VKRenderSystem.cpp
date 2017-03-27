/*
 * VKRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
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
    {
        appDesc.applicationName     = "LLGL-App";
        appDesc.applicationVersion  = 1;
        appDesc.engineName          = "LLGL";
        appDesc.engineVersion       = 1;
    }
    CreateInstance(appDesc);
}

VKRenderSystem::~VKRenderSystem()
{
}

/* ----- Render Context ----- */

RenderContext* VKRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return nullptr;//todo
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

std::vector<VkLayerProperties> VKRenderSystem::QueryInstanceLayerProperties()
{
    uint32_t propertiesCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&propertiesCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance layer properties");

    std::vector<VkLayerProperties> properties(propertiesCount);
    result = vkEnumerateInstanceLayerProperties(&propertiesCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance layer properties");

    return properties;
}

std::vector<VkExtensionProperties> VKRenderSystem::QueryInstanceExtensionProperties()
{
    uint32_t propertiesCount = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance extension properties");

    std::vector<VkExtensionProperties> properties(propertiesCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance extension properties");

    return properties;
}

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
    #ifdef LLGL_DEBUG
    
    auto layerProperties = QueryInstanceLayerProperties();
    
    std::vector<const char*> layerNames;
    layerNames.reserve(layerProperties.size());

    for (const auto& prop : layerProperties)
        layerNames.push_back(prop.layerName);

    #endif

    /* Query instance extension properties */
    auto extensionProperties = QueryInstanceExtensionProperties();

    std::vector<const char*> extensionNames;
    extensionNames.reserve(extensionProperties.size());

    for (const auto& prop : extensionProperties)
        extensionNames.push_back(prop.extensionName);

    /* Setup Vulkan instance descriptor */
    VkInstanceCreateInfo instanceInfo;
    
    instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext                      = nullptr;
    instanceInfo.flags                      = 0;
    instanceInfo.pApplicationInfo           = (&appInfo);

    #ifdef LLGL_DEBUG
    instanceInfo.enabledLayerCount          = static_cast<uint32_t>(layerNames.size());
    instanceInfo.ppEnabledLayerNames        = layerNames.data();
    #else
    instanceInfo.enabledLayerCount          = 0;
    instanceInfo.ppEnabledLayerNames        = nullptr;
    #endif

    instanceInfo.enabledExtensionCount      = static_cast<uint32_t>(extensionNames.size());
    instanceInfo.ppEnabledExtensionNames    = extensionNames.data();
    
    /* Create Vulkan instance */
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, instance_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan instance");
}


} // /namespace LLGL



// ================================================================================
