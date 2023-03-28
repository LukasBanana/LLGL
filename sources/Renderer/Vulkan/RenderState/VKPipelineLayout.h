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
#include "../Shader/VKShader.h"
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

        /*
        Creates a permutation of this pipeline layout for the specified shaders with push constants.
        If this pipeline layout does not have any push constants (i.e. uniform descriptors), no permutation is created and the return value is VK_NULL_HANDLE.
        */
        VKPtr<VkPipelineLayout> CreateVkPipelineLayoutPermutation(
            VkDevice                            device,
            const ArrayView<Shader*>&           shaders,
            std::vector<VkPushConstantRange>&   outUniformRanges
        ) const;

        // Creates a permutation of the specified shader.
        VKPtr<VkShaderModule> CreateVkShaderModulePermutation(VKShader& shaderVK) const;

        // Returns the native VkPipelineLayout object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        // Returns the native VkDescriptorSetLayout object for heap bindings.
        inline VkDescriptorSetLayout GetSetLayoutForHeapBindings() const
        {
            return setLayouts_[SetLayoutType_HeapBindings].Get();
        }

        // Returns the native VkDescriptorSetLayout object for dynamic bindings.
        inline VkDescriptorSetLayout GetSetLayoutForDynamicBindings() const
        {
            return setLayouts_[SetLayoutType_DynamicBindings].Get();
        }

        // Returns the descriptor set binding point for dynamic resource bindings.
        inline std::uint32_t GetBindPointForHeapBindings() const
        {
            return setBindingTables_[SetLayoutType_HeapBindings].dstSet;
        }

        // Returns the descriptor set binding point for dynamic resource bindings.
        inline std::uint32_t GetBindPointForDynamicBindings() const
        {
            return setBindingTables_[SetLayoutType_DynamicBindings].dstSet;
        }

        // Returns the descriptor set binding point for immutable samplers.
        inline std::uint32_t GetBindPointForImmutableSamplers() const
        {
            return setBindingTables_[SetLayoutType_ImmutableSamplers].dstSet;
        }

        // Returns a Vulkan handle of the static descriptor. May also be VK_NULL_HANLDE.
        inline VkDescriptorSet GetStaticDescriptorSet() const
        {
            return staticDescriptorSet_;
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

    public:

        // Creates the default VkPipelineLayout object.
        static void CreateDefault(VkDevice device);

        // Destroys the default VkPipelineLayout object.
        static void ReleaseDefault();

        // Returns the default VkPipelineLayout object.
        static VkPipelineLayout GetDefault();

    private:

        // Enumeration of descriptor set layout types.
        enum SetLayoutType
        {
            SetLayoutType_HeapBindings = 0,
            SetLayoutType_DynamicBindings,
            SetLayoutType_ImmutableSamplers,

            SetLayoutType_Num,
        };

        // Container for binding slots that must be re-assigned to a new descriptor set in the SPIR-V shader modules.
        struct DescriptorSetBindingTable
        {
            std::uint32_t               dstSet      = ~0u;
            std::vector<BindingSlot>    srcSlots;
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

        void CreateImmutableSamplers(
            VkDevice                                    device,
            const ArrayView<StaticSamplerDescriptor>&   staticSamplers
        );

        VKPtr<VkPipelineLayout> CreateVkPipelineLayout(
            VkDevice                                device,
            const ArrayView<VkPushConstantRange>&   pushConstantRanges = {}
        ) const;

        void CreateDescriptorPool(VkDevice device);
        void CreateDescriptorCache(VkDevice device, VkDescriptorSet setLayout);
        void CreateStaticDescriptorSet(VkDevice device, VkDescriptorSet setLayout);

        void BuildDescriptorSetBindingTables(const PipelineLayoutDescriptor& desc);

    private:

        template <typename TContainer>
        void BuildDescriptorSetBindingSlots(DescriptorSetBindingTable& dst, const TContainer& src);

    private:

        static VKPtr<VkPipelineLayout>      defaultPipelineLayout_;

        VKPtr<VkPipelineLayout>             pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>        setLayouts_[SetLayoutType_Num];
        DescriptorSetBindingTable           setBindingTables_[SetLayoutType_Num];

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::unique_ptr<VKDescriptorCache>  descriptorCache_;
        VkDescriptorSet                     staticDescriptorSet_                = VK_NULL_HANDLE;

        std::vector<VKLayoutBinding>        heapBindings_;
        std::vector<VKLayoutBinding>        bindings_;
        std::vector<VKPtr<VkSampler>>       immutableSamplers_;
        std::vector<UniformDescriptor>      uniformDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
