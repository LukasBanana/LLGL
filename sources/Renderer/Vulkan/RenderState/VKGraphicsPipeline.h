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

class VKGraphicsPipeline : public GraphicsPipeline
{

    public:

        VKGraphicsPipeline(
            const VKPtr<VkDevice>& device, VkRenderPass renderPass, VkPipelineLayout defaultPipelineLayout,
            const GraphicsPipelineDescriptor& desc, const VKGraphicsPipelineLimits& limits, const VkExtent2D& extent
        );

        // Returns the VkPipeline Vulkan object.
        inline VkPipeline GetVkPipeline() const
        {
            return pipeline_.Get();
        }

        // Returns the VkPipelineLayout Vulkan object.
        inline VkPipelineLayout GetVkPipelineLayout() const
        {
            return pipelineLayout_;
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

        void CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const VKGraphicsPipelineLimits& limits, const VkExtent2D& extent);

        VkDevice            device_             = VK_NULL_HANDLE;
        VkRenderPass        renderPass_         = VK_NULL_HANDLE;
        VkPipelineLayout    pipelineLayout_     = VK_NULL_HANDLE;
        VKPtr<VkPipeline>   pipeline_;

        bool                scissorEnabled_     = false;
        bool                hasDynamicScissor_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
