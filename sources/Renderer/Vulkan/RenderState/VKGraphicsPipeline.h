/*
 * VKGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_GRAPHICS_PIPELINE_H
#define LLGL_VK_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


// Vulkan graphics pipeline limitations structure.
struct VKGraphicsPipelineLimits
{
    float lineWidthRange[2];
    float lineWidthGranularity;
};

struct GraphicsPipelineDescriptor;
class VKShaderProgram;

class VKGraphicsPipeline final : public GraphicsPipeline
{

    public:

        VKGraphicsPipeline(
            const VKPtr<VkDevice>&              device,
            VkPipelineLayout                    defaultPipelineLayout,
            const GraphicsPipelineDescriptor&   desc,
            const VKGraphicsPipelineLimits&     limits
        );

        // Returns the native VkPipeline Vulkan object.
        inline VkPipeline GetVkPipeline() const
        {
            return pipeline_.Get();
        }

        // Returns true if scissors are enabled.
        inline bool IsScissorEnabled() const
        {
            return scissorEnabled_;
        }

        // Returns true if this graphics pipeline has dynamic scissor state enabled (allows 'vkCmdSetScissor' commands).
        inline bool HasDynamicScissor() const
        {
            return hasDynamicScissor_;
        }

    private:

        void CreateVkGraphicsPipeline(
            const GraphicsPipelineDescriptor&   desc,
            const VKGraphicsPipelineLimits&     limits,
            VkPipelineLayout                    pipelineLayout,
            VkRenderPass                        renderPass
        );

        VkDevice            device_             = VK_NULL_HANDLE;
        VKPtr<VkPipeline>   pipeline_;

        bool                scissorEnabled_     = false;
        bool                hasDynamicScissor_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
