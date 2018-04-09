/*
 * VKResourceViewHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKResourceViewHeap.h"
#include "VKPipelineLayout.h"
#include "../Buffer/VKBuffer.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include <map>


namespace LLGL
{


VKResourceViewHeap::VKResourceViewHeap(const VKPtr<VkDevice>& device, const ResourceViewHeapDescriptor& desc) :
    device_         { device                          },
    descriptorPool_ { device, vkDestroyDescriptorPool }
{
    /* Get pipeline layout object */
    auto pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        throw std::invalid_argument("failed to create resource view heap due to missing pipeline layout");

    pipelineLayout_ = pipelineLayoutVK->GetVkPipelineLayout();

    /* Create resource descriptor pool */
    CreateDescriptorPool(desc);
    
    /* Create resource descriptor set for pipeline layout */
    VkDescriptorSetLayout setLayouts[] = { pipelineLayoutVK->GetVkDescriptorSetLayout() };
    CreateDescriptorSets(1, setLayouts);

    /* Update write descriptors in descriptor set */
    UpdateDescriptorSets(desc, pipelineLayoutVK->GetDstBindings());
}

VKResourceViewHeap::~VKResourceViewHeap()
{
    //INFO: is automatically deleted when pool is deleted
    #if 0
    auto result = vkFreeDescriptorSets(device_, descriptorPool_, 1, &descriptorSet_);
    VKThrowIfFailed(result, "failed to release Vulkan descriptor sets");
    #endif
}


/*
 * ======= Private: =======
 */

static std::uint32_t AccumDescriptorPoolSizes(
    VkDescriptorType type,
    std::vector<VkDescriptorPoolSize>::iterator it,
    std::vector<VkDescriptorPoolSize>::iterator itEnd)
{
    std::uint32_t descriptorCount = it->descriptorCount;

    for (++it; it != itEnd; ++it)
    {
        if (it->type == type)
        {
            descriptorCount += it->descriptorCount;
            it->descriptorCount = 0;
        }
    }

    return descriptorCount;
}

static void CompressDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& poolSizes)
{
    /* Accumulate all descriptors of the same type */
    for (auto it = poolSizes.begin(); it != poolSizes.end(); ++it)
        it->descriptorCount = AccumDescriptorPoolSizes(it->type, it, poolSizes.end());
    
    /* Remove all remaining pool sizes with zero descriptors */
    RemoveAllFromListIf(
        poolSizes,
        [](const VkDescriptorPoolSize& dps)
        {
            return (dps.descriptorCount == 0);
        }
    );
}

void VKResourceViewHeap::CreateDescriptorPool(const ResourceViewHeapDescriptor& desc)
{
    /* Initialize descriptor pool sizes */
    std::vector<VkDescriptorPoolSize> poolSizes(desc.resourceViews.size());
    for (std::size_t i = 0; i < desc.resourceViews.size(); ++i)
    {
        poolSizes[i].type               = VKTypes::Map(desc.resourceViews[i].type);
        poolSizes[i].descriptorCount    = 1;
    }

    /* Compress pool sizes by merging equal types with accumulated number of descriptors */
    CompressDescriptorPoolSizes(poolSizes);

    /* Create descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = 1;
        poolCreateInfo.poolSizeCount    = static_cast<std::uint32_t>(poolSizes.size());
        poolCreateInfo.pPoolSizes       = poolSizes.data();
    }
    auto result = vkCreateDescriptorPool(device_, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool");
}

void VKResourceViewHeap::CreateDescriptorSets(std::uint32_t numSetLayouts, const VkDescriptorSetLayout* setLayouts)
{
    descriptorSets_.resize(1, VK_NULL_HANDLE);

    /* Allocate descriptor set */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_;
        allocInfo.descriptorSetCount    = numSetLayouts;
        allocInfo.pSetLayouts           = setLayouts;
    }
    auto result = vkAllocateDescriptorSets(device_, &allocInfo, descriptorSets_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
}

void VKResourceViewHeap::UpdateDescriptorSets(const ResourceViewHeapDescriptor& desc, const std::vector<std::uint32_t>& dstBindings)
{
    const auto numResourceViewsMax = std::min(desc.resourceViews.size(), dstBindings.size());

    std::vector<VkDescriptorBufferInfo> bufferInfos(numResourceViewsMax);
    std::vector<VkWriteDescriptorSet> writeDescriptors(numResourceViewsMax);

    std::uint32_t numResourceViews = 0;

    for (std::size_t i = 0; i < numResourceViewsMax; ++i)
    {
        /* Get resource view information */
        const auto& rvDesc = desc.resourceViews[i];

        switch (rvDesc.type)
        {
            case ResourceViewType::ConstantBuffer:
            case ResourceViewType::StorageBuffer:
            {
                auto bufferVK = LLGL_CAST(VKBuffer*, rvDesc.buffer);

                /* Initialize buffer information */
                auto& bufInfo = bufferInfos[numResourceViews];
                
                bufInfo.buffer  = bufferVK->GetVkBuffer();
                bufInfo.offset  = 0;
                bufInfo.range   = bufferVK->GetSize();

                /* Initialize write descriptor */
                auto& writeDesc = writeDescriptors[numResourceViews];

                writeDesc.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDesc.pNext               = nullptr;
                writeDesc.dstSet              = descriptorSets_[0];//numResourceViews];
                writeDesc.dstBinding          = dstBindings[i];
                writeDesc.dstArrayElement     = 0;
                writeDesc.descriptorCount     = 1;
                writeDesc.descriptorType      = VKTypes::Map(rvDesc.type);
                writeDesc.pImageInfo          = nullptr;
                writeDesc.pBufferInfo         = &bufferInfos[numResourceViews];
                writeDesc.pTexelBufferView    = nullptr;

                ++numResourceViews;
            }
            break;

            default:
            {
                throw std::invalid_argument(
                    "invalid resource view type to create ResourceViewHeap object: 0x" +
                    ToHex(static_cast<std::uint32_t>(rvDesc.type))
                );
            }
            break;
        }
    }

    if (numResourceViews > 0)
        vkUpdateDescriptorSets(device_, numResourceViews, writeDescriptors.data(), 0, nullptr);
}


} // /namespace LLGL



// ================================================================================
