/*
 * VKGraphicsPSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
            VkDevice                            device,
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
