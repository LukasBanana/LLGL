/*
 * VKPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineLayout.h"
#include "VKPipelineLayoutPermutationPool.h"
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
    pipelineLayout_             { device, vkDestroyPipelineLayout      },
    setLayoutHeapBindings_      { device                               },
    setLayoutDynamicBindings_   { device                               },
    setLayoutImmutableSamplers_ { device, vkDestroyDescriptorSetLayout },
    descriptorPool_             { device, vkDestroyDescriptorPool      },
    uniformDescs_               { desc.uniforms                        },
    barrierFlags_               { desc.barrierFlags                    },
    flags_                      { 0                                    }
{
    /* Create Vulkan descriptor set layouts */
    if (!desc.heapBindings.empty())
        CreateDescriptorSetLayout(device, desc.heapBindings, bindingTable_.heapBindings, setLayoutHeapBindings_);
    if (!desc.bindings.empty())
        CreateDescriptorSetLayout(device, desc.bindings, bindingTable_.dynamicBindings, setLayoutDynamicBindings_);
    if (!desc.staticSamplers.empty())
        CreateImmutableSamplers(device, desc.staticSamplers);

    /* Create descriptor pool for dynamic descriptors and immutable samplers */
    if (!desc.bindings.empty() || !desc.staticSamplers.empty())
        CreateDescriptorPool(device);
    if (!desc.bindings.empty())
        CreateDescriptorCache(device, setLayoutDynamicBindings_.GetVkDescriptorSetLayout());
    if (!desc.staticSamplers.empty())
        CreateStaticDescriptorSet(device, setLayoutImmutableSamplers_.Get());

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
    return static_cast<std::uint32_t>(bindingTable_.heapBindings.size());
}

std::uint32_t VKPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindingTable_.dynamicBindings.size());
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

    for_range(shaderIndex, shaders.size())
    {
        outStageRanges[shaderIndex].stageFlags = VKTypes::Map(shaders[shaderIndex]->GetType());

        pushConstantBlockRanges[shaderIndex].offset = ~0u;

        LLGL_ASSERT(uniformRanges[shaderIndex].size() == outUniformRanges.size());
        for_range(uniformIndex, outUniformRanges.size())
        {
            //TODO: shader permutations must be generated if uniforms have different offsets between stages
            const std::uint32_t stageUniformOffset = uniformRanges[shaderIndex][uniformIndex].offset;
            if (stageUniformOffset != 0)
            {
                if (!(outUniformRanges[uniformIndex].offset == 0 || outUniformRanges[uniformIndex].offset == stageUniformOffset))
                {
                    LLGL_TRAP(
                        "cannot handle different push constant offsets between shader stages for uniform '%s'; got %u and %u",
                        uniformDescs[uniformIndex].name.c_str(), outUniformRanges[uniformIndex].offset, stageUniformOffset
                    );
                }
                outUniformRanges[uniformIndex].offset = stageUniformOffset;
            }

            const std::uint32_t stageUniformSize = uniformRanges[shaderIndex][uniformIndex].size;
            if (stageUniformSize != 0)
            {
                /* Add current shader stage flag to merged flags only if this push constant range is assigned, i.e. its size is non-zero */
                outUniformRanges[uniformIndex].stageFlags |= outStageRanges[shaderIndex].stageFlags;

                if (!(outUniformRanges[uniformIndex].size == 0 || outUniformRanges[uniformIndex].size == stageUniformSize))
                {
                    LLGL_TRAP(
                        "cannot handle different push constant sizes between shader stages for uniform '%s'; got %u and %u",
                        uniformDescs[uniformIndex].name.c_str(), outUniformRanges[uniformIndex].size, stageUniformSize
                    );
                }
                outUniformRanges[uniformIndex].size = stageUniformSize;

                /* Use offset and size as start and end pointers and resolve size after all elements are inserted into the range */
                pushConstantBlockRanges[shaderIndex].offset   = std::min(outUniformRanges[uniformIndex].offset, pushConstantBlockRanges[shaderIndex].offset);
                pushConstantBlockRanges[shaderIndex].size     = std::max(outUniformRanges[uniformIndex].offset + outUniformRanges[uniformIndex].size, pushConstantBlockRanges[shaderIndex].size);
            }
        }

        outStageRanges[shaderIndex].offset  = pushConstantBlockRanges[shaderIndex].offset;
        outStageRanges[shaderIndex].size    = pushConstantBlockRanges[shaderIndex].size;
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

#if LLGL_VK_ENABLE_SPIRV_REFLECT

// Returns true if any of the specified shaders has at least one texel buffer,
// i.e. of type VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER or VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.
static bool HasAnyShaderWithTexelBuffers(const ArrayView<Shader*>& shaders)
{
    for (Shader* shader : shaders)
    {
        auto* shaderVK = LLGL_CAST(VKShader*, shader);
        if (shaderVK->HasAnyDescriptorOfType(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
            shaderVK->HasAnyDescriptorOfType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER))
        {
            return true;
        }
    }
    return false;
}

#endif // /LLGL_VK_ENABLE_SPIRV_REFLECT

bool VKPipelineLayout::CanHaveLayoutPermutations() const
{
    return (!uniformDescs_.empty() || HasNonUniformBuffers());
}

VKPipelineLayoutPermutationSPtr VKPipelineLayout::CreatePermutation(
    VkDevice                            device,
    const ArrayView<Shader*>&           shaders,
    std::vector<VkPushConstantRange>&   outUniformRanges) const
{
    #if LLGL_VK_ENABLE_SPIRV_REFLECT

    VKLayoutPermutationParameters permutationParams;

    /*
    Only check all shaders for any texel buffers if this PSO layout is known to contain non-uniform buffers.
    Otherwise, the search for texel buffers is irrelevant since the PSO layout must not have such bindings in the first place.
    */
    const bool hasTexelBuffers = (HasNonUniformBuffers() && HasAnyShaderWithTexelBuffers(shaders));

    if (!uniformDescs_.empty() || hasTexelBuffers)
    {
        permutationParams.setLayoutHeapBindings     = setLayoutHeapBindings_.GetVkLayoutBindings();
        permutationParams.setLayoutDynamicBindings  = setLayoutDynamicBindings_.GetVkLayoutBindings();
    }

    if (hasTexelBuffers)
    {
        /* Create permutation of set-layout bindings */
        auto GetDescriptorTypeForBinding = [&shaders](const BindingSlot& slot) -> VkDescriptorType
        {
            for (Shader* shader : shaders)
            {
                auto* shaderVK = LLGL_CAST(VKShader*, shader);
                VkDescriptorType descriptorType = shaderVK->GetDescriptorTypeForBinding(slot);
                if (descriptorType != VK_DESCRIPTOR_TYPE_MAX_ENUM)
                    return descriptorType;
            }
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        };

        auto UpdateSetLayoutDescriptorTypes = [&GetDescriptorTypeForBinding](
            const std::vector<BindingSlot>&             inBindingSlots,
            std::vector<VkDescriptorSetLayoutBinding>&  setLayoutBindings) -> void
        {
            LLGL_ASSERT(inBindingSlots.size() == setLayoutBindings.size());
            for_range(i, inBindingSlots.size())
                setLayoutBindings[i].descriptorType = GetDescriptorTypeForBinding(inBindingSlots[i]);
        };

        UpdateSetLayoutDescriptorTypes(setBindingTables_[SetLayoutType_HeapBindings].srcSlots, permutationParams.setLayoutHeapBindings);
        UpdateSetLayoutDescriptorTypes(setBindingTables_[SetLayoutType_DynamicBindings].srcSlots, permutationParams.setLayoutDynamicBindings);
    }

    if (!uniformDescs_.empty())
        BuildPushConstantRanges(shaders, uniformDescs_, permutationParams.pushConstantRanges, outUniformRanges);

    if (!permutationParams.pushConstantRanges.empty() || hasTexelBuffers)
    {
        permutationParams.numImmutableSamplers = static_cast<std::uint32_t>(immutableSamplers_.size());

        return VKPipelineLayoutPermutationPool::Get().CreatePermutation(
            device, this, setLayoutImmutableSamplers_.Get(), permutationParams
        );
    }

    #else // LLGL_VK_ENABLE_SPIRV_REFLECT

    LLGL_ASSERT(uniformDescs_.empty(), "uniform descriptors in Vulkan PSO layout, but LLGL was not built with LLGL_VK_ENABLE_SPIRV_REFLECT");

    #endif // /LLGL_VK_ENABLE_SPIRV_REFLECT

    return nullptr;
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
            if ((desc.bindFlags & BindFlags::TexelBuffer) != 0)
            {
                if ((desc.bindFlags & BindFlags::Sampled) != 0)
                    return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                else if ((desc.bindFlags & BindFlags::Storage) != 0)
                    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            }
            else
            {
                if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                else if ((desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0)
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
            break;

        default:
            break;
    }
    VKTypes::MapFailed("ResourceType", "VkDescriptorType");
}

static void ConvertBindingDesc(VkDescriptorSetLayoutBinding& dst, const BindingDescriptor& src)
{
    dst.binding             = src.slot.index;
    dst.descriptorType      = GetVkDescriptorType(src);
    dst.descriptorCount     = std::max(1u, src.arraySize);
    dst.stageFlags          = GetVkShaderStageFlags(src.stageFlags);
    dst.pImmutableSamplers  = nullptr;
}

static bool IsNonUniformBufferBinding(const BindingDescriptor& bindingDesc)
{
    return (bindingDesc.type == ResourceType::Buffer && (bindingDesc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0);
}

void VKPipelineLayout::CreateDescriptorSetLayout(
    VkDevice                                device,
    const std::vector<BindingDescriptor>&   inBindings,
    std::vector<VKLayoutBinding>&           outBindings,
    VKDescriptorSetLayout&                  outDescriptorSetLayout)
{
    /* Convert heap bindings to native descriptor set layout bindings and create Vulkan descriptor set layout */
    const std::size_t numBindings = inBindings.size();
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(numBindings);

    for_range(i, numBindings)
    {
        ConvertBindingDesc(setLayoutBindings[i], inBindings[i]);

        if (IsNonUniformBufferBinding(inBindings[i]))
            flags_ |= PSOLayoutFlag_HasNonUniformBuffers;
    }

    outDescriptorSetLayout.Initialize(device, std::move(setLayoutBindings));
    outDescriptorSetLayout.GetLayoutBindings(outBindings);
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

    VKDescriptorSetLayout::CreateVkDescriptorSetLayout(device, setLayoutBindings, setLayoutImmutableSamplers_);
}

VKPtr<VkPipelineLayout> VKPipelineLayout::CreateVkPipelineLayout(VkDevice device, const ArrayView<VkPushConstantRange>& pushConstantRanges) const
{
    /* Create native Vulkan pipeline layout with up to 3 descriptor sets */
    SmallVector<VkDescriptorSetLayout, SetLayoutType_Num> setLayoutsVK;
    if (setLayoutHeapBindings_.GetVkDescriptorSetLayout() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutHeapBindings_.GetVkDescriptorSetLayout());
    if (setLayoutDynamicBindings_.GetVkDescriptorSetLayout() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutDynamicBindings_.GetVkDescriptorSetLayout());
    if (setLayoutImmutableSamplers_.Get() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutImmutableSamplers_.Get());

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

    for (const VKLayoutBinding& binding : bindingTable_.dynamicBindings)
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
    for (const VKLayoutBinding& binding : bindingTable_.dynamicBindings)
        poolSizeAccum.Accumulate(binding.descriptorType);
    poolSizeAccum.Finalize();

    /* Allocate unique descriptor cache */
    descriptorCache_ = MakeUnique<VKDescriptorCache>(
        device, descriptorPool_, setLayout, poolSizeAccum.Size(), poolSizeAccum.Data(), bindingTable_.dynamicBindings
    );
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
    VkDescriptorSetLayout setLayoutsVK[SetLayoutType_Num] =
    {
        setLayoutHeapBindings_.GetVkDescriptorSetLayout(),
        setLayoutDynamicBindings_.GetVkDescriptorSetLayout(),
        setLayoutImmutableSamplers_.Get()
    };

    for_range(i, SetLayoutType_Num)
    {
        if (setLayoutsVK[i] != VK_NULL_HANDLE)
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
