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


class VKPipelineLayout : public PipelineLayout
{

    public:

        VKPipelineLayout(const VKPtr<VkDevice>& device, const PipelineLayoutDescriptor& desc);

        inline VkPipelineLayout Get() const
        {
            return pipelineLayout_.Get();
        }

        inline VkDescriptorSetLayout GetDescriptorSetLayout() const
        {
            return descriptorSetLayout_.Get();
        }

    private:

        VkDevice                        device_                 = VK_NULL_HANDLE;
        VKPtr<VkPipelineLayout>         pipelineLayout_;
        VKPtr<VkDescriptorSetLayout>    descriptorSetLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
