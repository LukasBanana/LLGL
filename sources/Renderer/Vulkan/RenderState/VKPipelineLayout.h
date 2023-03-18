/*
 * VKPipelineLayout.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PIPELINE_LAYOUT_H
#define LLGL_VK_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


struct VKLayoutBinding
{
    std::uint32_t       dstBinding;
    long                stageFlags;
    VkDescriptorType    descriptorType;
};

class VKPipelineLayout final : public PipelineLayout
{

    public:

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc);

        // Returns the native VkPipelineLayout object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        // Returns the native VkDescriptorSetLayout object for heap bindings.
        inline VkDescriptorSetLayout GetVkHeapBindingsSetLayout() const
        {
            return descriptorSetLayouts_[SetLayoutType_HeapBindings].Get();
        }

        // Returns the list of binding points that must be passed to 'VkWriteDescriptorSet' members.
        inline const std::vector<VKLayoutBinding>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        // Returns true if this instance provides permutations for the native Vulkan pipeline layout.
        inline bool HasVkPipelineLayoutPermutations() const
        {
            return !uniformDescs_.empty();
        }

    private:

        // Enumeration of descriptor set layout types.
        enum SetLayoutType
        {
            SetLayoutType_HeapBindings = 0,
            SetLayoutType_DynamicBindings,
            SetLayoutType_ImmutableSamplers,

            SetLayoutType_Num,
        };

    private:

        void CreateVkDescriptorSetLayout(
            VkDevice                                        device,
            SetLayoutType                                   setLayoutType,
            const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings
        );

        void CreateHeapBindingSetLayout(const VKPtr<VkDevice>& device, const std::vector<BindingDescriptor>& heapBindings);
        void CreateDynamicBindingSetLayout(const VKPtr<VkDevice>& device, const std::vector<BindingDescriptor>& bindings);
        void CreateImmutableSamplers(const VKPtr<VkDevice>& device, const std::vector<StaticSamplerDescriptor>& staticSamplers);

        VKPtr<VkPipelineLayout> CreateVkPipelineLayout(
            const VKPtr<VkDevice>&                  device,
            const ArrayView<VkPushConstantRange>&   pushConstantRanges = {}
        );

    private:

        VKPtr<VkPipelineLayout>         pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>    descriptorSetLayouts_[SetLayoutType_Num];
        std::vector<VKLayoutBinding>    heapBindings_;
      //std::vector<VKLayoutBinding>    bindings_;
        std::vector<VKPtr<VkSampler>>   immutableSamplers_;
        std::vector<UniformDescriptor>  uniformDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
