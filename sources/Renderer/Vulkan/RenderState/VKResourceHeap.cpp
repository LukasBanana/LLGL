/*
 * VKResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKResourceHeap.h"
#include "VKPipelineLayout.h"
#include "VKDescriptorSetWriter.h"
#include "VKPoolSizeAccumulator.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKSampler.h"
#include "../Texture/VKTexture.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../ResourceUtils.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/Map.h>


namespace LLGL
{


VKResourceHeap::VKResourceHeap(
    VkDevice                                    device,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
:
    descriptorPool_    { device, vkDestroyDescriptorPool },
    numBufferBarriers_ { 0                               },
    numImageBarriers_  { 0                               }
{
    /* Get pipeline layout object */
    auto* pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        LLGL_TRAP("failed to create resource view heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    ConvertAllLayoutBindings(pipelineLayoutVK->GetBindingTable().heapBindings);

    const std::uint32_t numBindings         = static_cast<std::uint32_t>(bindings_.size());
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    /* Create descriptor pool and array of descriptor sets */
    const std::uint32_t numDescriptorSets = (numResourceViews / numBindings);
    CreateDescriptorPool(device, numDescriptorSets);
    CreateDescriptorSets(device, numDescriptorSets, pipelineLayoutVK->GetSetLayoutForHeapBindings());
    AllocateBarrierSlots(numDescriptorSets);

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        WriteResourceViews(device, 0, initialResourceViews);
}

std::uint32_t VKResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(descriptorSets_.size());
}

std::uint32_t VKResourceHeap::WriteResourceViews(
    VkDevice                                    device,
    std::uint32_t                               firstDescriptor,
    const ArrayView<ResourceViewDescriptor>&    resourceViews)
{
    if (resourceViews.empty())
        return 0;

    /* Allocate local storage for buffer and image descriptors */
    const std::uint32_t numSets         = GetNumDescriptorSets();
    const std::uint32_t numBindings     = static_cast<std::uint32_t>(bindings_.size());
    const std::uint32_t numDescriptors  = numSets * numBindings;

    /* Silently quit on out of bounds; debug layer must report these errors */
    if (firstDescriptor >= numDescriptors)
        return 0;
    if (firstDescriptor + resourceViews.size() > numDescriptors)
        return 0;

    const std::uint32_t numResourceViewWrites = static_cast<std::uint32_t>(resourceViews.size());
    VKDescriptorSetWriter setWriter{ numResourceViewWrites, numResourceViewWrites };

    for (const ResourceViewDescriptor& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get resource view information */
        const VKLayoutHeapBinding& binding = bindings_[firstDescriptor % numBindings];

        const std::uint32_t descriptorSet = firstDescriptor / numBindings;

        switch (binding.descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                FillWriteDescriptorWithSampler(desc, descriptorSet, binding, setWriter);
                break;

            #if 0
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                FillWriteDescriptorWithCombinedImageSampler(device, desc, descriptorSet, binding, setWriter);
                break;
            #endif

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                FillWriteDescriptorWithImageView(device, desc, descriptorSet, binding, setWriter);
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                FillWriteDescriptorWithBufferRange(device, desc, descriptorSet, binding, setWriter);
                break;

            default:
                LLGL_TRAP("invalid descriptor type in Vulkan descriptor set: 0x%08X", static_cast<unsigned>(binding.descriptorType));
                break;
        }

        ++firstDescriptor;
    }

    if (setWriter.GetNumWrites() > 0)
    {
        /* All command buffers must have finished execution before any affected descriptor set can be updated */
        vkDeviceWaitIdle(device);
        setWriter.UpdateDescriptorSets(device);
    }

    return setWriter.GetNumWrites();
}

void VKResourceHeap::SetBarrierSlots(VKPipelineBarrier& barrier, std::uint32_t descriptorSet)
{
    const std::size_t barrierResourceOffset = barrierSlots_.size()*descriptorSet;
    if (barrierResourceOffset >= barrierResources_.size())
        return /*Out of bounds*/;

    /* Set buffer barriers */
    for_range(i, numBufferBarriers_)
    {
        barrier.SetBufferBarrier(
            barrierSlots_[i],
            barrierResources_[barrierResourceOffset + i].buffer
        );
    }

    /* Set image barriers */
    for_range(i, numImageBarriers_)
    {
        barrier.SetImageBarrier(
            barrierSlots_[numBufferBarriers_ + i],
            barrierResources_[barrierResourceOffset + numBufferBarriers_ + i].image
        );
    }
}


/*
 * ======= Private: =======
 */

static bool IsDescriptorTypeImageView(VkDescriptorType type)
{
    return
    (
        type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
        type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    );
}

static bool IsDescriptorTypeBufferView(VkDescriptorType type)
{
    return
    (
        type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   ||
        type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   ||
        type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         ||
        type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         ||
        type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
        type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    );
}

void VKResourceHeap::ConvertAllLayoutBindings(const ArrayView<VKLayoutBinding>& layoutBindings)
{
    bindings_.resize(layoutBindings.size());
    for_range(i, layoutBindings.size())
        ConvertLayoutBinding(bindings_[i], layoutBindings[i]);
}

void VKResourceHeap::ConvertLayoutBinding(VKLayoutHeapBinding& dst, const VKLayoutBinding& src)
{
    dst.dstBinding      = src.dstBinding;
    dst.dstArrayElement = src.dstArrayElement;
    dst.barrierSlot     = src.barrierSlot;
    dst.descriptorType  = src.descriptorType;
    dst.stageFlags      = src.stageFlags;
    dst.imageViewIndex  = (IsDescriptorTypeImageView(src.descriptorType) ? numImageViewsPerSet_++ : VKResourceHeap::invalidViewIndex);
    dst.bufferViewIndex = (IsDescriptorTypeBufferView(src.descriptorType) ? numBufferViewsPerSet_++ : VKResourceHeap::invalidViewIndex);
}

void VKResourceHeap::CreateDescriptorPool(VkDevice device, std::uint32_t numDescriptorSets)
{
    /* Accumulate descriptor pool sizes */
    VKPoolSizeAccumulator poolSizeAccum;
    for (const VKLayoutHeapBinding& binding : bindings_)
    {
        if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            poolSizeAccum.Accumulate(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numDescriptorSets);
            poolSizeAccum.Accumulate(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numDescriptorSets);
            poolSizeAccum.Accumulate(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numDescriptorSets);
        }
        else
            poolSizeAccum.Accumulate(binding.descriptorType, numDescriptorSets);
    }
    poolSizeAccum.Finalize();

    /* Create Vulkan descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = numDescriptorSets;
        poolCreateInfo.poolSizeCount    = poolSizeAccum.Size();
        poolCreateInfo.pPoolSizes       = poolSizeAccum.Data();
    }
    VkResult result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool");
}

void VKResourceHeap::CreateDescriptorSets(
    VkDevice                device,
    std::uint32_t           numDescriptorSets,
    VkDescriptorSetLayout   globalSetLayout)
{
    /* Use copy of descritpor set layout for each descriptor set */
    vector<VkDescriptorSetLayout> setLayouts;
    setLayouts.resize(numDescriptorSets, globalSetLayout);

    /* Pre-allocate descriptor sets */
    descriptorSets_.resize(numDescriptorSets, VK_NULL_HANDLE);

    /* Allocate descriptor set */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_;
        allocInfo.descriptorSetCount    = numDescriptorSets;
        allocInfo.pSetLayouts           = setLayouts.data();
    }
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
}

void VKResourceHeap::FillWriteDescriptorWithSampler(
    const ResourceViewDescriptor&   desc,
    std::uint32_t                   descriptorSet,
    const VKLayoutHeapBinding&      binding,
    VKDescriptorSetWriter&          setWriter)
{
    auto* samplerVK = LLGL_CAST(VKSampler*, desc.resource);

    /* Initialize image information */
    VkDescriptorImageInfo* imageInfo = setWriter.NextImageInfo();
    {
        imageInfo->sampler          = samplerVK->GetVkSampler();
        imageInfo->imageView        = VK_NULL_HANDLE;
        imageInfo->imageLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    /* Initialize write descriptor */
    VkWriteDescriptorSet* writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[descriptorSet];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = binding.dstArrayElement;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::FillWriteDescriptorWithImageView(
    VkDevice                        device,
    const ResourceViewDescriptor&   desc,
    std::uint32_t                   descriptorSet,
    const VKLayoutHeapBinding&      binding,
    VKDescriptorSetWriter&          setWriter)
{
    auto* textureVK = LLGL_CAST(VKTexture*, desc.resource);

    /* Initialize image information */
    const std::size_t imageViewIndex = descriptorSet * numImageViewsPerSet_ + binding.imageViewIndex;
    VkDescriptorImageInfo* imageInfo = setWriter.NextImageInfo();
    {
        imageInfo->sampler       = VK_NULL_HANDLE;
        imageInfo->imageView     = GetOrCreateImageView(device, *textureVK, desc, imageViewIndex);
        imageInfo->imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //TODO: VK_IMAGE_LAYOUT_GENERAL for storage image
    }

    /* Initialize write descriptor */
    VkWriteDescriptorSet* writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[descriptorSet];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = binding.dstArrayElement;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }

    /* Write barrier slot */
    if (binding.barrierSlot < barrierSlots_.size())
    {
        switch (binding.descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                barrierResources_[barrierSlots_.size()*descriptorSet + binding.barrierSlot] = textureVK->GetVkImage();
                break;

            default:
                break;
        }
    }
}

void VKResourceHeap::FillWriteDescriptorWithBufferRange(
    VkDevice                        device,
    const ResourceViewDescriptor&   desc,
    std::uint32_t                   descriptorSet,
    const VKLayoutHeapBinding&      binding,
    VKDescriptorSetWriter&          setWriter)
{
    auto* bufferVK = LLGL_CAST(VKBuffer*, desc.resource);

    /* Initialize write descriptor */
    VkWriteDescriptorSet* writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[descriptorSet];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = binding.dstArrayElement;
        writeDesc->descriptorCount  = 1;
        writeDesc->pImageInfo       = nullptr;
        writeDesc->descriptorType   = binding.descriptorType;

        if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
            binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
        {
            //TODO: when a texel buffer is used, the set-layout must be updated as well.
            const std::size_t bufferViewIndex = descriptorSet * numBufferViewsPerSet_ + binding.bufferViewIndex;
            VkBufferView* bufferViewInfo = setWriter.NextBufferView();
            {
                *bufferViewInfo = GetOrCreateBufferView(device, *bufferVK, desc, bufferViewIndex);
            }
            writeDesc->pBufferInfo      = nullptr;
            writeDesc->pTexelBufferView = bufferViewInfo;
        }
        else
        {
            /* Initialize buffer information */
            VkDescriptorBufferInfo* bufferInfo = setWriter.NextBufferInfo();
            {
                bufferInfo->buffer = bufferVK->GetVkBuffer();
                if (desc.bufferView.size == LLGL_WHOLE_SIZE)
                {
                    bufferInfo->offset  = 0;
                    bufferInfo->range   = bufferVK->GetSize();
                }
                else
                {
                    bufferInfo->offset  = desc.bufferView.offset;
                    bufferInfo->range   = desc.bufferView.size;
                }
            }
            writeDesc->pBufferInfo      = bufferInfo;
            writeDesc->pTexelBufferView = nullptr;
        }
    }

    /* Write barrier slot */
    if (binding.barrierSlot < barrierSlots_.size())
    {
        switch (binding.descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                barrierResources_[barrierSlots_.size()*descriptorSet + binding.barrierSlot] = bufferVK->GetVkBuffer();
                break;

            default:
                break;
        }
    }
}

VkImageView VKResourceHeap::GetOrCreateImageView(
    VkDevice                        device,
    VKTexture&                      textureVK,
    const ResourceViewDescriptor&   desc,
    std::size_t                     imageViewIndex)
{
    if (IsTextureViewEnabled(desc.textureView))
    {
        /* Creates a new image view for the specified subresource descriptor */
        VKPtr<VkImageView> imageView{ device, vkDestroyImageView };
        textureVK.CreateImageView(device, desc.textureView, imageView);

        /* Remove previous image view entry */
        if (imageViewIndex < imageViews_.size() && imageViews_[imageViewIndex])
            imageViews_[imageViewIndex].Release();

        /* Increase image view container for new entry */
        if (imageViewIndex >= imageViews_.size())
            imageViews_.resize(imageViewIndex + 1);

        /* Emplace new image view entry */
        imageViews_[imageViewIndex] = std::move(imageView);
        return imageViews_[imageViewIndex].Get();
    }
    else
    {
        /* Remove previous image view entry */
        if (imageViewIndex < imageViews_.size() && imageViews_[imageViewIndex])
            imageViews_[imageViewIndex].Release();

        /* Return the standard image view */
        return textureVK.GetVkImageView();
    }
}

VkBufferView VKResourceHeap::GetOrCreateBufferView(
    VkDevice                        device,
    VKBuffer&                       bufferVK,
    const ResourceViewDescriptor&   desc,
    std::size_t                     bufferViewIndex)
{
    if (IsBufferViewEnabled(desc.bufferView))
    {
        /* Creates a new buffer view for the specified subresource descriptor */
        VKPtr<VkBufferView> bufferView{ device, vkDestroyBufferView };
        bufferVK.CreateBufferView(device, bufferView, static_cast<VkDeviceSize>(desc.bufferView.offset), static_cast<VkDeviceSize>(desc.bufferView.size));

        /* Remove previous buffer view entry */
        if (bufferViewIndex < bufferViews_.size() && bufferViews_[bufferViewIndex])
            bufferViews_[bufferViewIndex].Release();

        /* Increase buffer view container for new entry */
        if (bufferViewIndex >= bufferViews_.size())
            bufferViews_.resize(bufferViewIndex + 1);

        /* Emplace new buffer view entry */
        bufferViews_[bufferViewIndex] = std::move(bufferView);
        return bufferViews_[bufferViewIndex].Get();
    }
    else
    {
        /* Remove previous buffer view entry */
        if (bufferViewIndex < bufferViews_.size() && bufferViews_[bufferViewIndex])
            bufferViews_[bufferViewIndex].Release();

        /* Return the standard buffer view */
        LLGL_ASSERT(bufferVK.GetBufferView());
        return bufferVK.GetBufferView();
    }
}

void VKResourceHeap::AllocateBarrierSlots(std::uint32_t numDescriptorSets)
{
    /* Allocate all buffer barrier slots first */
    for (VKLayoutHeapBinding& binding : bindings_)
    {
        if (binding.barrierSlot != ~0u)
        {
            switch (binding.descriptorType)
            {
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    /* Move barrier slot and assign old field a new value as index to the barrier array */
                    barrierSlots_.push_back(binding.barrierSlot);
                    binding.barrierSlot = static_cast<std::uint32_t>(barrierSlots_.size());
                    ++numBufferBarriers_;
                    break;

                default:
                    break;
            }
        }
    }

    /* Allocate all imge barrier slots next */
    for (VKLayoutHeapBinding& binding : bindings_)
    {
        if (binding.barrierSlot != ~0u)
        {
            switch (binding.descriptorType)
            {
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    /* Move barrier slot and assign old field a new value as index to the barrier array */
                    barrierSlots_.push_back(binding.barrierSlot);
                    binding.barrierSlot = static_cast<std::uint32_t>(barrierSlots_.size());
                    ++numImageBarriers_;
                    break;

                default:
                    break;
            }
        }
    }

    /* Allocate barrier resource array */
    barrierResources_.resize(barrierSlots_.size()*numDescriptorSets);
}


} // /namespace LLGL



// ================================================================================
