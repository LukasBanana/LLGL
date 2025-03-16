/*
 * VKPipelineLayoutPermutationPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineLayoutPermutationPool.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"


namespace LLGL
{


VKPipelineLayoutPermutationPool& VKPipelineLayoutPermutationPool::Get()
{
    static VKPipelineLayoutPermutationPool instance;
    return instance;
}

void VKPipelineLayoutPermutationPool::Clear()
{
    permutations_.clear();
}

VKPipelineLayoutPermutationSPtr VKPipelineLayoutPermutationPool::CreatePermutation(
    VkDevice                                device,
    const VKPipelineLayout*                 owner,
    VkDescriptorSetLayout                   setLayoutImmutableSamplers,
    const VKLayoutPermutationParameters&    permutationParams)
{
    /* Find existing entry */
    std::size_t position = 0;
    VKPipelineLayoutPermutationSPtr* permutation = FindInSortedArray<VKPipelineLayoutPermutationSPtr>(
        permutations_.data(),
        permutations_.size(),
        [owner, &permutationParams](const VKPipelineLayoutPermutationSPtr& entry) -> int
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(owner, entry->GetOwner());
            return VKPipelineLayoutPermutation::CompareSWO(*entry, permutationParams);
        },
        &position
    );

    if (permutation != nullptr)
        return *permutation;

    /* Create new layout permutation */
    VKPipelineLayoutPermutationSPtr newPermutation = std::make_shared<VKPipelineLayoutPermutation>(
        device, owner, setLayoutImmutableSamplers, permutationParams
    );
    permutations_.insert(permutations_.begin() + position, newPermutation);
    return newPermutation;
}

void VKPipelineLayoutPermutationPool::ReleasePermutation(VKPipelineLayoutPermutationSPtr&& layoutPermutation)
{
    if (layoutPermutation && layoutPermutation.use_count() == 2)
    {
        RemoveFromListIf(
            permutations_,
            [&layoutPermutation](const VKPipelineLayoutPermutationSPtr& entry) -> bool
            {
                return (entry.get() == layoutPermutation.get());
            }
        );
        layoutPermutation.reset();
    }
}


} // /namespace LLGL



// ================================================================================
