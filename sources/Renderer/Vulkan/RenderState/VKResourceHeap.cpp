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
#include <map>


namespace LLGL
{


VKResourceHeap::VKResourceHeap(
    VkDevice                                    device,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
:
    descriptorPool_ { device, vkDestroyDescriptorPool }
{
    /* Get pipeline layout object */
    auto* pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        LLGL_TRAP("failed to create resource view heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    ConvertLayoutBindings(pipelineLayoutVK->GetLayoutHeapBindings());

    const std::uint32_t numBindings         = static_cast<std::uint32_t>(bindings_.size());
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    /* Create descriptor pool and array of descriptor sets */
    const std::uint32_t numDescriptorSets = (numResourceViews / numBindings);
    CreateDescriptorPool(device, numDescriptorSets);
    CreateDescriptorSets(device, numDescriptorSets, pipelineLayoutVK->GetSetLayoutForHeapBindings());

    /* Allocate array for descriptor set barriers */
    if ((pipelineLayoutVK->GetBarrierFlags() & BarrierFlags::Storage) != 0)
        barriers_.resize(numDescriptorSets);

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
    VKDescriptorBarrierWriter barrierWriter;

    for (const ResourceViewDescriptor& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get resource view information */
        const VKDescriptorBinding& binding = bindings_[firstDescriptor % numBindings];

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
                FillWriteDescriptorWithBufferRange(device, desc, descriptorSet, binding, setWriter, barrierWriter);
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

    /* Update pipeline barriers */
    for_subrange(i, barrierWriter.barrierChangeRanges[0], barrierWriter.barrierChangeRanges[1])
    {
        if (VKPipelineBarrier* barrier = barriers_[i].get())
        {
            if (!barrier->Update())
                barriers_[i].reset();
        }
    }

    return setWriter.GetNumWrites();
}

void VKResourceHeap::SubmitPipelineBarrier(VkCommandBuffer commandBuffer, std::uint32_t descriptorSet)
{
    if (descriptorSet < barriers_.size())
    {
        if (VKPipelineBarrier* barrier = barriers_[descriptorSet].get())
        {
            if (barrier->IsActive())
                barrier->Submit(commandBuffer);
        }
    }
}


/*
 * ======= Private: =======
 */

static VkPipelineStageFlags ToVkStageFlags(long stageFlags)
{
    VkPipelineStageFlags bitmask = 0;

    if ((stageFlags & StageFlags::VertexStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if ((stageFlags & StageFlags::TessControlStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    if ((stageFlags & StageFlags::TessEvaluationStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    if ((stageFlags & StageFlags::GeometryStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if ((stageFlags & StageFlags::ComputeStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    return bitmask;
}

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

void VKResourceHeap::ConvertLayoutBindings(const ArrayView<VKLayoutBinding>& layoutBindings)
{
    bindings_.resize(layoutBindings.size());
    for_range(i, layoutBindings.size())
        ConvertLayoutBinding(bindings_[i], layoutBindings[i]);
}

void VKResourceHeap::ConvertLayoutBinding(VKDescriptorBinding& dst, const VKLayoutBinding& src)
{
    dst.dstBinding      = src.dstBinding;
    dst.dstArrayElement = src.dstArrayElement;
    dst.descriptorType  = src.descriptorType;
    dst.stageFlags      = ToVkStageFlags(src.stageFlags);
    dst.imageViewIndex  = (IsDescriptorTypeImageView(src.descriptorType) ? numImageViewsPerSet_++ : VKResourceHeap::invalidViewIndex);
    dst.bufferViewIndex = (IsDescriptorTypeBufferView(src.descriptorType) ? numBufferViewsPerSet_++ : VKResourceHeap::invalidViewIndex);
}

void VKResourceHeap::CreateDescriptorPool(VkDevice device, std::uint32_t numDescriptorSets)
{
    /* Accumulate descriptor pool sizes */
    VKPoolSizeAccumulator poolSizeAccum;
    for (const VKDescriptorBinding& binding : bindings_)
        poolSizeAccum.Accumulate(binding.descriptorType, numDescriptorSets);
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
    std::vector<VkDescriptorSetLayout> setLayouts;
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
    const VKDescriptorBinding&      binding,
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
    const VKDescriptorBinding&      binding,
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
}

void VKResourceHeap::FillWriteDescriptorWithBufferRange(
    VkDevice                        /*device*/,
    const ResourceViewDescriptor&   desc,
    std::uint32_t                   descriptorSet,
    const VKDescriptorBinding&      binding,
    VKDescriptorSetWriter&          setWriter,
    VKDescriptorBarrierWriter&      barrierWriter)
{
    auto* bufferVK = LLGL_CAST(VKBuffer*, desc.resource);

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

    /* Initialize write descriptor */
    VkWriteDescriptorSet* writeDesc = setWriter.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descriptorSets_[descriptorSet];
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = binding.dstArrayElement;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = nullptr;
        writeDesc->pBufferInfo      = bufferInfo;
        writeDesc->pTexelBufferView = nullptr;
    }

    /* Emplace pipeline barrier for storage buffer */
    if (ExchangeBufferBarrier(descriptorSet, bufferVK, binding))
    {
        barrierWriter.barrierChangeRanges[0] = std::min(barrierWriter.barrierChangeRanges[0], descriptorSet);
        barrierWriter.barrierChangeRanges[1] = std::max(barrierWriter.barrierChangeRanges[1], descriptorSet + 1);
    }
}

bool VKResourceHeap::ExchangeBufferBarrier(std::uint32_t descriptorSet, Buffer* resource, const VKDescriptorBinding& binding)
{
    if (descriptorSet < barriers_.size())
    {
        if ((resource->GetBindFlags() & BindFlags::Storage) != 0)
            return EmplaceBarrier(descriptorSet, binding.dstBinding, resource, binding.stageFlags);
        else
            return RemoveBarrier(descriptorSet, binding.dstBinding);
    }
    return false;
}

bool VKResourceHeap::EmplaceBarrier(std::uint32_t descriptorSet, std::uint32_t slot, Resource* resource, VkPipelineStageFlags stageFlags)
{
    if (VKPipelineBarrier* barrier = barriers_[descriptorSet].get())
    {
        /* Emplace into existing pipeline barrier */
        return barrier->Emplace(slot, resource, stageFlags);
    }
    else
    {
        /* Allocate new pipeline barrier for descriptor set */
        VKPipelineBarrierPtr newBarrier = MakeUnique<VKPipelineBarrier>();
        newBarrier->Emplace(slot, resource, stageFlags);
        barriers_[descriptorSet] = std::move(newBarrier);
        return true;
    }
}

bool VKResourceHeap::RemoveBarrier(std::uint32_t descriptorSet, std::uint32_t slot)
{
    if (VKPipelineBarrier* barrier = barriers_[descriptorSet].get())
        return barrier->Remove(slot);
    else
        return false;
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

        /* Returns the standard image view */
        return textureVK.GetVkImageView();
    }
}


} // /namespace LLGL



// ================================================================================
