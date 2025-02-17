/*
 * VKShaderModulePool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKShaderModulePool.h"
#include "../RenderState/VKPipelineLayout.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"


namespace LLGL
{


VKShaderModulePool& VKShaderModulePool::Get()
{
    static VKShaderModulePool instance;
    return instance;
}

void VKShaderModulePool::Clear()
{
    permutations_.clear();
}

VkShaderModule VKShaderModulePool::GetOrCreateVkShaderModulePermutation(VKShader& shader, const VKPipelineLayout& pipelineLayout)
{
    /* Try to find existing pair of shader/pipeline-layout */
    const auto* shaderPtr = &shader;
    const auto* pipelineLayoutPtr = &pipelineLayout;

    std::size_t insertionPos = 0;
    auto* permutation = FindInSortedArray<ShaderModulePermutation>(
        permutations_.data(),
        permutations_.size(),
        [shaderPtr, pipelineLayoutPtr](const ShaderModulePermutation& entry) -> int
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(pipelineLayoutPtr, entry.pipelineLayout); // Must be the first key element; See NotifyReleasePipelineLayout().
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(shaderPtr, entry.shader);
            return 0;
        },
        &insertionPos
    );

    if (permutation == nullptr)
    {
        /* Create new shader module permutation */
        VKPtr<VkShaderModule> shaderModule = pipelineLayout.CreateVkShaderModulePermutation(shader);
        VkShaderModule nativeHandle = shaderModule.Get();

        if (nativeHandle != VK_NULL_HANDLE)
        {
            ShaderModulePermutation newPermutation;
            {
                newPermutation.pipelineLayout   = pipelineLayoutPtr;
                newPermutation.shader           = shaderPtr;
                newPermutation.shaderModule     = std::move(shaderModule);
            }
            permutations_.insert(permutations_.begin() + insertionPos, std::move(newPermutation));
        }
        return nativeHandle;
    }

    return permutation->shaderModule.Get();
}

void VKShaderModulePool::NotifyReleaseShader(VKShader* shader)
{
    /* Since shader is the second key, we have to iterate over the entire list */
    RemoveAllFromListIf(
        permutations_,
        [shader](const ShaderModulePermutation& entry) -> bool
        {
            return (entry.shader == shader);
        }
    );
}

void VKShaderModulePool::NotifyReleasePipelineLayout(VKPipelineLayout* pipelineLayout)
{
    /* Since pipeline layout is the first key, we can search for the first occurance and then delete all consecutive entries that match the key */
    RemoveAllConsecutiveFromListIf(
        permutations_,
        [pipelineLayout](const ShaderModulePermutation& entry) -> bool
        {
            return (entry.pipelineLayout == pipelineLayout);
        }
    );
}


} // /namespace LLGL



// ================================================================================
