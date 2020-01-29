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
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include <LLGL/ResourceHeapFlags.h>
#include <map>


namespace LLGL
{


/*
Returns the optimal pipeline binding point for the specified layout,
or VK_PIPELINE_BIND_POINT_MAX_ENUM if there are multiple pipelines in use.
*/
static VkPipelineBindPoint FindPipelineBindPoint(const VKPipelineLayout& pipelineLayout)
{
    for (const auto& binding : pipelineLayout.GetBindings())
    {
        if ((binding.stageFlags & StageFlags::AllGraphicsStages) != 0)
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        if ((binding.stageFlags & StageFlags::ComputeStage) != 0)
            return VK_PIPELINE_BIND_POINT_COMPUTE;
    }
    return VK_PIPELINE_BIND_POINT_MAX_ENUM;
}

VKResourceHeap::VKResourceHeap(const VKPtr<VkDevice>& device, const ResourceHeapDescriptor& desc) :
    descriptorPool_ { device, vkDestroyDescriptorPool }
{
    /* Get pipeline layout object */
    auto pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        throw std::invalid_argument("failed to create resource view heap due to missing pipeline layout");

    pipelineLayout_ = pipelineLayoutVK->GetVkPipelineLayout();
    bindPoint_      = FindPipelineBindPoint(*pipelineLayoutVK);

    /* Validate binding descriptors */
    const auto& bindings            = pipelineLayoutVK->GetBindings();
    const auto  numBindings         = bindings.size();
    const auto  numResourceViews    = desc.resourceViews.size();

    if (numBindings == 0)
        throw std::invalid_argument("cannot create resource heap without bindings in pipeline layout");
    if (numResourceViews % numBindings != 0)
        throw std::invalid_argument("failed to create resource heap because number of resource views is not a multiple of bindings in pipeline layou");

    /* Pre-allocate descriptor sets */
    const auto numDescriptorSets = (numResourceViews / numBindings);
    descriptorSets_.resize(numDescriptorSets, VK_NULL_HANDLE);

    /* Create resource descriptor pool */
    CreateDescriptorPool(device, desc, bindings);

    /* Create resource descriptor set for pipeline layout */
    std::vector<VkDescriptorSetLayout> setLayouts;
    setLayouts.resize(numDescriptorSets, pipelineLayoutVK->GetVkDescriptorSetLayout());
    CreateDescriptorSets(device, static_cast<std::uint32_t>(numDescriptorSets), setLayouts.data());

    /* Update write descriptors in descriptor set */
    UpdateDescriptorSets(device, desc, bindings);

    /* Create pipeline barrier for resource views that require it, e.g. those with storage binding flags */
    CreatePipelineBarrier(desc.resourceViews, pipelineLayoutVK->GetBindings());
}

std::uint32_t VKResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(descriptorSets_.size());
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

//TODO: enhance pool size accumulation
void VKResourceHeap::CreateDescriptorPool(
    const VKPtr<VkDevice>&              device,
    const ResourceHeapDescriptor&       desc,
    const std::vector<VKLayoutBinding>& bindings)
{
    /* Initialize descriptor pool sizes */
    const auto numResourceViews = desc.resourceViews.size();
    const auto numBindings      = bindings.size();

    std::vector<VkDescriptorPoolSize> poolSizes(numResourceViews);
    for (std::size_t i = 0; i < numResourceViews; ++i)
    {
        poolSizes[i].type               = bindings[i % numBindings].descriptorType;
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
        poolCreateInfo.maxSets          = GetNumDescriptorSets();
        poolCreateInfo.poolSizeCount    = static_cast<std::uint32_t>(poolSizes.size());
        poolCreateInfo.pPoolSizes       = poolSizes.data();
    }
    auto result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool");
}

void VKResourceHeap::CreateDescriptorSets(
    const VKPtr<VkDevice>&          device,
    std::uint32_t                   numSetLayouts,
    const VkDescriptorSetLayout*    setLayouts)
{
    /* Allocate descriptor set */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_;
        allocInfo.descriptorSetCount    = numSetLayouts;
        allocInfo.pSetLayouts           = setLayouts;
    }
    auto result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
}

void VKResourceHeap::UpdateDescriptorSets(
    const VKPtr<VkDevice>&              device,
    const ResourceHeapDescriptor&       desc,
    const std::vector<VKLayoutBinding>& bindings)
{
    /* Allocate local storage for buffer and image descriptors */
    const auto numResourceViews = desc.resourceViews.size();
    const auto numBindings      = bindings.size();

    VKWriteDescriptorContainer container{ numResourceViews };

    for (std::size_t i = 0; i < numResourceViews; ++i)
    {
        /* Get resource view information */
        const auto& binding = bindings[i % numBindings];
        const auto descriptorType = binding.descriptorType;

        const auto& rvDesc = desc.resourceViews[i];
        VkDescriptorSet descSet = descriptorSets_[i / numBindings];

        switch (descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                FillWriteDescriptorForSampler(rvDesc, descSet, binding, container);
                break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                FillWriteDescriptorForTexture(device, rvDesc, descSet, binding, container);
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                FillWriteDescriptorForBuffer(device, rvDesc, descSet, binding, container);
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
            device,
            container.numWriteDescriptors,      // Number of write descriptor
            container.writeDescriptors.data(),  // Descriptors to be written
            0,                                  // No copy descriptors
            nullptr                             // No descriptors to be copied
        );
    }
}

void VKResourceHeap::FillWriteDescriptorForSampler(
    const ResourceViewDescriptor&   rvDesc,
    VkDescriptorSet                 descSet,
    const VKLayoutBinding&          binding,
    VKWriteDescriptorContainer&     container)
{
    auto samplerVK = LLGL_CAST(VKSampler*, rvDesc.resource);

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
        writeDesc->dstSet           = descSet;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::FillWriteDescriptorForTexture(
    const VKPtr<VkDevice>&          device,
    const ResourceViewDescriptor&   rvDesc,
    VkDescriptorSet                 descSet,
    const VKLayoutBinding&          binding,
    VKWriteDescriptorContainer&     container)
{
    auto textureVK = LLGL_CAST(VKTexture*, rvDesc.resource);

    /* Initialize image information */
    auto imageInfo = container.NextImageInfo();
    {
        imageInfo->sampler       = VK_NULL_HANDLE;
        imageInfo->imageView     = GetOrCreateImageView(device, *textureVK, rvDesc);
        imageInfo->imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    /* Initialize write descriptor */
    auto writeDesc = container.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descSet;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = imageInfo;
        writeDesc->pBufferInfo      = nullptr;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::FillWriteDescriptorForBuffer(
    const VKPtr<VkDevice>&          /*device*/,
    const ResourceViewDescriptor&   rvDesc,
    VkDescriptorSet                 descSet,
    const VKLayoutBinding&          binding,
    VKWriteDescriptorContainer&     container)
{
    auto bufferVK = LLGL_CAST(VKBuffer*, rvDesc.resource);

    /* Initialize buffer information */
    auto bufferInfo = container.NextBufferInfo();
    {
        bufferInfo->buffer = bufferVK->GetVkBuffer();
        if (rvDesc.bufferView.size == Constants::wholeSize)
        {
            bufferInfo->offset  = 0;
            bufferInfo->range   = bufferVK->GetSize();
        }
        else
        {
            bufferInfo->offset  = rvDesc.bufferView.offset;
            bufferInfo->range   = rvDesc.bufferView.size;
        }
    }

    /* Initialize write descriptor */
    auto writeDesc = container.NextWriteDescriptor();
    {
        writeDesc->dstSet           = descSet;
        writeDesc->dstBinding       = binding.dstBinding;
        writeDesc->dstArrayElement  = 0;
        writeDesc->descriptorCount  = 1;
        writeDesc->descriptorType   = binding.descriptorType;
        writeDesc->pImageInfo       = nullptr;
        writeDesc->pBufferInfo      = bufferInfo;
        writeDesc->pTexelBufferView = nullptr;
    }
}

void VKResourceHeap::CreatePipelineBarrier(
    const std::vector<ResourceViewDescriptor>&  resourceViews,
    const std::vector<VKLayoutBinding>&         bindings)
{
    const auto numBindings = bindings.size();
    for (std::size_t i = 0; i < resourceViews.size(); ++i)
    {
        const auto& desc    = resourceViews[i];
        const auto& binding = bindings[i % numBindings];

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
                        binding.stageFlags,
                        VK_ACCESS_SHADER_WRITE_BIT,
                        VK_ACCESS_SHADER_READ_BIT
                    );
                }
            }
        }
    }
}

VkImageView VKResourceHeap::GetOrCreateImageView(
    const VKPtr<VkDevice>&          device,
    VKTexture&                      textureVK,
    const ResourceViewDescriptor&   rvDesc)
{
    if (IsTextureViewEnabled(rvDesc.textureView))
    {
        /* Creates a new image view for the specified subresource descriptor */
        imageViews_.emplace_back(device, vkDestroyImageView);
        textureVK.CreateImageView(device, rvDesc.textureView, imageViews_.back().ReleaseAndGetAddressOf());
        return imageViews_.back();
    }
    else
    {
        /* Returns the standard image view */
        return textureVK.GetVkImageView();
    }
}


} // /namespace LLGL



// ================================================================================
