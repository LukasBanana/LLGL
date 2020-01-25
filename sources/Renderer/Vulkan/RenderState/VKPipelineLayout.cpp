/*
 * VKPipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineLayout.h"
#include "../VKTypes.h"
#include "../VKCore.h"


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

//TODO:
// looks like 'VkDescriptorSetLayoutBinding::descriptorCount' can only be greater than 1
// for arrays in a shader (e.g. array of uniform buffers), but not for multiple binding points.
static void Convert(VkDescriptorSetLayoutBinding& dst, const BindingDescriptor& src)
{
    dst.binding             = src.slot;
    dst.descriptorType      = GetVkDescriptorType(src);
    dst.descriptorCount     = src.arraySize;
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = nullptr;
}

/*static void Convert(VkDescriptorPoolSize& dst, const BindingDescriptor& src)
{
    dst.type            = VKTypes::Map(src.type);
    dst.descriptorCount = src.numSlots;
}*/

/*
TODO:
maybe move the VkPipelineLayout object into "VKGraphicsPipeline",
in this case the "PipelineLayout" interface might need a renaming
*/
VKPipelineLayout::VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc) :
    pipelineLayout_      { device, vkDestroyPipelineLayout      },
    descriptorSetLayout_ { device, vkDestroyDescriptorSetLayout }
{
    /* Initialize all descriptor-set layout bindings */
    const auto numBindings = desc.bindings.size();
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(numBindings);

    for (std::size_t i = 0; i < numBindings; ++i)
        Convert(layoutBindings[i], desc.bindings[i]);

    /* Create descriptor set layout */
    VkDescriptorSetLayoutCreateInfo descSetCreateInfo;
    {
        descSetCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descSetCreateInfo.pNext         = nullptr;
        descSetCreateInfo.flags         = 0;
        descSetCreateInfo.bindingCount  = static_cast<std::uint32_t>(layoutBindings.size());
        descSetCreateInfo.pBindings     = layoutBindings.data();
    }
    auto result = vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, descriptorSetLayout_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor set layout");

    /* Create pipeline layout */
    VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout_.Get() };

    VkPipelineLayoutCreateInfo layoutCreateInfo;
    {
        layoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pNext                  = nullptr;
        layoutCreateInfo.flags                  = 0;
        layoutCreateInfo.setLayoutCount         = 1;
        layoutCreateInfo.pSetLayouts            = setLayouts;
        layoutCreateInfo.pushConstantRangeCount = 0;
        layoutCreateInfo.pPushConstantRanges    = nullptr;
    }
    result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, pipelineLayout_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan pipeline layout");

    /* Create list of binding points (for later pass to 'VkWriteDescriptorSet::dstBinding') */
    bindings_.reserve(numBindings);
    for (std::size_t i = 0; i < numBindings; ++i)
    {
        bindings_.push_back(
            {
                desc.bindings[i].slot,
                desc.bindings[i].stageFlags,
                layoutBindings[i].descriptorType
            }
        );
    }
}

std::uint32_t VKPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindings_.size());
}


} // /namespace LLGL



// ================================================================================
