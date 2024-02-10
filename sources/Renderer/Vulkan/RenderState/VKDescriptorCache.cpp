/*
 * VKDescriptorCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDescriptorCache.h"
#include "VKStagingDescriptorSetPool.h"
#include "../VKCore.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKTexture.h"
#include "../Texture/VKSampler.h"
#include "../../CheckedCast.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <algorithm>


namespace LLGL
{


static std::uint32_t SumDescriptorPoolSizes(std::uint32_t numSizes, const VkDescriptorPoolSize* sizes)
{
    std::uint32_t count = 0;
    for_range(i, numSizes)
        count += sizes[i].descriptorCount;
    return count;
}

VKDescriptorCache::VKDescriptorCache(
    VkDevice                            device,
    VkDescriptorPool                    descriptorPool,
    VkDescriptorSetLayout               setLayout,
    std::uint32_t                       numSizes,
    const VkDescriptorPoolSize*         sizes,
    const ArrayView<VKLayoutBinding>&   bindings)
:
    device_         { device                                  },
    setLayout_      { setLayout                               },
    poolSizes_      { sizes, sizes + numSizes                 },
    numDescriptors_ { SumDescriptorPoolSizes(numSizes, sizes) }
{
    /* Allocate descriptor set for immutable samplers */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool;
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = &setLayout;
    }
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet_);
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");

    /* Pre-allocate VkCopyDescriptorSet array */
    BuildCopyDescriptors(bindings);
}

void VKDescriptorCache::Reset()
{
    dirty_ = true;
}

void VKDescriptorCache::EmplaceDescriptor(Resource& resource, const VKLayoutBinding& binding, VKDescriptorSetWriter& setWriter)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
            EmplaceBufferDescriptor(LLGL_CAST(VKBuffer&, resource), binding, setWriter);
            dirty_ = true;
            break;

        case ResourceType::Texture:
            EmplaceTextureDescriptor(LLGL_CAST(VKTexture&, resource), binding, setWriter);
            dirty_ = true;
            break;

        case ResourceType::Sampler:
            EmplaceSamplerDescriptor(LLGL_CAST(VKSampler&, resource), binding, setWriter);
            dirty_ = true;
            break;

        default:
            break;
    }
}

VkDescriptorSet VKDescriptorCache::FlushDescriptorSet(VKStagingDescriptorSetPool& pool, VKDescriptorSetWriter& setWriter)
{
    if (!dirty_ || setLayout_ == VK_NULL_HANDLE)
        return VK_NULL_HANDLE;

    /*
    Perform two operations in order:
    1. Update previously written descriptors to cache; Descriptor writes are performed first by 'vkUpdateDescriptorSets'.
    2. Copy cache into new descriptor set; Descriptor copies are performed second by 'vkUpdateDescriptorSets'.
    */
    VkDescriptorSet descriptorSetCopy = pool.AllocateDescriptorSet(setLayout_, static_cast<std::uint32_t>(poolSizes_.size()), poolSizes_.data());

    /* Lock mutex to guard copy descriptors since they are shared across threads */
    std::lock_guard<std::mutex> guard{ copyDescMutex_ };

    UpdateCopyDescriptorSet(descriptorSetCopy);

    vkUpdateDescriptorSets(
        device_,
        setWriter.GetNumWrites(),
        setWriter.GetWrites(),
        static_cast<std::uint32_t>(copyDescs_.size()),
        copyDescs_.data()
    );

    /* Clear cache after updated  */
    dirty_ = false;

    return descriptorSetCopy;
}


/*
 * ======= Private: =======
 */

VkDescriptorBufferInfo* VKDescriptorCache::NextBufferInfoOrUpdateCache(VKDescriptorSetWriter& setWriter)
{
    auto info = setWriter.NextBufferInfo();
    if (info == nullptr)
    {
        /* Flush descriptor set update */
        setWriter.UpdateDescriptorSets(device_);
        setWriter.Reset();
        return setWriter.NextBufferInfo();
    }
    return info;
}

VkDescriptorImageInfo* VKDescriptorCache::NextImageInfoOrUpdateCache(VKDescriptorSetWriter& setWriter)
{
    auto info = setWriter.NextImageInfo();
    if (info == nullptr)
    {
        /* Flush descriptor set update */
        setWriter.UpdateDescriptorSets(device_);
        setWriter.Reset();
        return setWriter.NextImageInfo();
    }
    return info;
}

void VKDescriptorCache::EmplaceBufferDescriptor(VKBuffer& bufferVK, const VKLayoutBinding& binding, VKDescriptorSetWriter& setWriter)
{
    auto bufferInfo = NextBufferInfoOrUpdateCache(setWriter);
    {
        bufferInfo->buffer  = bufferVK.GetVkBuffer();
        bufferInfo->offset  = 0;
        bufferInfo->range   = VK_WHOLE_SIZE;
    }
    auto writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSet_;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = nullptr;
        writeDesc->pBufferInfo      = bufferInfo;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKDescriptorCache::EmplaceTextureDescriptor(VKTexture& textureVK, const VKLayoutBinding& binding, VKDescriptorSetWriter& setWriter)
{
    auto imageInfo = NextImageInfoOrUpdateCache(setWriter);
    {
        imageInfo->sampler       = VK_NULL_HANDLE;
        imageInfo->imageView     = textureVK.GetVkImageView();
        imageInfo->imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    auto writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSet_;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKDescriptorCache::EmplaceSamplerDescriptor(VKSampler& samplerVK, const VKLayoutBinding& binding, VKDescriptorSetWriter& setWriter)
{
    auto imageInfo = NextImageInfoOrUpdateCache(setWriter);
    {
        imageInfo->sampler          = samplerVK.GetVkSampler();
        imageInfo->imageView        = VK_NULL_HANDLE;
        imageInfo->imageLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    auto writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSet_;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

// Returns true if the specified bindings are already sorted by 'dstBinding' in ascending order.
static bool AreLayoutBindingsSorted(const ArrayView<VKLayoutBinding>& bindings)
{
    std::uint32_t dstBindingPrev = 0;
    for (const VKLayoutBinding& binding : bindings)
    {
        if (dstBindingPrev > binding.dstBinding)
            return false;
        dstBindingPrev = binding.dstBinding;
    }
    return true;
}

void VKDescriptorCache::BuildCopyDescriptors(ArrayView<VKLayoutBinding> bindings)
{
    /* Sort list by binding slots to build array of consecutive descriptors for each entry in the copy descriptor array */
    std::vector<VKLayoutBinding> sortedBindings;
    if (!AreLayoutBindingsSorted(bindings))
    {
        sortedBindings = std::vector<VKLayoutBinding>(bindings.begin(), bindings.end());
        std::sort(
            sortedBindings.begin(), sortedBindings.end(),
            [](const VKLayoutBinding& lhs, const VKLayoutBinding& rhs) -> bool
            {
                return (lhs.dstBinding < rhs.dstBinding);
            }
        );
        bindings = sortedBindings;
    }

    /* Iterate over all bindings and create a new copy descriptor whenever the type changes */
    VkDescriptorType descTypePrev = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    std::uint32_t firstBinding = 0;
    std::uint32_t numBindings = 0;

    auto FlushBindingsToCopyDesc = [this, &descTypePrev, &numBindings, &firstBinding]()
    {
        if (numBindings > 0)
        {
            VkCopyDescriptorSet copyDesc;
            {
                copyDesc.sType              = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                copyDesc.pNext              = nullptr;
                copyDesc.srcSet             = this->descriptorSet_;
                copyDesc.srcBinding         = firstBinding;
                copyDesc.srcArrayElement    = 0;
                copyDesc.dstSet             = VK_NULL_HANDLE;
                copyDesc.dstBinding         = firstBinding;
                copyDesc.dstArrayElement    = 0;
                copyDesc.descriptorCount    = numBindings;
            }
            this->copyDescs_.push_back(copyDesc);
            numBindings = 0;
        }
    };

    for (const VKLayoutBinding& binding : bindings)
    {
        if (descTypePrev == VK_DESCRIPTOR_TYPE_MAX_ENUM)
        {
            firstBinding = binding.dstBinding;
            descTypePrev = binding.descriptorType;
        }
        else if (descTypePrev != binding.descriptorType)
        {
            FlushBindingsToCopyDesc();
            firstBinding = binding.dstBinding;
            descTypePrev = binding.descriptorType;
        }
        ++numBindings;
    }
    FlushBindingsToCopyDesc();
}

void VKDescriptorCache::UpdateCopyDescriptorSet(VkDescriptorSet dstSet)
{
    for (VkCopyDescriptorSet& copyDesc : copyDescs_)
        copyDesc.dstSet = dstSet;
}


} // /namespace LLGL



// ================================================================================
