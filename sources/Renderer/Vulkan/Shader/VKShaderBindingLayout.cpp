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


bool VKShaderBindingLayout::BuildFromSpirvModule(const void* data, std::size_t size)
{
    #if LLGL_VK_ENABLE_SPIRV_REFLECT

    /* Reflect all SPIR-V binding points */
    std::vector<SpirvReflect::SpvBindingPoint> bindingPoints;
    SpirvResult result = SpirvReflectBindingPoints(SpirvModuleView{ data, size }, bindingPoints);
    if (result != SpirvResult::NoError)
        return false;

    /* Convert binding points into to module bindings */
    bindings_.resize(bindingPoints.size());

    auto ConvertBindingPoint = [](ModuleBinding& dst, const SpirvReflect::SpvBindingPoint& src)
    {
        dst.srcDescriptorSet    = src.set;
        dst.srcBinding          = src.binding;
        dst.dstDescriptorSet    = src.set;
        dst.dstBinding          = src.binding;
        dst.spirvDescriptorSet  = src.setWordOffset;
        dst.spirvBinding        = src.bindingWordOffset;
    };

    for_range(i, bindingPoints.size())
        ConvertBindingPoint(bindings_[i], bindingPoints[i]);

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

    #else

    /* Cannot build binding layout from SPIR-V module without capability of SPIR-V reflection */
    return false;

    #endif
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
    auto* words = reinterpret_cast<std::uint32_t*>(data);
    const std::size_t numWords = size/4;

    for (const ModuleBinding& binding : bindings_)
    {
        if (binding.spirvDescriptorSet < numWords)
            words[binding.spirvDescriptorSet] = binding.dstDescriptorSet;
        if (binding.spirvBinding < numWords)
            words[binding.spirvBinding] = binding.dstBinding;
    }
}


} // /namespace LLGL



// ================================================================================
