/*
 * VKComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COMPUTE_PIPELINE_H
#define LLGL_VK_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class VKShaderProgram;

class VKComputePipeline final : public ComputePipeline
{

    public:

        VKComputePipeline(const VKPtr<VkDevice>& device, const ComputePipelineDescriptor& desc, VkPipelineLayout defaultPipelineLayout);

        inline VkPipeline GetVkPipeline() const
        {
            return pipeline_.Get();
        }

        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_;
        }

    private:

        void CreateComputePipeline(const ComputePipelineDescriptor& desc);

        VkDevice            device_         = VK_NULL_HANDLE;
        VkPipelineLayout    pipelineLayout_ = VK_NULL_HANDLE;
        VKPtr<VkPipeline>   pipeline_;

};


} // /namespace LLGL


#endif



// ================================================================================
