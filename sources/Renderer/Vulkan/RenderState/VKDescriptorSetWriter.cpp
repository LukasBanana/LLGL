/*
 * VKDescriptorSetWriter.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDescriptorSetWriter.h"
#include <algorithm>


namespace LLGL
{


VKDescriptorSetWriter::VKDescriptorSetWriter(
    std::uint32_t numResourceViewsMax,
    std::uint32_t numReservedWrites,
    std::uint32_t numReservedCopies)
:
    bufferInfos_ { numResourceViewsMax },
    imageInfos_  { numResourceViewsMax },
    bufferViews_ { numResourceViewsMax }
{
    writes_.reserve(numReservedWrites);
    copies_.reserve(numReservedCopies);
}

void VKDescriptorSetWriter::Reset()
{
    writes_.clear();
    copies_.clear();

    numBufferInfos_ = 0;
    numImageInfos_  = 0;
    numBufferViews_ = 0;
}

void VKDescriptorSetWriter::Reset(
    std::uint32_t numResourceViewsMax,
    std::uint32_t numReservedWrites,
    std::uint32_t numReservedCopies)
{
    if (bufferInfos_.size() < numResourceViewsMax)
        bufferInfos_.resize(numResourceViewsMax);
    if (imageInfos_.size() < numResourceViewsMax)
        imageInfos_.resize(numResourceViewsMax);
    if (bufferViews_.size() < numResourceViewsMax)
        bufferViews_.resize(numResourceViewsMax);

    writes_.clear();
    copies_.clear();

    writes_.reserve(numReservedWrites);
    copies_.reserve(numReservedCopies);

    numBufferInfos_ = 0;
    numImageInfos_  = 0;
    numBufferViews_ = 0;
}

VkDescriptorBufferInfo* VKDescriptorSetWriter::NextBufferInfo()
{
    if (numBufferInfos_ < bufferInfos_.size())
        return &(bufferInfos_[numBufferInfos_++]);
    else
        return nullptr;
}

VkDescriptorImageInfo* VKDescriptorSetWriter::NextImageInfo()
{
    if (numImageInfos_ < imageInfos_.size())
        return &(imageInfos_[numImageInfos_++]);
    else
        return nullptr;
}

VkBufferView* VKDescriptorSetWriter::NextBufferView()
{
    if (numBufferViews_ < bufferViews_.size())
        return &(bufferViews_[numBufferViews_++]);
    else
        return nullptr;
}

VkWriteDescriptorSet* VKDescriptorSetWriter::NextWriteDescriptor()
{
    VkWriteDescriptorSet initialWriteDescriptor = {};
    {
        initialWriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    }
    writes_.push_back(initialWriteDescriptor);
    return &(writes_.back());
}

VkCopyDescriptorSet* VKDescriptorSetWriter::NextCopyDescriptor()
{
    VkCopyDescriptorSet initialCopyDescriptor = {};
    {
        initialCopyDescriptor.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    }
    copies_.push_back(initialCopyDescriptor);
    return &(copies_.back());
}

void VKDescriptorSetWriter::UpdateDescriptorSets(VkDevice device)
{
    if (!writes_.empty() || !copies_.empty())
    {
        vkUpdateDescriptorSets(
            device,
            static_cast<std::uint32_t>(writes_.size()),
            writes_.data(),
            static_cast<std::uint32_t>(copies_.size()),
            copies_.data()
        );
    }
}


} // /namespace LLGL



// ================================================================================
