/*
 * NativeHandle.h (Vulkan)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VULKAN_NATIVE_HANDLE_H
#define LLGL_VULKAN_NATIVE_HANDLE_H


#include <cstdint>
#include <vulkan/vulkan.h>
#include <LLGL/Deprecated.h>


namespace LLGL
{

namespace Vulkan
{


/**
\brief Native handle structure for the Vulkan render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    //! Native handle to the Vulkan instance.
    VkInstance          instance;

    //! Native handle to the physical Vulkan device.
    VkPhysicalDevice    physicalDevice;

    //! Native handle to the logical Vulkan device.
    VkDevice            device;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queue is deprecated since 0.04b; Use vkGetDeviceQueue on the logical device instead!")
    VkQueue             queue;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queueGraphicsFamily is deprecated since 0.04b; Use vkGetPhysicalDeviceQueueFamilyProperties on the physical device instead!")
    std::uint32_t       queueGraphicsFamily;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queuePresentFamily is deprecated since 0.04b; Use vkGetPhysicalDeviceQueueFamilyProperties on the physical device instead!")
    std::uint32_t       queuePresentFamily;
};

/**
\brief Native handle structure for the Vulkan command buffer.
\see CommandBuffer::GetNativeHandle
*/
struct CommandBufferNativeHandle
{
    VkCommandBuffer commandBuffer;
};

/**
\brief Native Vulkan resource type enumeration.
\see ResourceNativeHandle::type
*/
enum class ResourceNativeType
{
    /**
    \brief Native Vulkan VkBuffer type.
    \see ResourceNativeHandle::buffer
    */
    Buffer,

    /**
    \brief Native Vulkan VkImage type.
    \see ResourceNativeHandle::texture
    */
    Image,

    /**
    \brief Native Vulkan VkSampler type.
    \see ResourceNativeHandle::sampler
    */
    Sampler,
};

/**
\brief Native handle structure for a Vulkan resource.
\see Resource::GetNativeHandle
*/
struct ResourceNativeHandle
{
    struct NativeBuffer
    {
        VkBuffer buffer;    //!< Native Vulkan VkBuffer object.
    };

    struct NativeImage
    {
        VkImage                 image;              //!< Primary Vulkan image stored as native VkImage type.
        VkImageLayout           imageLayout;        //!< Current image layout. This depends on resource transitioning.
        VkFormat                format;             //!< Native Vulkan image format.
        VkExtent3D              extent;             //!< Native Vulkan image extent. Does \e not include array layers.
        std::uint32_t           numMipLevels;       //!< Number of MIP-map levels.
        std::uint32_t           numArrayLayers;     //!< Number of array layers.
        VkSampleCountFlagBits   sampleCountBits;    //!< Sample count bitmask for multi-sampled textures.
        VkImageUsageFlags       imageUsageFlags;    //!< Image usag flags the texture was created with.
    };

    struct NativeSampler
    {
        VkSampler sampler;  //!< Native Vulkan VkSampler object.
    };

    /**
    \brief Specifies the native resource type.
    \remarks This allows to distinguish a resource between native Vulkan types.
    */
    ResourceNativeType  type;

    union
    {
        //! Buffer specific attriubtes.
        NativeBuffer    buffer;

        //! Texture specific attriubtes.
        NativeImage     image;

        //! Sampler specific attriubtes.
        NativeSampler   sampler;
    };
};


} // /namespace Vulkan

} // /namespace LLGL


#endif



// ================================================================================
