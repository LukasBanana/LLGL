/*
 * VKPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineLayout.h"
#include "VKPoolSizeAccumulator.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../VKStaticLimits.h"
#include "../Texture/VKSampler.h"
#include "../Shader/VKShader.h"
#include "../Shader/VKShaderModulePool.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/SmallVector.h>
#include <algorithm>


namespace LLGL
{


VKPtr<VkPipelineLayout> VKPipelineLayout::defaultPipelineLayout_;

VKPipelineLayout::VKPipelineLayout(VkDevice device, const PipelineLayoutDescriptor& desc) :
    pipelineLayout_ { device, vkDestroyPipelineLayout          },
    setLayouts_     { { device, vkDestroyDescriptorSetLayout },
                      { device, vkDestroyDescriptorSetLayout },
                      { device, vkDestroyDescriptorSetLayout } },
    descriptorPool_ { device, vkDestroyDescriptorPool          },
    uniformDescs_   { desc.uniforms                            },
    barrierFlags_   { desc.barrierFlags                        }
{
    /* Create Vulkan descriptor set layouts */
    if (!desc.heapBindings.empty())
        CreateBindingSetLayout(device, desc.heapBindings, heapBindings_, SetLayoutType_HeapBindings);
    if (!desc.bindings.empty())
        CreateBindingSetLayout(device, desc.bindings, bindings_, SetLayoutType_DynamicBindings);
    if (!desc.staticSamplers.empty())
        CreateImmutableSamplers(device, desc.staticSamplers);

    /* Create descriptor pool for dynamic descriptors and immutable samplers */
    if (!desc.bindings.empty() || !desc.staticSamplers.empty())
        CreateDescriptorPool(device);
    if (!desc.bindings.empty())
        CreateDescriptorCache(device, setLayouts_[SetLayoutType_DynamicBindings].Get());
    if (!desc.staticSamplers.empty())
        CreateStaticDescriptorSet(device, setLayouts_[SetLayoutType_ImmutableSamplers].Get());

    /* Don't create a VkPipelineLayout object if this instance only has push constants as those are part of the permutations for each PSO */
    if (!desc.heapBindings.empty() || !desc.bindings.empty() || !desc.staticSamplers.empty())
    {
        BuildDescriptorSetBindingTables(desc);
        pipelineLayout_ = CreateVkPipelineLayout(device);
    }
}

VKPipelineLayout::~VKPipelineLayout()
{
    VKShaderModulePool::Get().NotifyReleasePipelineLayout(this);
}

std::uint32_t VKPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size());
}

std::uint32_t VKPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindings_.size());
}

std::uint32_t VKPipelineLayout::GetNumStaticSamplers() const
{
    return static_cast<std::uint32_t>(immutableSamplers_.size());
}

std::uint32_t VKPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniformDescs_.size());
}

// Builds one push-constant range for each uniform but with convoluted stage flags.
static void BuildPushConstantRanges(
    const ArrayView<Shader*>&           shaders,
    const ArrayView<UniformDescriptor>& uniformDescs,
    std::vector<VkPushConstantRange>&   outStageRanges,
    std::vector<VkPushConstantRange>&   outUniformRanges)
{
    /* Reflect all push constant ranges */
    SmallVector<std::vector<VKUniformRange>, LLGL_VK_MAX_NUM_PSO_SHADER_STAGES> uniformRanges;
    uniformRanges.resize(shaders.size());

    for_range(i, shaders.size())
    {
        auto* shaderVK = LLGL_CAST(VKShader*, shaders[i]);
        shaderVK->ReflectPushConstants(uniformDescs, uniformRanges[i]);
    }

    /* Consolidate push constant ranges across all shader stages */
    SmallVector<VKUniformRange, LLGL_VK_MAX_NUM_PSO_SHADER_STAGES> pushConstantBlockRanges;

    outUniformRanges.resize(uniformDescs.size());
    outStageRanges.resize(shaders.size());
    pushConstantBlockRanges.resize(shaders.size());

    for_range(i, shaders.size())
    {
        outStageRanges[i].stageFlags = VKTypes::Map(shaders[i]->GetType());

        pushConstantBlockRanges[i].offset = ~0u;

        LLGL_ASSERT(uniformRanges[i].size() == outUniformRanges.size());
        for_range(j, outUniformRanges.size())
        {
            outUniformRanges[j].stageFlags |= outStageRanges[i].stageFlags;

            //TODO: shader permutations must be generated if uniforms have different offsets between stages
            const std::uint32_t stageUniformOffset = uniformRanges[i][j].offset;
            if (stageUniformOffset != 0)
            {
                if (!(outUniformRanges[j].offset == 0 || outUniformRanges[j].offset == stageUniformOffset))
                {
                    LLGL_TRAP(
                        "cannot handle different push constant offsets between shader stages for uniform '%s'; got %u and %u",
                        uniformDescs[j].name.c_str(), outUniformRanges[j].offset, stageUniformOffset
                    );
                }
                outUniformRanges[j].offset = stageUniformOffset;
            }

            const std::uint32_t stageUniformSize = uniformRanges[i][j].size;
            if (stageUniformSize != 0)
            {
                if (!(outUniformRanges[j].size == 0 || outUniformRanges[j].size == stageUniformSize))
                {
                    LLGL_TRAP(
                        "cannot handle different push constant sizes between shader stages for uniform '%s'; got %u and %u",
                        uniformDescs[j].name.c_str(), outUniformRanges[j].size, stageUniformSize
                    );
                }
                outUniformRanges[j].size = stageUniformSize;
            }

            /* Use offset and size as start and end pointers and resolve size after all elements are inserted into the range */
            pushConstantBlockRanges[i].offset   = std::min(outUniformRanges[j].offset, pushConstantBlockRanges[i].offset);
            pushConstantBlockRanges[i].size     = std::max(outUniformRanges[j].offset + outUniformRanges[j].size, pushConstantBlockRanges[i].size);
        }

        outStageRanges[i].offset    = pushConstantBlockRanges[i].offset;
        outStageRanges[i].size      = pushConstantBlockRanges[i].size;
    }

    /* Remove empty stage ranges */
    RemoveFromListIf(
        outStageRanges,
        [](const VkPushConstantRange& range) -> bool
        {
            return (range.size == 0);
        }
    );
}

VKPtr<VkPipelineLayout> VKPipelineLayout::CreateVkPipelineLayoutPermutation(
    VkDevice                            device,
    const ArrayView<Shader*>&           shaders,
    std::vector<VkPushConstantRange>&   outUniformRanges) const
{
    #if LLGL_VK_ENABLE_SPIRV_REFLECT
    if (!uniformDescs_.empty())
    {
        std::vector<VkPushConstantRange> pushConstantRangesPerStage;
        BuildPushConstantRanges(shaders, uniformDescs_, pushConstantRangesPerStage, outUniformRanges);
        return CreateVkPipelineLayout(device, pushConstantRangesPerStage);
    }
    #else
    LLGL_ASSERT(uniformDescs_.empty(), "uniform descriptors in Vulkan PSO layout but LLGL was not compiled with LLGL_VK_ENABLE_SPIRV_REFLECT");
    #endif
    return {};
}

//private
bool VKPipelineLayout::GetBindingSlotsAssignment(
    unsigned                                index,
    ConstFieldRangeIterator<BindingSlot>&   iter,
    std::uint32_t&                          dstSet) const
{
    if (index < layoutTypeOrder_.Count())
    {
        const DescriptorSetBindingTable& bindingTable = setBindingTables_[layoutTypeOrder_[index]];
        dstSet  = bindingTable.dstSet;
        iter    = ConstFieldRangeIterator<BindingSlot>{ bindingTable.srcSlots.data(), bindingTable.srcSlots.size() };
        return true;
    }
    return false;
}

bool VKPipelineLayout::NeedsShaderModulePermutation(const VKShader& shaderVK) const
{
    return shaderVK.NeedsShaderModulePermutation(
        std::bind(&VKPipelineLayout::GetBindingSlotsAssignment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
}

VKPtr<VkShaderModule> VKPipelineLayout::CreateVkShaderModulePermutation(VKShader& shaderVK) const
{
    return shaderVK.CreateVkShaderModulePermutation(
        std::bind(&VKPipelineLayout::GetBindingSlotsAssignment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
}

void VKPipelineLayout::CreateDefault(VkDevice device)
{
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    {
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    }
    VKPipelineLayout::defaultPipelineLayout_ = VKPtr<VkPipelineLayout>{ device, vkDestroyPipelineLayout };
    VkResult result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, VKPipelineLayout::defaultPipelineLayout_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan default pipeline layout");
}

void VKPipelineLayout::ReleaseDefault()
{
    VKPipelineLayout::defaultPipelineLayout_.Release();
}

VkPipelineLayout VKPipelineLayout::GetDefault()
{
    return VKPipelineLayout::defaultPipelineLayout_.Get();
}


/*
 * ======= Private: =======
 */

// Converts the bitmask of LLGL::StageFlags to VkShaderStageFlags
static VkShaderStageFlags GetVkShaderStageFlags(long flags)
{
    VkShaderStageFlags bitmask = 0;

    if ((flags & StageFlags::VertexStage        ) != 0) { bitmask |= VK_SHADER_STAGE_VERTEX_BIT;                  }
    if ((flags & StageFlags::TessControlStage   ) != 0) { bitmask |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;    }
    if ((flags & StageFlags::TessEvaluationStage) != 0) { bitmask |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
    if ((flags & StageFlags::GeometryStage      ) != 0) { bitmask |= VK_SHADER_STAGE_GEOMETRY_BIT;                }
    if ((flags & StageFlags::FragmentStage      ) != 0) { bitmask |= VK_SHADER_STAGE_FRAGMENT_BIT;                }
    if ((flags & StageFlags::ComputeStage       ) != 0) { bitmask |= VK_SHADER_STAGE_COMPUTE_BIT;                 }

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
            if ((desc.bindFlags & (BindFlags::Storage)) != 0)
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            else
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;

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
    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, setLayouts_[setLayoutType].ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor set layout");
}

static void ConvertBindingDesc(VkDescriptorSetLayoutBinding& dst, const BindingDescriptor& src)
{
    dst.binding             = src.slot.index;
    dst.descriptorType      = GetVkDescriptorType(src);
    dst.descriptorCount     = std::max(1u, src.arraySize);
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = nullptr;
}

void VKPipelineLayout::CreateBindingSetLayout(
    VkDevice                                device,
    const std::vector<BindingDescriptor>&   inBindings,
    std::vector<VKLayoutBinding>&           outBindings,
    SetLayoutType                           setLayoutType)
{
    /* Convert heap bindings to native descriptor set layout bindings and create Vulkan descriptor set layout */
    const std::size_t numBindings = inBindings.size();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(numBindings);

    for_range(i, numBindings)
        ConvertBindingDesc(setLayoutBindings[i], inBindings[i]);

    CreateVkDescriptorSetLayout(device, setLayoutType, setLayoutBindings);

    /* Create list of binding points (for later pass to 'VkWriteDescriptorSet::dstBinding') */
    outBindings.reserve(numBindings);
    for_range(i, numBindings)
    {
        for_range(arrayElement, setLayoutBindings[i].descriptorCount)
        {
            outBindings.push_back(
                VKLayoutBinding
                {
                    /*dstBinding:*/         inBindings[i].slot.index,
                    /*dstArrayElement:*/    arrayElement,
                    /*stageFlags:*/         inBindings[i].stageFlags,
                    /*descriptorType:*/     setLayoutBindings[i].descriptorType
                }
            );
        }
    }
}

static void ConvertImmutableSamplerDesc(VkDescriptorSetLayoutBinding& dst, const StaticSamplerDescriptor& src, const VkSampler* immutableSamplerVK)
{
    dst.binding             = src.slot.index;
    dst.descriptorType      = VK_DESCRIPTOR_TYPE_SAMPLER;
    dst.descriptorCount     = 1u;
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = immutableSamplerVK;
}

void VKPipelineLayout::CreateImmutableSamplers(VkDevice device, const ArrayView<StaticSamplerDescriptor>& staticSamplers)
{
    /* Create all immutable Vulkan samplers */
    immutableSamplers_.reserve(staticSamplers.size());
    for (const StaticSamplerDescriptor& staticSamplerDesc : staticSamplers)
        immutableSamplers_.push_back(VKSampler::CreateVkSampler(device, staticSamplerDesc.sampler));

    /* Convert heap bindings to native descriptor set layout bindings and create Vulkan descriptor set layout */
    const std::size_t numBindings = staticSamplers.size();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(numBindings);

    for_range(i, numBindings)
        ConvertImmutableSamplerDesc(setLayoutBindings[i], staticSamplers[i], immutableSamplers_[i].GetAddressOf());

    CreateVkDescriptorSetLayout(device, SetLayoutType_ImmutableSamplers, setLayoutBindings);
}

VKPtr<VkPipelineLayout> VKPipelineLayout::CreateVkPipelineLayout(VkDevice device, const ArrayView<VkPushConstantRange>& pushConstantRanges) const
{
    /* Create native Vulkan pipeline layout with up to 3 descriptor sets */
    SmallVector<VkDescriptorSetLayout, SetLayoutType_Num> setLayoutsVK;

    for_range(i, SetLayoutType_Num)
    {
        if (setLayouts_[i].Get() != VK_NULL_HANDLE)
            setLayoutsVK.push_back(setLayouts_[i].Get());
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
    VkResult result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, pipelineLayout.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan pipeline layout");
    return pipelineLayout;
}

void VKPipelineLayout::CreateDescriptorPool(VkDevice device)
{
    /* Accumulate descriptor pool sizes for all dynamic resources and immutable samplers */
    VKPoolSizeAccumulator poolSizeAccum;

    for (const VKLayoutBinding& binding : bindings_)
        poolSizeAccum.Accumulate(binding.descriptorType);

    if (!immutableSamplers_.empty())
        poolSizeAccum.Accumulate(VK_DESCRIPTOR_TYPE_SAMPLER, static_cast<std::uint32_t>(immutableSamplers_.size()));

    poolSizeAccum.Finalize();

    /* Create Vulkan descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = 2;
        poolCreateInfo.poolSizeCount    = poolSizeAccum.Size();
        poolCreateInfo.pPoolSizes       = poolSizeAccum.Data();
    }
    VkResult result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool for static samplers");
}

void VKPipelineLayout::CreateDescriptorCache(VkDevice device, VkDescriptorSetLayout setLayout)
{
    /*
    Don't account descriptors in the dynamic cache for immutable samplers,
    so accumulate pool sizes only for dynamiuc resources here
    */
    VKPoolSizeAccumulator poolSizeAccum;
    for (const VKLayoutBinding& binding : bindings_)
        poolSizeAccum.Accumulate(binding.descriptorType);
    poolSizeAccum.Finalize();

    /* Allocate unique descriptor cache */
    descriptorCache_ = MakeUnique<VKDescriptorCache>(device, descriptorPool_, setLayout, poolSizeAccum.Size(), poolSizeAccum.Data(), bindings_);
}

void VKPipelineLayout::CreateStaticDescriptorSet(VkDevice device, VkDescriptorSetLayout setLayout)
{
    /* Allocate descriptor set for immutable samplers */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_.Get();
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = &setLayout;
    }
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &staticDescriptorSet_);
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
}

void VKPipelineLayout::BuildDescriptorSetBindingTables(const PipelineLayoutDescriptor& desc)
{
    /* Assign binding slots for all descrioptor set layouts, i.e. 'layout(set = N)' in SPIR-V code */
    for_range(i, SetLayoutType_Num)
    {
        if (setLayouts_[i].Get() != VK_NULL_HANDLE)
        {
            setBindingTables_[i].dstSet = layoutTypeOrder_.Count();
            layoutTypeOrder_.Append(static_cast<std::uint8_t>(i));
        }
    }

    /* Build binding table slots */
    BuildDescriptorSetBindingSlots(setBindingTables_[SetLayoutType_HeapBindings], desc.heapBindings);
    BuildDescriptorSetBindingSlots(setBindingTables_[SetLayoutType_DynamicBindings], desc.bindings);
    BuildDescriptorSetBindingSlots(setBindingTables_[SetLayoutType_ImmutableSamplers], desc.staticSamplers);
}

template <typename TContainer>
void VKPipelineLayout::BuildDescriptorSetBindingSlots(DescriptorSetBindingTable& dst, const TContainer& src)
{
    if (!src.empty())
    {
        dst.srcSlots.resize(src.size());
        for_range(i, src.size())
            dst.srcSlots[i] = src[i].slot;
    }
}


} // /namespace LLGL



// ================================================================================
