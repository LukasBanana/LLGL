/*
 * VKResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKResourceHeap.h"
#include "VKPipelineLayout.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKSampler.h"
#include "../Texture/VKTexture.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../VKContainers.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include <LLGL/ResourceHeapFlags.h>
#include <map>


namespace LLGL
{


VKResourceHeap::VKResourceHeap(const VKPtr<VkDevice>& device, const ResourceHeapDescriptor& desc) :
    device_         { device                          },
    descriptorPool_ { device, vkDestroyDescriptorPool }
{
    /* Get pipeline layout object */
    auto pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        throw std::invalid_argument("failed to create resource view heap due to missing pipeline layout");

    pipelineLayout_ = pipelineLayoutVK->GetVkPipelineLayout();

    /* Validate binding descriptors */
    const auto& bindings = pipelineLayoutVK->GetBindings();
    if (desc.resourceViews.size() != bindings.size())
        throw std::invalid_argument("failed to create resource vied heap due to mismatch between number of resources and bindings");

    /* Create resource descriptor pool */
    CreateDescriptorPool(desc, bindings);

    /* Create resource descriptor set for pipeline layout */
    VkDescriptorSetLayout setLayouts[] = { pipelineLayoutVK->GetVkDescriptorSetLayout() };
    CreateDescriptorSets(1, setLayouts);

    /* Update write descriptors in descriptor set */
    UpdateDescriptorSets(desc, bindings);

    /* Create pipeline barrier for resource views that require it, e.g. those with storage binding flags */
    CreatePipelineBarrier(desc.resourceViews, pipelineLayoutVK->GetBindings());
}

VKResourceHeap::~VKResourceHeap()
{
    //INFO: is automatically deleted when pool is deleted
    #if 0
    auto result = vkFreeDescriptorSets(device_, descriptorPool_, 1, &descriptorSet_);
    VKThrowIfFailed(result, "failed to release Vulkan descriptor sets");
    #endif
}

void VKResourceHeap::InsertPipelineBarrier(VkCommandBuffer commandBuffer)
{
    if (barrier_.IsEnabled())
        barrier_.Submit(commandBuffer);
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

void VKResourceHeap::CreateDescriptorPool(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings)
{
    /* Initialize descriptor pool sizes */
    std::vector<VkDescriptorPoolSize> poolSizes(desc.resourceViews.size());
    for (std::size_t i = 0; i < desc.resourceViews.size(); ++i)
    {
        poolSizes[i].type               = bindings[i].descriptorType;
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

void VKResourceHeap::CreateDescriptorSets(std::uint32_t numSetLayouts, const VkDescriptorSetLayout* setLayouts)
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

void VKResourceHeap::UpdateDescriptorSets(const ResourceHeapDescriptor& desc, const std::vector<VKLayoutBinding>& bindings)
{
    /* Allocate local storage for buffer and image descriptors */
    const auto numResourceViewsMax = std::min(desc.resourceViews.size(), bindings.size());

    VKWriteDescriptorContainer container{ numResourceViewsMax };

    for (std::size_t i = 0; i < numResourceViewsMax; ++i)
    {
        /* Get resource view information */
        const auto& rvDesc = desc.resourceViews[i];
        auto descriptorType = bindings[i].descriptorType;

        switch (descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                FillWriteDescriptorForSampler(rvDesc, bindings[i], container);
                break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                FillWriteDescriptorForTexture(rvDesc, bindings[i], container);
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                FillWriteDescriptorForBuffer(rvDesc, bindings[i], container);
                break;

            default:
                throw std::invalid_argument(
                    "invalid descriptor type to create ResourceHeap object: 0x" +
                    ToHex(static_cast<std::uint32_t>(descriptorType))
                );
                break;
        }
    }

    if (container.numWriteDescriptors > 0)
    {
        vkUpdateDescriptorSets(
            device_,
            container.numWriteDescriptors,
            container.writeDescriptors.data(),
            0,
            nullptr
        );
    }
}

void VKResourceHeap::FillWriteDescriptorForSampler(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container)
{
    auto samplerVK = LLGL_CAST(VKSampler*, resourceViewDesc.resource);

    /* Initialize image information */
    auto imageInfo = container.NextImageInfo();
    {
        imageInfo->sampler          = samplerVK->GetVkSampler();
        imageInfo->imageView        = VK_NULL_HANDLE;
        imageInfo->imageLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    /* Initialize write descriptor */
    auto writeDesc = container.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[0];//numResourceViews];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::FillWriteDescriptorForTexture(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container)
{
    auto textureVK = LLGL_CAST(VKTexture*, resourceViewDesc.resource);

    /* Initialize image information */
    auto imageInfo = container.NextImageInfo();
    {
        imageInfo->sampler       = VK_NULL_HANDLE;
        imageInfo->imageView     = textureVK->GetVkImageView();
        imageInfo->imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    /* Initialize write descriptor */
    auto writeDesc = container.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[0];//numResourceViews];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::FillWriteDescriptorForBuffer(const ResourceViewDescriptor& resourceViewDesc, const VKLayoutBinding& binding, VKWriteDescriptorContainer& container)
{
    auto bufferVK = LLGL_CAST(VKBuffer*, resourceViewDesc.resource);

    /* Initialize buffer information */
    auto bufferInfo = container.NextBufferInfo();
    {
        bufferInfo->buffer    = bufferVK->GetVkBuffer();
        bufferInfo->offset    = 0;
        bufferInfo->range     = bufferVK->GetSize();
    }

    /* Initialize write descriptor */
    auto writeDesc = container.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[0];//numResourceViews];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = nullptr;
        writeDesc->pBufferInfo      = bufferInfo;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::CreatePipelineBarrier(const std::vector<ResourceViewDescriptor>& resourceViews, const std::vector<VKLayoutBinding>& bindings)
{
    for (std::size_t i = 0; i < resourceViews.size(); ++i)
    {
        const auto& desc    = resourceViews[i];
        const auto& binding = bindings[i];

        if (auto resource = desc.resource)
        {
            /* Enable <GL_SHADER_STORAGE_BARRIER_BIT> bitmask for UAV buffers */
            if (resource->GetResourceType() == ResourceType::Buffer)
            {
                auto buffer = LLGL_CAST(Buffer*, resource);
                if ((buffer->GetBindFlags() & BindFlags::Storage) != 0)
                {
                    /* Insert worst case scenario for a UAV resource */
                    barrier_.InsertMemoryBarrier(
                        bindings[i].stageFlags,
                        VK_ACCESS_SHADER_WRITE_BIT,
                        VK_ACCESS_SHADER_READ_BIT
                    );
                }
            }
        }
    }
}


} // /namespace LLGL



// ================================================================================
