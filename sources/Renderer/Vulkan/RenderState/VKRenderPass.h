/*
 * VKRenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_PASS_H
#define LLGL_VK_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


struct RenderPassDescriptor;

class VKRenderPass final : public RenderPass
{

    public:

        VKRenderPass(const VKPtr<VkDevice>& device, const RenderPassDescriptor& desc);

        // Returns the Vulkan render pass object.
        inline VkRenderPass GetVkRenderPass() const
        {
            return renderPass_;
        }

    private:

        void CreateRenderPass(const VKPtr<VkDevice>& device, const RenderPassDescriptor& desc);

        VKPtr<VkRenderPass> renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
