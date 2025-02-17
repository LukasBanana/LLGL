/*
 * VKShaderBindingLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKShaderBindingLayout.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>

#if LLGL_VK_ENABLE_SPIRV_REFLECT
#   include "../../SPIRV/SpirvReflect.h"
#   include "../../SPIRV/SpirvModule.h"
#endif


namespace LLGL
{


#if LLGL_VK_ENABLE_SPIRV_REFLECT

// Returns true if the input type is an OpTypeStruct with a single OpTypeRuntimeArray element that is readonly
static bool IsTypeStructWithReadonlyRuntimeArray(const SpirvReflect::SpvType* type)
{
    if (type != nullptr)
    {
        if (type->opcode == spv::OpTypeStruct && type->fields.size() == 1)
        {
            const SpirvReflect::SpvRecordField& field0 = type->fields.front();
            if (const SpirvReflect::SpvType* field0Type = field0.type)
                return (field0Type->opcode == spv::OpTypeRuntimeArray && field0.readonly);
        }
    }
    return false;
}

static VkDescriptorType SpirvTypeToVkDescriptorType(const SpirvReflect::SpvType* type, bool isSampledImage = false)
{
    if (type != nullptr)
    {
        const SpirvReflect::SpvType* derefType = type->Deref();
        switch (derefType->opcode)
        {
            case spv::OpTypeImage:
                if (derefType->dimension == spv::DimBuffer)
                {
                    if (type->readonly || isSampledImage)
                        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                    else
                        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                }
                else
                {
                    if (type->readonly || isSampledImage)
                        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    else
                        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                }

            case spv::OpTypeSampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;

            case spv::OpTypeSampledImage:
                return SpirvTypeToVkDescriptorType(derefType->baseType, true);

            case spv::OpTypeStruct:
                if (derefType->storage == spv::StorageClassStorageBuffer)
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                else
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            default:
                break;
        }
    }
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

#endif // /LLGL_VK_ENABLE_SPIRV_REFLECT

bool VKShaderBindingLayout::BuildFromSpirvModule(const void* data, std::size_t size)
{
    #if LLGL_VK_ENABLE_SPIRV_REFLECT

    /* Reflect all SPIR-V binding points */
    SpirvReflect reflection;
    SpirvResult result = reflection.Reflect(SpirvModuleView{ data, size });
    if (result != SpirvResult::NoError)
        return false;

    /* Convert binding points into to module bindings */
    auto ConvertBindingPoint = [](ModuleBinding& dst, const SpirvReflect::SpvUniform& src)
    {
        dst.srcDescriptorSet    = src.set;
        dst.srcBinding          = src.binding;
        dst.dstDescriptorSet    = src.set;      // Initialize with copy
        dst.dstBinding          = src.binding;  // Initialize with copy
        dst.spirvDescriptorSet  = src.setWordOffset;
        dst.spirvBinding        = src.bindingWordOffset;
        dst.descriptorType      = SpirvTypeToVkDescriptorType(src.type);
    };

    bindings_.resize(reflection.GetUniforms().size());

    std::size_t bindingIndex = 0;
    for (const auto& it : reflection.GetUniforms())
        ConvertBindingPoint(bindings_[bindingIndex++], it.second);

    /* Sort module bindings by descriptor set and binding points */
    std::sort(
        bindings_.begin(), bindings_.end(),
        [](const ModuleBinding& lhs, const ModuleBinding& rhs) -> bool
        {
            if (lhs.srcDescriptorSet < rhs.srcDescriptorSet) { return true;  }
            if (lhs.srcDescriptorSet > rhs.srcDescriptorSet) { return false; }
            return lhs.dstBinding < rhs.dstBinding;
        }
    );

    return true;

    #else // LLGL_VK_ENABLE_SPIRV_REFLECT

    /* Cannot build binding layout from SPIR-V module without capability of SPIR-V reflection */
    return false;

    #endif // /LLGL_VK_ENABLE_SPIRV_REFLECT
}

//private
bool VKShaderBindingLayout::MatchesBindingSlot(
    const ModuleBinding&    binding,
    const BindingSlot&      slot,
    std::uint32_t           dstSet,
    std::uint32_t*          dstBinding) const
{
    if (binding.dstDescriptorSet != dstSet)
        return false;

    if (dstBinding != nullptr)
    {
        if (binding.dstBinding != *dstBinding)
            return false;
        ++(*dstBinding);
    }

    return true;
}

bool VKShaderBindingLayout::MatchesBindingSlots(
    ConstFieldRangeIterator<BindingSlot>    iter,
    std::uint32_t                           dstSet,
    bool                                    dstBindingInAscendingOrder) const
{
    std::uint32_t dstBinding = 0;

    while (const BindingSlot* slot = iter.Next())
    {
        const ModuleBinding* binding = FindInSortedArray<const ModuleBinding>(
            bindings_.data(), bindings_.size(),
            [slot](const ModuleBinding& entry) -> int
            {
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->set  , entry.srcDescriptorSet);
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->index, entry.srcBinding      );
                return 0;
            }
        );
        if (binding != nullptr)
        {
            if (!MatchesBindingSlot(*binding, *slot, dstSet, (dstBindingInAscendingOrder ? &dstBinding : nullptr)))
                return false;
        }
    }

    return true;
}

//private
bool VKShaderBindingLayout::AssignBindingSlot(
    ModuleBinding&      binding,
    const BindingSlot&  slot,
    std::uint32_t       dstSet,
    std::uint32_t*      dstBinding)
{
    bool modified = false;

    if (binding.dstDescriptorSet != dstSet)
    {
        binding.dstDescriptorSet = dstSet;
        modified = true;
    }

    if (dstBinding != nullptr)
    {
        if (binding.dstBinding != *dstBinding)
        {
            binding.dstBinding = *dstBinding;
            modified = true;
        }
        ++(*dstBinding);
    }

    return modified;
}

std::uint32_t VKShaderBindingLayout::AssignBindingSlots(
    ConstFieldRangeIterator<BindingSlot>    iter,
    std::uint32_t                           dstSet,
    bool                                    dstBindingInAscendingOrder)
{
    std::uint32_t numBindings = 0, dstBinding = 0;

    while (const BindingSlot* slot = iter.Next())
    {
        ModuleBinding* binding = FindInSortedArray<ModuleBinding>(
            bindings_.data(), bindings_.size(),
            [slot](const ModuleBinding& entry) -> int
            {
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->set  , entry.srcDescriptorSet);
                LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot->index, entry.srcBinding      );
                return 0;
            }
        );
        if (binding != nullptr)
        {
            if (AssignBindingSlot(*binding, *slot, dstSet, (dstBindingInAscendingOrder ? &dstBinding : nullptr)))
                ++numBindings;
        }
    }

    return numBindings;
}

void VKShaderBindingLayout::UpdateSpirvModule(void* data, std::size_t size)
{
    auto* words = static_cast<std::uint32_t*>(data);
    const std::size_t numWords = size/4;

    for (const ModuleBinding& binding : bindings_)
    {
        if (binding.spirvDescriptorSet < numWords)
            words[binding.spirvDescriptorSet] = binding.dstDescriptorSet;
        if (binding.spirvBinding < numWords)
            words[binding.spirvBinding] = binding.dstBinding;
    }
}

bool VKShaderBindingLayout::HasAnyDescriptorOfType(VkDescriptorType type) const
{
    for (const ModuleBinding& binding : bindings_)
    {
        if (binding.descriptorType == type)
            return true;
    }
    return false;
}

VkDescriptorType VKShaderBindingLayout::GetDescriptorTypeForBinding(const BindingSlot& slot) const
{
    const ModuleBinding* binding = FindInSortedArray<const ModuleBinding>(
        bindings_.data(), bindings_.size(),
        [slot](const ModuleBinding& entry) -> int
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot.set  , entry.srcDescriptorSet);
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(slot.index, entry.srcBinding      );
            return 0;
        }
    );
    return (binding != nullptr ? binding->descriptorType : VK_DESCRIPTOR_TYPE_MAX_ENUM);
}


} // /namespace LLGL



// ================================================================================
