/*
 * VKResourceViewHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKResourceViewHeap.h"
#include "VKPipelineLayout.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKSampler.h"
#include "../Texture/VKTexture.h"
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

//TODO: separate the switch statement into separate functions
void VKResourceViewHeap::UpdateDescriptorSets(const ResourceViewHeapDescriptor& desc, const std::vector<std::uint32_t>& dstBindings)
{
    /* Allocate local storage for buffer and image descriptors */
    const auto numResourceViewsMax = std::min(desc.resourceViews.size(), dstBindings.size());

    std::vector<VkDescriptorBufferInfo> bufferInfos(numResourceViewsMax);
    std::uint32_t numBufferInfos = 0;

    std::vector<VkDescriptorImageInfo> imageInfos(numResourceViewsMax);
    std::uint32_t numImageInfos = 0;

    std::vector<VkWriteDescriptorSet> writeDescriptors(numResourceViewsMax);
    std::uint32_t numWriteDescriptors = 0;

    /* Acquires a new VkDescriptorBufferInfo from the local container */
    auto NextBufferInfo = [&](VKBuffer& bufferVK)
    {
        auto resourceInfo = &(bufferInfos[numBufferInfos++]);

        resourceInfo->buffer    = bufferVK.GetVkBuffer();
        resourceInfo->offset    = 0;
        resourceInfo->range     = bufferVK.GetSize();

        return resourceInfo;
    };

    /* Acquires a new VkDescriptorImageInfo from the local container */
    auto NextImageInfo = [&]()
    {
        return &(imageInfos[numImageInfos++]);
    };

    /* Acquires a new VkWriteDescriptorSet from the local container */
    auto NextWriteDescriptor = [&](std::size_t resourceIndex)
    {
        const auto& rvDesc = desc.resourceViews[resourceIndex];

        auto writeDesc = (&writeDescriptors[numWriteDescriptors++]);

        writeDesc->sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc->pNext             = nullptr;
        writeDesc->dstSet            = descriptorSets_[0];//numResourceViews];
        writeDesc->dstBinding        = dstBindings[resourceIndex];
        writeDesc->dstArrayElement   = 0;
        writeDesc->descriptorCount   = 1;
        writeDesc->descriptorType    = VKTypes::Map(rvDesc.type);
        writeDesc->pImageInfo        = nullptr;
        writeDesc->pBufferInfo       = nullptr;
        writeDesc->pTexelBufferView  = nullptr;

        return writeDesc;
    };

    for (std::size_t i = 0; i < numResourceViewsMax; ++i)
    {
        /* Get resource view information */
        const auto& rvDesc = desc.resourceViews[i];

        switch (rvDesc.type)
        {
            case ResourceViewType::Sampler:
            {
                auto samplerVK = LLGL_CAST(VKSampler*, rvDesc.sampler);

                /* Initialize image information */
                auto resourceInfo = NextImageInfo();

                resourceInfo->sampler       = samplerVK->GetVkSampler();
                resourceInfo->imageView     = VK_NULL_HANDLE;
                resourceInfo->imageLayout   = VK_IMAGE_LAYOUT_UNDEFINED;

                /* Initialize write descriptor */
                auto writeDesc = NextWriteDescriptor(i);

                writeDesc->pImageInfo = resourceInfo;
            }
            break;

            case ResourceViewType::Texture:
            {
                auto textureVK = LLGL_CAST(VKTexture*, rvDesc.texture);

                /* Initialize image information */
                auto resourceInfo = NextImageInfo();

                resourceInfo->sampler       = VK_NULL_HANDLE;
                resourceInfo->imageView     = textureVK->GetVkImageView();
                resourceInfo->imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                /* Initialize write descriptor */
                auto writeDesc = NextWriteDescriptor(i);

                writeDesc->pImageInfo = resourceInfo;
            }
            break;

            case ResourceViewType::ConstantBuffer:
            case ResourceViewType::StorageBuffer:
            {
                auto bufferVK = LLGL_CAST(VKBuffer*, rvDesc.buffer);

                /* Initialize buffer information */
                auto resourceInfo = NextBufferInfo(*bufferVK);

                /* Initialize write descriptor */
                auto writeDesc = NextWriteDescriptor(i);

                writeDesc->pBufferInfo = resourceInfo;
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

    if (numWriteDescriptors > 0)
        vkUpdateDescriptorSets(device_, numWriteDescriptors, writeDescriptors.data(), 0, nullptr);
}


} // /namespace LLGL



// ================================================================================
