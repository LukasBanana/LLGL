/*
 * VKDevicePhysical.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPhysicalDevice.h"
#include "VKCore.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "../../Core/Vendor.h"
#include <string>
#include <set>


namespace LLGL
{


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
    const char* requiredExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    };

    std::vector<VkExtensionProperties> extensions;
    bool suitable = CheckDeviceExtensionSupport(
        physicalDevice,
        requiredExtensions,
        sizeof(requiredExtensions) / sizeof(requiredExtensions[0]),
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
            supportedExtensionNames_.reserve(supportedExtensions_.size());
            for (const auto& extension : supportedExtensions_)
                supportedExtensionNames_.push_back(extension.extensionName);

            /* Store device and store properties */
            physicalDevice_ = device;
            QueryDeviceProperties();

            return true;
        }
    }

    return false;
}

void VKPhysicalDevice::QueryDeviceProperties(
    RendererInfo&               info,
    RenderingCapabilities&      caps,
    VKGraphicsPipelineLimits&   pipelineLimits)
{
    /* Query properties of selected physical device */
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

    /* Map properties to output renderer info */
    info.rendererName           = ("Vulkan " + VKApiVersionToString(properties.apiVersion));
    info.deviceName             = properties.deviceName;
    info.vendorName             = GetVendorByID(properties.vendorID);
    info.shadingLanguageName    = "SPIR-V";

    /* Map limits to output rendering capabilites */
    const auto& limits = properties.limits;

    /* Query common attributes */
    caps.screenOrigin                               = ScreenOrigin::UpperLeft;
    caps.clippingRange                              = ClippingRange::ZeroToOne;
    caps.shadingLanguages                           = { ShadingLanguage::SPIRV, ShadingLanguage::SPIRV_100 };
    //caps.textureFormats                             = ; //???

    /* Query features */
    caps.features.hasRenderTargets                  = true;
    caps.features.has3DTextures                     = true;
    caps.features.hasCubeTextures                   = true;
    caps.features.hasArrayTextures                  = true;
    caps.features.hasCubeArrayTextures              = (features_.imageCubeArray != VK_FALSE);
    caps.features.hasMultiSampleTextures            = true;
    caps.features.hasTextureViews                   = true;
    caps.features.hasTextureViewSwizzle             = true;
    caps.features.hasSamplers                       = true;
    caps.features.hasConstantBuffers                = true;
    caps.features.hasStorageBuffers                 = true;
    caps.features.hasUniforms                       = true;
    caps.features.hasGeometryShaders                = (features_.geometryShader != VK_FALSE);
    caps.features.hasTessellationShaders            = (features_.tessellationShader != VK_FALSE);
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

    /* Store graphics pipeline spcific limitations */
    pipelineLimits.lineWidthRange[0]    = limits.lineWidthRange[0];
    pipelineLimits.lineWidthRange[1]    = limits.lineWidthRange[1];
    pipelineLimits.lineWidthGranularity = limits.lineWidthGranularity;
}

VKDevice VKPhysicalDevice::CreateLogicalDevice()
{
    VKDevice device;
    device.CreateLogicalDevice(
        physicalDevice_,
        &features_,
        supportedExtensionNames_.data(),
        static_cast<std::uint32_t>(supportedExtensionNames_.size())
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

void VKPhysicalDevice::QueryDeviceProperties()
{
    /* Query physical device features and memory propertiers */
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
    vkGetPhysicalDeviceFeatures(physicalDevice_, &features_);
}


} // /namespace LLGL



// ================================================================================
