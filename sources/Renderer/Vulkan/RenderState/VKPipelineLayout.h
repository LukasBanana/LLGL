/*
 * VKPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_LAYOUT_H
#define LLGL_VK_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "VKDescriptorSetLayout.h"
#include "VKPipelineLayoutPermutation.h"
#include "VKDescriptorCache.h"
#include "../Shader/VKShader.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include "../../../Core/PackedPermutation.h"
#include <vector>


namespace LLGL
{


// Implementation of the PipelineLayout interface for the Vulkan backend.
// This class acts as a template for permutations of pipeline layouts rather than wrapping the native VkPipelineLayout directly (see VKPipelineLayoutPermutation).
class VKPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        VKPipelineLayout(VkDevice device, const PipelineLayoutDescriptor& desc);
        ~VKPipelineLayout();

        // Returns true if this pipeline layout can have permutations, i.e. if this layout contains uniforms or non-uniform buffers.
        bool CanHaveLayoutPermutations() const;

        /*
        Creates a permutation of this pipeline layout for the specified shaders with push constants.
        If this pipeline layout does not have any push constants (i.e. uniform descriptors), no permutation is created and the return value is VK_NULL_HANDLE.
        */
        VKPipelineLayoutPermutationSPtr CreatePermutation(
            VkDevice                            device,
            const ArrayView<Shader*>&           shaders,
            std::vector<VkPushConstantRange>&   outUniformRanges
        ) const;

        // Returns true if a permutation is required for the specified shader.
        bool NeedsShaderModulePermutation(const VKShader& shaderVK) const;

        // Creates a permutation of the specified shader. Should only be used by VKShaderModulePool.
        VKPtr<VkShaderModule> CreateVkShaderModulePermutation(VKShader& shaderVK) const;

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

        // Returns the barrier flags this pipeline layout was created with. See PipelineLayoutDescriptor::barrierFlags.
        inline long GetBarrierFlags() const
        {
            return barrierFlags_;
        }

        // Returns true if this PSO layout has at least one non-uniform buffer binding.
        inline bool HasNonUniformBuffers() const
        {
            return ((flags_ & PSOLayoutFlag_HasNonUniformBuffers) != 0);
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

        enum PSOLayoutFlags
        {
            // Has at least one non-uniform buffer binding i.e. SSBO or texel buffer.
            // Such bindings must be dynamically resolved to either an SSBO buffer or texel buffer
            // since the LLGL interface does not differentiate between them.
            PSOLayoutFlag_HasNonUniformBuffers = (1 << 0),
        };

        // Container for binding slots that must be re-assigned to a new descriptor set in the SPIR-V shader modules.
        struct DescriptorSetBindingTable
        {
            std::uint32_t               dstSet      = ~0u;
            std::vector<BindingSlot>    srcSlots;
        };

    private:

        void CreateDescriptorSetLayout(
            VkDevice                                device,
            const std::vector<BindingDescriptor>&   inBindings,
            std::vector<VKLayoutBinding>&           outBindings,
            VKDescriptorSetLayout&                  outDescriptorSetLayout
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
        void CreateDescriptorCache(VkDevice device, VkDescriptorSetLayout setLayout);
        void CreateStaticDescriptorSet(VkDevice device, VkDescriptorSetLayout setLayout);

        void BuildDescriptorSetBindingTables(const PipelineLayoutDescriptor& desc);

        bool GetBindingSlotsAssignment(
            unsigned                                index,
            ConstFieldRangeIterator<BindingSlot>&   iter,
            std::uint32_t&                          dstSet
        ) const;

    private:

        template <typename TContainer>
        void BuildDescriptorSetBindingSlots(DescriptorSetBindingTable& dst, const TContainer& src);

    private:

        static VKPtr<VkPipelineLayout>      defaultPipelineLayout_;

        VKPtr<VkPipelineLayout>             pipelineLayout_;

        VKDescriptorSetLayout               setLayoutHeapBindings_;
        VKDescriptorSetLayout               setLayoutDynamicBindings_;
        VKPtr<VkDescriptorSetLayout>        setLayoutImmutableSamplers_;

        DescriptorSetBindingTable           setBindingTables_[SetLayoutType_Num];
        PackedPermutation3                  layoutTypeOrder_;

        VKPtr<VkDescriptorPool>             descriptorPool_;
        std::unique_ptr<VKDescriptorCache>  descriptorCache_;
        VkDescriptorSet                     staticDescriptorSet_                    = VK_NULL_HANDLE;

        VKLayoutBindingTable                bindingTable_;
        std::vector<VKPtr<VkSampler>>       immutableSamplers_;
        std::vector<UniformDescriptor>      uniformDescs_;

        long                                barrierFlags_   : 2; // BarrierFlags
        long                                flags_          : 1; // PSOLayoutFlags

};


} // /namespace LLGL


#endif



// ================================================================================
