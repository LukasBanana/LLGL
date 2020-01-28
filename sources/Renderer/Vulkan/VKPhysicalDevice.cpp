/*
 * VKDevicePhysical.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPhysicalDevice.h"
#include "Ext/VKExtensionRegistry.h"
#include "VKCore.h"
#include "RenderState/VKGraphicsPSO.h"
#include "../../Core/Vendor.h"
#include <string>
#include <cstring>
#include <set>


namespace LLGL
{


static const char* g_requiredVulkanExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    nullptr,
};

static bool CheckDeviceExtensionSupport(
    VkPhysicalDevice                    physicalDevice,
    const char* const*                  requiredExtensions,
    std::size_t                         numRequiredExtensions,
    std::vector<VkExtensionProperties>& supportedExtensions)
{
    /* Check if device supports all required extensions */
    supportedExtensions = VKQueryDeviceExtensionProperties(physicalDevice);
    std::set<std::string> unsupported(requiredExtensions, requiredExtensions + numRequiredExtensions);

    for (const auto& ext : supportedExtensions)
    {
        if (unsupported.empty())
            break;
        else
            unsupported.erase(ext.extensionName);
    }

    /* No required extensions must remain */
    return unsupported.empty();
}

static bool IsPhysicalDeviceSuitable(
    VkPhysicalDevice                    physicalDevice,
    std::vector<VkExtensionProperties>& supportedExtensions)
{
    /* Check if physical devices supports at least these extensions */
    std::vector<VkExtensionProperties> extensions;
    bool suitable = CheckDeviceExtensionSupport(
        physicalDevice,
        g_requiredVulkanExtensions,
        (sizeof(g_requiredVulkanExtensions) / sizeof(g_requiredVulkanExtensions[0]) - 1),
        extensions
    );

    if (suitable)
    {
        /* Store all supported extensions */
        supportedExtensions = std::move(extensions);
        return true;
    }

    return false;
}

bool VKPhysicalDevice::PickPhysicalDevice(VkInstance instance)
{
    /* Query all physical devices and pick suitable */
    auto physicalDevices = VKQueryPhysicalDevices(instance);

    for (const auto& device : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(device, supportedExtensions_))
        {
            /* Store reference to all extension names */
            for (const auto& extension : supportedExtensions_)
                supportedExtensionNames_.insert(extension.extensionName);

            if (!EnableExtensions(g_requiredVulkanExtensions, true))
            {
                /* Stop considering this physical device, because some required extensions are not supported */
                supportedExtensionNames_.clear();
                continue;
            }

            EnableExtensions(GetOptionalExtensions());

            /* Store device and store properties */
            physicalDevice_ = device;
            QueryDeviceInfo();

            return true;
        }
    }

    return false;
}

static std::vector<Format> GetDefaultSupportedVKTextureFormats()
{
    return
    {
        Format::A8UNorm,
        Format::R8UNorm,            Format::R8SNorm,            Format::R8UInt,             Format::R8SInt,
        Format::R16UNorm,           Format::R16SNorm,           Format::R16UInt,            Format::R16SInt,            Format::R16Float,
        Format::R32UInt,            Format::R32SInt,            Format::R32Float,
        Format::R64Float,
        Format::RG8UNorm,           Format::RG8SNorm,           Format::RG8UInt,            Format::RG8SInt,
        Format::RG16UNorm,          Format::RG16SNorm,          Format::RG16UInt,           Format::RG16SInt,           Format::RG16Float,
        Format::RG32UInt,           Format::RG32SInt,           Format::RG32Float,
        Format::RG64Float,
        Format::RGB8UNorm,          Format::RGB8UNorm_sRGB,     Format::RGB8SNorm,          Format::RGB8UInt,           Format::RGB8SInt,
        Format::RGB16UNorm,         Format::RGB16SNorm,         Format::RGB16UInt,          Format::RGB16SInt,          Format::RGB16Float,
        Format::RGB32UInt,          Format::RGB32SInt,          Format::RGB32Float,
        Format::RGB64Float,
        Format::RGBA8UNorm,         Format::RGBA8UNorm_sRGB,    Format::RGBA8SNorm,         Format::RGBA8UInt,          Format::RGBA8SInt,
        Format::RGBA16UNorm,        Format::RGBA16SNorm,        Format::RGBA16UInt,         Format::RGBA16SInt,         Format::RGBA16Float,
        Format::RGBA32UInt,         Format::RGBA32SInt,         Format::RGBA32Float,
        Format::RGBA64Float,
        Format::BGRA8UNorm,         Format::BGRA8UNorm_sRGB,    Format::BGRA8SNorm,         Format::BGRA8UInt,          Format::BGRA8SInt,
        Format::RGB10A2UNorm,       Format::RGB10A2UInt,        Format::RG11B10Float,       Format::RGB9E5Float,
        Format::D16UNorm,           Format::D24UNormS8UInt,     Format::D32Float,           Format::D32FloatS8X24UInt,
    };
}

static std::initializer_list<Format> GetCompressedVKTextureFormatsS3TC()
{
    return
    {
        Format::BC1UNorm, Format::BC1UNorm_sRGB,
        Format::BC2UNorm, Format::BC2UNorm_sRGB,
        Format::BC3UNorm, Format::BC3UNorm_sRGB,
        Format::BC4UNorm, Format::BC4SNorm,
        Format::BC5UNorm, Format::BC5SNorm,
    };
}

void VKPhysicalDevice::QueryDeviceProperties(
    RendererInfo&               info,
    RenderingCapabilities&      caps,
    VKGraphicsPipelineLimits&   pipelineLimits)
{
    /* Map properties to output renderer info */
    info.rendererName           = ("Vulkan " + VKApiVersionToString(properties_.apiVersion));
    info.deviceName             = properties_.deviceName;
    info.vendorName             = GetVendorByID(properties_.vendorID);
    info.shadingLanguageName    = "SPIR-V";

    /* Map limits to output rendering capabilites */
    const auto& limits = properties_.limits;

    /* Query common attributes */
    caps.screenOrigin                               = ScreenOrigin::UpperLeft;
    caps.clippingRange                              = ClippingRange::ZeroToOne;
    caps.shadingLanguages                           = { ShadingLanguage::SPIRV, ShadingLanguage::SPIRV_100 };
    caps.textureFormats                             = GetDefaultSupportedVKTextureFormats();

    if (features_.textureCompressionBC != VK_FALSE)
        caps.textureFormats.insert(caps.textureFormats.end(), GetCompressedVKTextureFormatsS3TC());

    /* Query features */
    caps.features.hasRenderTargets                  = true;
    caps.features.has3DTextures                     = true;
    caps.features.hasCubeTextures                   = true;
    caps.features.hasArrayTextures                  = true;
    caps.features.hasCubeArrayTextures              = (features_.imageCubeArray != VK_FALSE);
    caps.features.hasMultiSampleTextures            = true;
    caps.features.hasTextureViews                   = true;
    caps.features.hasTextureViewSwizzle             = true;
    caps.features.hasBufferViews                    = true;
    caps.features.hasSamplers                       = true;
    caps.features.hasConstantBuffers                = true;
    caps.features.hasStorageBuffers                 = true;
    caps.features.hasUniforms                       = true;
    caps.features.hasGeometryShaders                = (features_.geometryShader != VK_FALSE);
    caps.features.hasTessellationShaders            = (features_.tessellationShader != VK_FALSE);
    caps.features.hasTessellatorStage               = caps.features.hasTessellationShaders;
    caps.features.hasComputeShaders                 = true;
    caps.features.hasInstancing                     = true;
    caps.features.hasOffsetInstancing               = true;
    caps.features.hasIndirectDrawing                = (features_.drawIndirectFirstInstance != VK_FALSE);
    caps.features.hasViewportArrays                 = (features_.multiViewport != VK_FALSE);
    caps.features.hasConservativeRasterization      = SupportsExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    caps.features.hasStreamOutputs                  = SupportsExtension(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    caps.features.hasLogicOp                        = (features_.logicOp != VK_FALSE);
    caps.features.hasPipelineStatistics             = (features_.pipelineStatisticsQuery != VK_FALSE);
    caps.features.hasRenderCondition                = SupportsExtension(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

    /* Query limits */
    caps.limits.lineWidthRange[0]                   = limits.lineWidthRange[0];
    caps.limits.lineWidthRange[1]                   = limits.lineWidthRange[1];
    caps.limits.maxTextureArrayLayers               = limits.maxImageArrayLayers;
    caps.limits.maxColorAttachments                 = limits.maxColorAttachments;
    caps.limits.maxPatchVertices                    = limits.maxTessellationPatchSize;
    caps.limits.max1DTextureSize                    = limits.maxImageDimension1D;
    caps.limits.max2DTextureSize                    = limits.maxImageDimension2D;
    caps.limits.max3DTextureSize                    = limits.maxImageDimension3D;
    caps.limits.maxCubeTextureSize                  = limits.maxImageDimensionCube;
    caps.limits.maxAnisotropy                       = static_cast<std::uint32_t>(limits.maxSamplerAnisotropy);
    caps.limits.maxComputeShaderWorkGroups[0]       = limits.maxComputeWorkGroupCount[0];
    caps.limits.maxComputeShaderWorkGroups[1]       = limits.maxComputeWorkGroupCount[1];
    caps.limits.maxComputeShaderWorkGroups[2]       = limits.maxComputeWorkGroupCount[2];
    caps.limits.maxComputeShaderWorkGroupSize[0]    = limits.maxComputeWorkGroupSize[0];
    caps.limits.maxComputeShaderWorkGroupSize[1]    = limits.maxComputeWorkGroupSize[1];
    caps.limits.maxComputeShaderWorkGroupSize[2]    = limits.maxComputeWorkGroupSize[2];
    caps.limits.maxViewports                        = limits.maxViewports;
    caps.limits.maxViewportSize[0]                  = limits.maxViewportDimensions[0];
    caps.limits.maxViewportSize[1]                  = limits.maxViewportDimensions[1];
    caps.limits.maxBufferSize                       = std::numeric_limits<VkDeviceSize>::max();
    caps.limits.maxConstantBufferSize               = limits.maxUniformBufferRange;
    caps.limits.maxStreamOutputs                    = 0; //TODO
    caps.limits.maxTessFactor                       = limits.maxTessellationGenerationLevel;
    caps.limits.minConstantBufferAlignment          = limits.minUniformBufferOffsetAlignment;
    caps.limits.minSampledBufferAlignment           = limits.minStorageBufferOffsetAlignment; // Use SSBO for both sampled and storage buffers
    caps.limits.minStorageBufferAlignment           = limits.minStorageBufferOffsetAlignment;

    /* Store graphics pipeline spcific limitations */
    pipelineLimits.lineWidthRange[0]    = limits.lineWidthRange[0];
    pipelineLimits.lineWidthRange[1]    = limits.lineWidthRange[1];
    pipelineLimits.lineWidthGranularity = limits.lineWidthGranularity;

    /*
    TODO: extension limits
    - VkPhysicalDeviceTransformFeedbackFeaturesEXT
    - VkPhysicalDeviceTransformFeedbackPropertiesEXT
    */
}

VKDevice VKPhysicalDevice::CreateLogicalDevice()
{
    VKDevice device;
    device.CreateLogicalDevice(
        physicalDevice_,
        &features_,
        enabledExtensionNames_.data(),
        static_cast<std::uint32_t>(enabledExtensionNames_.size())
    );
    return device;
}

std::uint32_t VKPhysicalDevice::FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const
{
    return VKFindMemoryType(memoryProperties_, memoryTypeBits, properties);
}

bool VKPhysicalDevice::SupportsExtension(const char* extension) const
{
    auto it = std::find_if(
        supportedExtensionNames_.begin(),
        supportedExtensionNames_.end(),
        [extension](const char* entry)
        {
            return (std::strcmp(extension, entry) == 0);
        }
    );
    return (it != supportedExtensionNames_.end());
}


/*
 * ======= Private: =======
 */

bool VKPhysicalDevice::EnableExtensions(const char** extensions, bool required)
{
    for (; *extensions != nullptr; ++extensions)
    {
        const char* name = *extensions;
        if (supportedExtensionNames_.find(name) != supportedExtensionNames_.end())
        {
            /* Add name to enabled Vulkan extensions */
            enabledExtensionNames_.push_back(name);
        }
        else if (required)
        {
            /* Cancel search and return with error */
            enabledExtensionNames_.clear();
            return false;
        }
    }
    return true;
}

void VKPhysicalDevice::QueryDeviceInfo()
{
    if (HasExtension(VKExt::KHR_get_physical_device_properties2))
    {
        /* Query physical device features and properties with extensions */
        QueryDeviceFeaturesWithExtensions();
        QueryDevicePropertiesWithExtensions();
        QueryDeviceMemoryPropertiesWithExtensions();
    }
    else
    {
        /* Query physical device features and memory properties */
        vkGetPhysicalDeviceFeatures(physicalDevice_, &features_);
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties_);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
    }
}

void VKPhysicalDevice::QueryDeviceFeaturesWithExtensions()
{
    /*VkPhysicalDeviceFeatures2 featuresExt;
    {
        featuresExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        //TODO...
    }
    vkGetPhysicalDeviceFeatures2(physicalDevice_, &featuresExt);
    features_ = featuresExt.features;*/

    vkGetPhysicalDeviceFeatures(physicalDevice_, &features_);
}

struct VKBaseStructureInfo
{
    VkStructureType sType;
    void*           pNext;
};

void VKPhysicalDevice::QueryDevicePropertiesWithExtensions()
{
    VKBaseStructureInfo* currentDesc = nullptr;

    auto ChainDescritpor = [&currentDesc](void* descPtr, VkStructureType type)
    {
        /* Chain next descriptor into previous one */
        currentDesc->pNext = descPtr;

        /* Write structure type and store next descriptor */
        auto baseDescPtr = reinterpret_cast<VKBaseStructureInfo*>(descPtr);
        {
            baseDescPtr->sType = type;
        }
        currentDesc = baseDescPtr;
    };

    /* Chain extensions into output descriptor */
    VkPhysicalDeviceProperties2 propertiesExt = {};
    propertiesExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    currentDesc = reinterpret_cast<VKBaseStructureInfo*>(&propertiesExt);

    if (HasExtension(VKExt::EXT_conservative_rasterization))
        ChainDescritpor(&conservRasterProps_, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT);

    /* Query device properties with extension "VK_KHR_get_physical_device_properties2" */
    vkGetPhysicalDeviceProperties2(physicalDevice_, &propertiesExt);

    /* Store primary device properties */
    properties_ = propertiesExt.properties;
}

void VKPhysicalDevice::QueryDeviceMemoryPropertiesWithExtensions()
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
}


} // /namespace LLGL



// ================================================================================
