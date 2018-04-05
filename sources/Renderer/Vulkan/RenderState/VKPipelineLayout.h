/*
 * VKPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PIPELINE_LAYOUT_H
#define LLGL_VK_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


/*
TODO:
maybe rename "PipelineLayout" interface into "ResourceViewLayout"
*/
class VKPipelineLayout : public PipelineLayout
{

    public:

        VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc);

        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_.Get();
        }

        inline VkDescriptorSetLayout GetVkDescriptorSetLayout() const
        {
            return descriptorSetLayout_.Get();
        }

        // Returns the list of binding points that must be passed to 'VkWriteDescriptorSet::dstBinding' member.
        inline const std::vector<std::uint32_t>& GetDstBindings() const
        {
            return dstBindings_;
        }

    private:

        VkDevice                        device_                 = VK_NULL_HANDLE;
        VKPtr<VkPipelineLayout>         pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>    descriptorSetLayout_;

        std::vector<std::uint32_t>      dstBindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
