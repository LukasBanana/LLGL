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
#include "VKDescriptorCache.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKDescriptorCache;
class VKPoolSizeAccumulator;

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

        VKPipelineLayout(VkDevice device, const PipelineLayoutDescriptor& desc);

        // Binds the descriptor set that is statically bound to this pipeline layout, such as immutable samplers.
        void BindStaticDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) const;

        // Binds the specified descriptor set to the dynamic descriptor set binding point.
        void BindDynamicDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkDescriptorSet descriptorSet) const;

        // Returns the native VkPipelineLayout object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        // Returns the native VkDescriptorSetLayout object for heap bindings.
        inline VkDescriptorSetLayout GetSetLayoutForHeapBindings() const
        {
            return descriptorSetLayouts_[SetLayoutType_HeapBindings].Get();
        }

        // Returns the native VkDescriptorSetLayout object for dynamic bindings.
        inline VkDescriptorSetLayout GetSetLayoutForDynamicBindings() const
        {
            return descriptorSetLayouts_[SetLayoutType_DynamicBindings].Get();
        }

        // Returns the list of binding points that must be passed to 'VkWriteDescriptorSet' members.
        inline const std::vector<VKLayoutBinding>& GetLayoutHeapBindings() const
        {
            return heapBindings_;
        }

        // Returns the list of binding points that must be passed to 'VkWriteDescriptorSet' members.
        inline const std::vector<VKLayoutBinding>& GetLayoutDynamicBindings() const
        {
            return bindings_;
        }

        // Returns the descriptor cache for dynamic resources or null if there is none.
        inline VKDescriptorCache* GetDescriptorCache() const
        {
            return descriptorCache_.get();
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

        void CreateBindingSetLayout(
            VkDevice                                device,
            const std::vector<BindingDescriptor>&   inBindings,
            std::vector<VKLayoutBinding>&           outBindings,
            SetLayoutType                           setLayoutType
        );

        void CreateImmutableSamplers(VkDevice device, const ArrayView<StaticSamplerDescriptor>& staticSamplers);

        VKPtr<VkPipelineLayout> CreateVkPipelineLayout(VkDevice device, const ArrayView<VkPushConstantRange>& pushConstantRanges = {});

        void CreateDescriptorPool(VkDevice device);
        void CreateDescriptorCache(VkDevice device, VkDescriptorSet setLayout);
        void CreateStaticDescriptorSet(VkDevice device, VkDescriptorSet setLayout);

    private:

        VKPtr<VkPipelineLayout>             pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>        descriptorSetLayouts_[SetLayoutType_Num];
        std::uint32_t                       descriptorSetBindSlots_[SetLayoutType_Num]  = {};

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::unique_ptr<VKDescriptorCache>  descriptorCache_;
        VkDescriptorSet                     staticDescriptorSet_                        = VK_NULL_HANDLE;

        std::vector<VKLayoutBinding>        heapBindings_;
        std::vector<VKLayoutBinding>        bindings_;
        std::vector<VKPtr<VkSampler>>       immutableSamplers_;
        std::vector<UniformDescriptor>      uniformDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
