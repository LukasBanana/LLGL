/*
 * VKGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_GRAPHICS_PIPELINE_H
#define LLGL_VK_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKShaderProgram;

class VKGraphicsPipeline : public GraphicsPipeline
{

    public:

        VKGraphicsPipeline(const VKPtr<VkDevice>& device, VkRenderPass renderPass, const GraphicsPipelineDescriptor& desc);

        inline VkPipeline Get() const
        {
            return pipeline_.Get();
        }

    private:

        void CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        VkDevice                device_         = VK_NULL_HANDLE;
        VkRenderPass            renderPass_     = VK_NULL_HANDLE;
        VKPtr<VkPipelineLayout> pipelineLayout_;
        VKPtr<VkPipeline>       pipeline_;

        VKShaderProgram*        shaderProgram_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
