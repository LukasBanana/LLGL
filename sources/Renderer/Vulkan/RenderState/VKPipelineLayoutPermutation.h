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

struct VKLayoutBinding
{
    std::uint32_t           dstBinding;
    std::uint32_t           dstArrayElement;
    VkDescriptorType        descriptorType;
    VkPipelineStageFlags    stageFlags;
    long                    bindFlags;
};

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

    static int CompareSWO(const VKLayoutPermutationParameters& lhs, const VKLayoutPermutationParameters& rhs);
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
            return setLayoutHeapBindings_.Get();
        }

        // Returns the native VkDescriptorSetLayout object for dynamic bindings.
        inline VkDescriptorSetLayout GetSetLayoutForDynamicBindings() const
        {
            return setLayoutDynamicBindings_.Get();
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

        // Returns the permutation parameters this layout was created with.
        inline const VKLayoutPermutationParameters& GetPermutationParams() const
        {
            return permutationParams_;
        }

    private:

        void CreateVkDescriptorSetLayout(
            VkDevice                                        device,
            const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings,
            VKPtr<VkDescriptorSetLayout>&                   outSetLayout
        );

        void CreateBindingSetLayout(
            VkDevice                                            device,
            const std::vector<VkDescriptorSetLayoutBinding>&    setLayoutBindings,
            std::vector<VKLayoutBinding>&                       outBindings,
            VKPtr<VkDescriptorSetLayout>&                       outSetLayout
        );

        VKPtr<VkPipelineLayout> CreateVkPipelineLayout(
            VkDevice                                device,
            VkDescriptorSetLayout                   setLayoutImmutableSamplers,
            const ArrayView<VkPushConstantRange>&   pushConstantRanges
        ) const;

        void CreateDescriptorPool(VkDevice device, std::uint32_t numImmutableSamplers);
        void CreateDescriptorCache(VkDevice device, VkDescriptorSetLayout setLayout);

    private:

        const VKPipelineLayout*             owner_                      = nullptr;

        VKPtr<VkPipelineLayout>             pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>        setLayoutHeapBindings_;
        VKPtr<VkDescriptorSetLayout>        setLayoutDynamicBindings_;

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::unique_ptr<VKDescriptorCache>  descriptorCache_;

        VKLayoutBindingTable                bindingTable_;
        VKLayoutPermutationParameters       permutationParams_;

};

using VKPipelineLayoutPermutationSPtr = std::shared_ptr<VKPipelineLayoutPermutation>;


} // /namespace LLGL


#endif



// ================================================================================
