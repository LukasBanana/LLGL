/*
 * VKPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_STATE_H
#define LLGL_VK_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/Container/ArrayView.h>
#include "VKPipelineLayout.h"
#include "VKPipelineLayoutPermutation.h"
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class Shader;
class PipelineLayout;
class VKShader;
class VKPipelineLayout;

class VKPipelineState : public PipelineState
{

    public:

        VKPipelineState(
            VkDevice                    device,
            VkPipelineBindPoint         bindPoint,
            const ArrayView<Shader*>&   shaders,
            const PipelineLayout*       pipelineLayout = nullptr
        );

        ~VKPipelineState();

        const Report* GetReport() const override;

    public:

        // Binds this pipeline state and optional static descriptor sets (for immutable samplers) to the specified Vulkan command buffer.
        void BindPipelineAndStaticDescriptorSet(VkCommandBuffer commandBuffer);

        // Binds the specified descriptor set to the dynamic descriptor set binding point.
        void BindDynamicDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet);

        // Binds the specified descriptor set to teh heap descriptor set binding point.
        void BindHeapDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet);

        // Pushes the specified values to the command buffer as push-constants.
        void PushConstants(VkCommandBuffer commandBuffer, std::uint32_t first, const char* data, std::uint32_t size);

        // Returns the native PSO.
        inline VkPipeline GetVkPipeline() const
        {
            return pipeline_.Get();
        }

        // Returns the pipeline binding point.
        inline VkPipelineBindPoint GetBindPoint() const
        {
            return bindPoint_;
        }

        // Returns the pipeline layout this PSO was created with.
        inline const VKPipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

        // Returns the binding table and descriptor cache of this PSO's layout permutation.
        bool GetBindingTableAndDescriptorCache(const VKLayoutBindingTable*& outBindingTable, VKDescriptorCache*& outDescriptorCache) const;

    protected:

        // Releases the native PSO and returns its address.
        VkPipeline* ReleaseAndGetAddressOfVkPipeline();

        // Returns the native Vulkan pipeline layout this PSO was created with or the specified layout if there was no layout specified.
        VkPipelineLayout GetVkPipelineLayout() const;

        /*
        Fills the native shader stage descriptor for the specified shader:
        - If the pipeline layout constaints uniforms, the shader module will be parsed for push constants.
        - If the shader module has a binding set mismatch with the pipeline layout,
          a permutation of the shader module will be created to match the internal binding set layout of the Vulkan backend.
        */
        void GetShaderCreateInfoAndOptionalPermutation(VKShader& shaderVK, VkPipelineShaderStageCreateInfo& outCreateInfo);

        // Returns the mutable report object.
        inline Report& GetMutableReport()
        {
            return report_;
        }

    private:

        void BindDescriptorSets(
            VkCommandBuffer         commandBuffer,
            std::uint32_t           firstSet,
            std::uint32_t           descriptorSetCount,
            const VkDescriptorSet*  descriptorSets
        );

    private:

        VKPtr<VkPipeline>                   pipeline_;
        VKPipelineLayoutPermutationSPtr     pipelineLayoutPerm_;
        const VKPipelineLayout*             pipelineLayout_     = nullptr;
        VkPipelineBindPoint                 bindPoint_          = VK_PIPELINE_BIND_POINT_MAX_ENUM;
        std::vector<VkPushConstantRange>    uniformRanges_;     // Push constant ranges; One range for each uniform descriptor. See UniformDescriptor.
        Report                              report_;

};


} // /namespace LLGL


#endif



// ================================================================================
