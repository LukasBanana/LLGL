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


static bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensionNames)
{
    /* Check if device supports all required extensions */
    auto availableExtensions = VKQueryDeviceExtensionProperties(physicalDevice);

    std::set<std::string> requiredExtensions(extensionNames.begin(), extensionNames.end());

    for (const auto& ext : availableExtensions)
        requiredExtensions.erase(ext.extensionName);

    return requiredExtensions.empty();
}

static bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    static const std::vector<const char*> g_deviceExtensions
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    };

    if (CheckDeviceExtensionSupport(physicalDevice, g_deviceExtensions))
    {
        //TODO...
        return true;
    }

    return false;
}

bool VKPhysicalDevice::PickPhysicalDevice(VkInstance instance)
{
    /* Query all physical devices and pick suitable */
    auto physicalDevices = VKQueryPhysicalDevices(instance);

    for (auto device : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(device))
        {
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
    caps.features.hasSamplers                       = true;
    caps.features.hasConstantBuffers                = true;
    caps.features.hasStorageBuffers                 = true;
    caps.features.hasUniforms                       = true;
    caps.features.hasGeometryShaders                = (features_.geometryShader != VK_FALSE);
    caps.features.hasTessellationShaders            = (features_.tessellationShader != VK_FALSE);
    caps.features.hasComputeShaders                 = true;
    caps.features.hasInstancing                     = true;
    caps.features.hasOffsetInstancing               = true;
    caps.features.hasViewportArrays                 = (features_.multiViewport != VK_FALSE);
    caps.features.hasConservativeRasterization      = false;
    caps.features.hasStreamOutputs                  = false;
    caps.features.hasLogicOp                        = true;

    /* Query limits */
    caps.limits.lineWidthRange[0]                   = limits.lineWidthRange[0];
    caps.limits.lineWidthRange[1]                   = limits.lineWidthRange[1];
    caps.limits.maxNumTextureArrayLayers            = limits.maxImageArrayLayers;
    caps.limits.maxNumRenderTargetAttachments       = static_cast<std::uint32_t>(limits.framebufferColorSampleCounts);
    caps.limits.maxPatchVertices                    = limits.maxTessellationPatchSize;
    caps.limits.max1DTextureSize                    = limits.maxImageDimension1D;
    caps.limits.max2DTextureSize                    = limits.maxImageDimension2D;
    caps.limits.max3DTextureSize                    = limits.maxImageDimension3D;
    caps.limits.maxCubeTextureSize                  = limits.maxImageDimensionCube;
    caps.limits.maxAnisotropy                       = static_cast<std::uint32_t>(limits.maxSamplerAnisotropy);
    caps.limits.maxNumComputeShaderWorkGroups[0]    = limits.maxComputeWorkGroupCount[0];
    caps.limits.maxNumComputeShaderWorkGroups[1]    = limits.maxComputeWorkGroupCount[1];
    caps.limits.maxNumComputeShaderWorkGroups[2]    = limits.maxComputeWorkGroupCount[2];
    caps.limits.maxComputeShaderWorkGroupSize[0]    = limits.maxComputeWorkGroupSize[0];
    caps.limits.maxComputeShaderWorkGroupSize[1]    = limits.maxComputeWorkGroupSize[1];
    caps.limits.maxComputeShaderWorkGroupSize[2]    = limits.maxComputeWorkGroupSize[2];
    caps.limits.maxNumViewports                     = limits.maxViewports;
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
    device.CreateLogicalDevice(physicalDevice_, &features_);
    return device;
}

std::uint32_t VKPhysicalDevice::FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const
{
    return VKFindMemoryType(memoryProperties_, memoryTypeBits, properties);
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
