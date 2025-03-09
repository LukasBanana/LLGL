/*
 * VKPipelineLayoutPermutation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_LAYOUT_PERMUTATION_H
#define LLGL_VK_PIPELINE_LAYOUT_PERMUTATION_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "VKDescriptorSetLayout.h"
#include "VKDescriptorCache.h"
#include "../Shader/VKShader.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>
#include <memory>


namespace LLGL
{


class VKPoolSizeAccumulator;
class VKPipelineLayout;

struct VKLayoutBindingTable
{
    std::vector<VKLayoutBinding> heapBindings;
    std::vector<VKLayoutBinding> dynamicBindings;
};

struct VKLayoutPermutationParameters
{
    std::vector<VkDescriptorSetLayoutBinding>   setLayoutHeapBindings;
    std::vector<VkDescriptorSetLayoutBinding>   setLayoutDynamicBindings;
    std::vector<VkPushConstantRange>            pushConstantRanges;
    std::uint32_t                               numImmutableSamplers = 0;
};

class VKPipelineLayoutPermutation
{

    public:

        VKPipelineLayoutPermutation(
            VkDevice                                device,
            const VKPipelineLayout*                 owner,
            VkDescriptorSetLayout                   setLayoutImmutableSamplers,
            const VKLayoutPermutationParameters&    permutationParams
        );

        // Returns true owner this layout permutation belongs to.
        inline const VKPipelineLayout* GetOwner() const
        {
            return owner_;
        }

        // Returns the native VkPipelineLayout object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        // Returns the native VkDescriptorSetLayout object for heap bindings.
        inline VkDescriptorSetLayout GetSetLayoutForHeapBindings() const
        {
            return setLayoutHeapBindings_.GetVkDescriptorSetLayout();
        }

        // Returns the native VkDescriptorSetLayout object for dynamic bindings.
        inline VkDescriptorSetLayout GetSetLayoutForDynamicBindings() const
        {
            return setLayoutDynamicBindings_.GetVkDescriptorSetLayout();
        }

        // Returns the binding table for this pipeline layout.
        inline const VKLayoutBindingTable& GetBindingTable() const
        {
            return bindingTable_;
        }

        // Returns the descriptor cache for dynamic resources or null if there is none.
        inline VKDescriptorCache* GetDescriptorCache() const
        {
            return descriptorCache_.get();
        }

    public:

        static int CompareSWO(const VKPipelineLayoutPermutation& lhs, const VKLayoutPermutationParameters& rhs);

    private:

        void CreateBindingSetLayout(
            VkDevice                                    device,
            std::vector<VkDescriptorSetLayoutBinding>   setLayoutBindings,
            std::vector<VKLayoutBinding>&               outBindings,
            VKDescriptorSetLayout&                      outSetLayout
        );

        VKPtr<VkPipelineLayout> CreateVkPipelineLayout(VkDevice device, VkDescriptorSetLayout setLayoutImmutableSamplers) const;

        void CreateDescriptorPool(VkDevice device, std::uint32_t numImmutableSamplers);
        void CreateDescriptorCache(VkDevice device, VkDescriptorSetLayout setLayout);

    private:

        const VKPipelineLayout*             owner_                      = nullptr;

        VKPtr<VkPipelineLayout>             pipelineLayout_;
        VKDescriptorSetLayout               setLayoutHeapBindings_;
        VKDescriptorSetLayout               setLayoutDynamicBindings_;

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::unique_ptr<VKDescriptorCache>  descriptorCache_;

        VKLayoutBindingTable                bindingTable_;
        std::vector<VkPushConstantRange>    pushConstantRanges_;
        std::uint32_t                       numImmutableSamplers_       = 0;

};

using VKPipelineLayoutPermutationSPtr = std::shared_ptr<VKPipelineLayoutPermutation>;


} // /namespace LLGL


#endif



// ================================================================================
