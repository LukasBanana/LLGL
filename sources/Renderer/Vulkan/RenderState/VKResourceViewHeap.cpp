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

    pipelineLayout_ = pipelineLayoutVK->Get();

    /* Create resource descriptor pool */
    CreateDescriptorPool(desc);
    
    /* Create resource descriptor set for pipeline layout */
    VkDescriptorSetLayout setLayouts[] = { pipelineLayoutVK->GetDescriptorSetLayout() };
    CreateDescriptorSets(1, setLayouts);

    /* Update write descriptors in descriptor set */
    UpdateDescriptorSets(desc);
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

void VKResourceViewHeap::CreateDescriptorPool(const ResourceViewHeapDescriptor& desc)
{
    /* Initialize descriptor pool sizes */
    std::vector<VkDescriptorPoolSize> poolSizes(desc.resourceViews.size());
    for (std::size_t i = 0; i < desc.resourceViews.size(); ++i)
    {
        poolSizes[i].type            = VKTypes::Map(desc.resourceViews[i].type);
        poolSizes[i].descriptorCount = 1;
    }

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

void VKResourceViewHeap::UpdateDescriptorSets(const ResourceViewHeapDescriptor& desc)
{
    const auto numResourceViewsMax = desc.resourceViews.size();

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
            {
                auto bufferVK = LLGL_CAST(VKBuffer*, rvDesc.buffer);

                /* Initialize buffer information */
                auto& bufInfo = bufferInfos[numResourceViews];

                bufInfo.buffer  = bufferVK->Get();
                bufInfo.offset  = 0;
                bufInfo.range   = bufferVK->GetSize();

                /* Initialize write descriptor */
                auto& writeDesc = writeDescriptors[numResourceViews];

                writeDesc.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDesc.pNext               = nullptr;
                writeDesc.dstSet              = descriptorSets_[0];//numResourceViews];
                writeDesc.dstBinding          = 0;
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
