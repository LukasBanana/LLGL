/*
 * VKContainers.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_CONTAINERS_H
#define LLGL_VK_CONTAINERS_H


#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


// Helper structure to handle buffer and image information for a descriptor set.
struct VKWriteDescriptorContainer
{
    VKWriteDescriptorContainer() = default;
    VKWriteDescriptorContainer(std::size_t numResourceViewsMax);

    VkDescriptorBufferInfo* NextBufferInfo();
    VkDescriptorImageInfo* NextImageInfo();
    VkWriteDescriptorSet* NextWriteDescriptor();

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::uint32_t                       numBufferInfos      = 0;

    std::vector<VkDescriptorImageInfo>  imageInfos;
    std::uint32_t                       numImageInfos       = 0;

    std::vector<VkWriteDescriptorSet>   writeDescriptors;
    std::uint32_t                       numWriteDescriptors = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
