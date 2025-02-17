/*
 * VKPipelineLayoutPermutationPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_LAYOUT_PERMUTATION_POOL_H
#define LLGL_VK_PIPELINE_LAYOUT_PERMUTATION_POOL_H


#include "VKPipelineLayoutPermutation.h"
#include <vector>


namespace LLGL
{


class VKPipelineLayoutPermutationPool
{

    public:

        VKPipelineLayoutPermutationPool(const VKPipelineLayoutPermutationPool&) = delete;
        VKPipelineLayoutPermutationPool& operator = (const VKPipelineLayoutPermutationPool&) = delete;

        static VKPipelineLayoutPermutationPool& Get();

        // Clear all resource containers of this pool (used by VKRenderSystem).
        void Clear();

        VKPipelineLayoutPermutationSPtr CreatePermutation(
            VkDevice                                device,
            const VKPipelineLayout*                 owner,
            VkDescriptorSetLayout                   setLayoutImmutableSamplers,
            const VKLayoutPermutationParameters&    permutationParams
        );

        void ReleasePermutation(VKPipelineLayoutPermutationSPtr&& layoutPermutation);

    private:

        VKPipelineLayoutPermutationPool() = default;

    private:

        std::vector<VKPipelineLayoutPermutationSPtr> permutations_;

};


} // /namespace LLGL


#endif



// ================================================================================
