/*
 * VKPipelineState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PIPELINE_STATE_H
#define LLGL_VK_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class PipelineLayout;

class VKPipelineState : public PipelineState
{

    public:

        VKPipelineState(const VKPtr<VkDevice>& device, VkPipelineBindPoint bindPoint);

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

    protected:

        static VkPipelineLayout GetVkPipelineLayoutOrDefault(
            const PipelineLayout*   pipelineLayout,
            VkPipelineLayout        defaultPipelineLayout
        );

        // Releases the native PSO and returns its address.
        VkPipeline* GetVkPipelineAddress();

    private:

        VKPtr<VkPipeline>   pipeline_;
        VkPipelineBindPoint bindPoint_  = VK_PIPELINE_BIND_POINT_MAX_ENUM;

};


} // /namespace LLGL


#endif



// ================================================================================
