/*
 * VKGraphicsPSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_GRAPHICS_PSO_H
#define LLGL_VK_GRAPHICS_PSO_H


#include "VKPipelineState.h"


namespace LLGL
{


// Vulkan graphics pipeline limitations structure.
struct VKGraphicsPipelineLimits
{
    float lineWidthRange[2];
    float lineWidthGranularity;
};

struct GraphicsPipelineDescriptor;
class VKRenderPass;
class RenderPass;

class VKGraphicsPSO final : public VKPipelineState
{

    public:

        VKGraphicsPSO(
            const VKPtr<VkDevice>&              device,
            VkPipelineLayout                    defaultPipelineLayout,
            const RenderPass*                   defaultRenderPass,
            const GraphicsPipelineDescriptor&   desc,
            const VKGraphicsPipelineLimits&     limits
        );

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

        void CreateVkPipeline(
            VkDevice                            device,
            VkPipelineLayout                    pipelineLayout,
            const VKRenderPass&                 renderPass,
            const VKGraphicsPipelineLimits&     limits,
            const GraphicsPipelineDescriptor&   desc
        );

    private:

        bool scissorEnabled_    = false;
        bool hasDynamicScissor_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
