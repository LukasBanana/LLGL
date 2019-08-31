/*
 * VKContainers.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKContainers.h"


namespace LLGL
{


/*
 * VKWriteDescriptorContainer structure
 */

VKWriteDescriptorContainer::VKWriteDescriptorContainer(std::size_t numResourceViewsMax) :
    bufferInfos      { numResourceViewsMax },
    imageInfos       { numResourceViewsMax },
    writeDescriptors { numResourceViewsMax }
{
}

VkDescriptorBufferInfo* VKWriteDescriptorContainer::NextBufferInfo()
{
    return &(bufferInfos[numBufferInfos++]);
}

VkDescriptorImageInfo* VKWriteDescriptorContainer::NextImageInfo()
{
    return &(imageInfos[numImageInfos++]);
}

VkWriteDescriptorSet* VKWriteDescriptorContainer::NextWriteDescriptor()
{
    auto desc = (&writeDescriptors[numWriteDescriptors++]);
    {
        desc->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc->pNext = nullptr;
    }
    return desc;
}


} // /namespace LLGL



// ================================================================================
