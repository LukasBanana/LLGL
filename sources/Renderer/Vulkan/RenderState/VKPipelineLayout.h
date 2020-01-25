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

        std::uint32_t GetNumBindings() const override;

    public:

        VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc);

        // Returns the native VkPipelineLayout object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        // Returns the native VkDescriptorSetLayout object.
        inline VkDescriptorSetLayout GetVkDescriptorSetLayout() const
        {
            return descriptorSetLayout_.Get();
        }

        // Returns the list of binding points that must be passed to 'VkWriteDescriptorSet' members.
        inline const std::vector<VKLayoutBinding>& GetBindings() const
        {
            return bindings_;
        }

    private:

        VKPtr<VkPipelineLayout>         pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>    descriptorSetLayout_;
        std::vector<VKLayoutBinding>    bindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
