/*
 * VKPipelineLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineLayout.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../Texture/VKSampler.h"
#include <LLGL/Misc/ForRange.h>
#include <LLGL/Container/SmallVector.h>
#include <algorithm>


namespace LLGL
{


// Converts the bitmask of LLGL::StageFlags to VkShaderStageFlags
static VkShaderStageFlags GetVkShaderStageFlags(long flags)
{
    VkShaderStageFlags bitmask = 0;

    if ((flags & StageFlags::VertexStage) != 0)
        bitmask |= VK_SHADER_STAGE_VERTEX_BIT;
    if ((flags & StageFlags::TessControlStage) != 0)
        bitmask |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if ((flags & StageFlags::TessEvaluationStage) != 0)
        bitmask |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if ((flags & StageFlags::GeometryStage) != 0)
        bitmask |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if ((flags & StageFlags::FragmentStage) != 0)
        bitmask |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if ((flags & StageFlags::ComputeStage) != 0)
        bitmask |= VK_SHADER_STAGE_COMPUTE_BIT;

    return bitmask;
}

// Returns the appropriate VkDescriptorType enum entry for the specified binding descriptor
static VkDescriptorType GetVkDescriptorType(const BindingDescriptor& desc)
{
    switch (desc.type)
    {
        case ResourceType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ResourceType::Texture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case ResourceType::Buffer:
            if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            if ((desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        default:
            break;
    }
    VKTypes::MapFailed("ResourceType", "VkDescriptorType");
}

VKPipelineLayout::VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc) :
    pipelineLayout_       { device, vkDestroyPipelineLayout          },
    descriptorSetLayouts_ { { device, vkDestroyDescriptorSetLayout },
                            { device, vkDestroyDescriptorSetLayout },
                            { device, vkDestroyDescriptorSetLayout } },
    uniformDescs_         { desc.uniforms                            }
{
    /* Create Vulkan descriptor set layouts */
    if (!desc.heapBindings.empty())
        CreateHeapBindingSetLayout(device, desc.heapBindings);
    if (!desc.bindings.empty())
        CreateDynamicBindingSetLayout(device, desc.bindings);
    if (!desc.staticSamplers.empty())
        CreateImmutableSamplers(device, desc.staticSamplers);

    /* Don't create a VkPipelineLayout object if this instance only has push constants as those are part of the permutations for each PSO */
    if (!desc.heapBindings.empty() || !desc.bindings.empty() || !desc.staticSamplers.empty())
        pipelineLayout_ = CreateVkPipelineLayout(device);
}

std::uint32_t VKPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size());
}

std::uint32_t VKPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(0); //TODO
}

std::uint32_t VKPipelineLayout::GetNumStaticSamplers() const
{
    return static_cast<std::uint32_t>(immutableSamplers_.size());
}

std::uint32_t VKPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniformDescs_.size());
}


/*
 * ======= Private: =======
 */

void VKPipelineLayout::CreateVkDescriptorSetLayout(
    VkDevice                                        device,
    SetLayoutType                                   setLayoutType,
    const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings)
{
    VkDescriptorSetLayoutCreateInfo createInfo;
    {
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = static_cast<std::uint32_t>(setLayoutBindings.size());
        createInfo.pBindings    = setLayoutBindings.data();
    }
    auto result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, descriptorSetLayouts_[setLayoutType].ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor set layout");
}

static void Convert(VkDescriptorSetLayoutBinding& dst, const BindingDescriptor& src)
{
    dst.binding             = src.slot;
    dst.descriptorType      = GetVkDescriptorType(src);
    dst.descriptorCount     = std::max(1u, src.arraySize);
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = nullptr;
}

void VKPipelineLayout::CreateHeapBindingSetLayout(const VKPtr<VkDevice>& device, const std::vector<BindingDescriptor>& heapBindings)
{
    /* Convert heap bindings to native descriptor set layout bindings and create Vulkan descriptor set layout */
    const auto numBindings = heapBindings.size();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(numBindings);

    for_range(i, numBindings)
        Convert(setLayoutBindings[i], heapBindings[i]);

    CreateVkDescriptorSetLayout(device, SetLayoutType_HeapBindings, setLayoutBindings);

    /* Create list of binding points (for later pass to 'VkWriteDescriptorSet::dstBinding') */
    heapBindings_.reserve(numBindings);
    for_range(i, numBindings)
    {
        heapBindings_.push_back(
            VKLayoutBinding
            {
                heapBindings[i].slot,
                heapBindings[i].stageFlags,
                setLayoutBindings[i].descriptorType
            }
        );
    }
}

void VKPipelineLayout::CreateDynamicBindingSetLayout(const VKPtr<VkDevice>& device, const std::vector<BindingDescriptor>& bindings)
{
    //todo
}

static void Convert(VkDescriptorSetLayoutBinding& dst, const StaticSamplerDescriptor& src, const VkSampler* immutableSamplerVK)
{
    dst.binding             = src.slot;
    dst.descriptorType      = VK_DESCRIPTOR_TYPE_SAMPLER;
    dst.descriptorCount     = 1u;
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = immutableSamplerVK;
}

void VKPipelineLayout::CreateImmutableSamplers(const VKPtr<VkDevice>& device, const std::vector<StaticSamplerDescriptor>& staticSamplers)
{
    /* Create all immutable Vulkan samplers */
    immutableSamplers_.reserve(staticSamplers.size());
    for (const auto& staticSamplerDesc : staticSamplers)
        immutableSamplers_.push_back(VKSampler::CreateVkSampler(device, staticSamplerDesc.sampler));

    /* Convert heap bindings to native descriptor set layout bindings and create Vulkan descriptor set layout */
    const auto numBindings = staticSamplers.size();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(numBindings);

    for_range(i, numBindings)
        Convert(setLayoutBindings[i], staticSamplers[i], &(immutableSamplers_[i]));

    CreateVkDescriptorSetLayout(device, SetLayoutType_ImmutableSamplers, setLayoutBindings);
}

VKPtr<VkPipelineLayout> VKPipelineLayout::CreateVkPipelineLayout(
    const VKPtr<VkDevice>&                  device,
    const ArrayView<VkPushConstantRange>&   pushConstantRanges)
{
    /* Create native Vulkan pipeline layout with up to 3 descriptor sets */
    SmallVector<VkDescriptorSetLayout, 3> setLayoutsVK;

    for (const auto& setLayout : descriptorSetLayouts_)
    {
        if (setLayout.Get() != VK_NULL_HANDLE)
            setLayoutsVK.push_back(setLayout.Get());
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo;
    {
        layoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pNext                      = nullptr;
        layoutCreateInfo.flags                      = 0;
        layoutCreateInfo.setLayoutCount             = static_cast<std::uint32_t>(setLayoutsVK.size());
        layoutCreateInfo.pSetLayouts                = setLayoutsVK.data();
        if (pushConstantRanges.empty())
        {
            layoutCreateInfo.pushConstantRangeCount = 0;
            layoutCreateInfo.pPushConstantRanges    = nullptr;
        }
        else
        {
            layoutCreateInfo.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size());
            layoutCreateInfo.pPushConstantRanges    = pushConstantRanges.data();
        }
    }
    VKPtr<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
    auto result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, pipelineLayout.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan pipeline layout");
    return pipelineLayout;
}


} // /namespace LLGL



// ================================================================================
