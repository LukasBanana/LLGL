/*
 * VKDescriptorSetWriter.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DESCRIPTOR_SET_WRITER_H
#define LLGL_VK_DESCRIPTOR_SET_WRITER_H


#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


// Helper structure to handle buffer and image information for a descriptor set.
class VKDescriptorSetWriter
{

    public:

        VKDescriptorSetWriter() = default;
        VKDescriptorSetWriter(
            std::uint32_t numResourceViewsMax,
            std::uint32_t numReservedWrites     = 0,
            std::uint32_t numReservedCopies     = 0
        );

        void Reset();

        void Reset(
            std::uint32_t numResourceViewsMax,
            std::uint32_t numReservedWrites     = 0,
            std::uint32_t numReservedCopies     = 0
        );

        VkDescriptorBufferInfo* NextBufferInfo();
        VkDescriptorImageInfo* NextImageInfo();
        VkBufferView* NextBufferView();

        VkWriteDescriptorSet* NextWriteDescriptor();
        VkCopyDescriptorSet* NextCopyDescriptor();

        // Returns the number of written descritpors.
        inline std::uint32_t GetNumWrites() const
        {
            return static_cast<std::uint32_t>(writes_.size());
        }

        // Returns the array of written descriptors.
        inline const VkWriteDescriptorSet* GetWrites() const
        {
            return writes_.data();
        }

        // Returns the number of copied descriptors.
        inline std::uint32_t GetNumCopies() const
        {
            return static_cast<std::uint32_t>(copies_.size());
        }

        // Returns the array of copied descriptors.
        inline const VkCopyDescriptorSet* GetCopies() const
        {
            return copies_.data();
        }

        // Invokes vkUpdateDescrpitorSets with the current containers.
        void UpdateDescriptorSets(VkDevice device);

    private:

        std::vector<VkDescriptorBufferInfo> bufferInfos_;
        std::uint32_t                       numBufferInfos_     = 0;

        std::vector<VkDescriptorImageInfo>  imageInfos_;
        std::uint32_t                       numImageInfos_      = 0;

        std::vector<VkBufferView>           bufferViews_;
        std::uint32_t                       numBufferViews_     = 0;

        std::vector<VkWriteDescriptorSet>   writes_;
        std::vector<VkCopyDescriptorSet>    copies_;

};


} // /namespace LLGL


#endif



// ================================================================================
