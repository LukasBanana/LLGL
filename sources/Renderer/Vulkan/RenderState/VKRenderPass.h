/*
 * VKRenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_PASS_H
#define LLGL_VK_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


struct RenderPassDescriptor;

class VKRenderPass final : public RenderPass
{

    public:

        VKRenderPass(const VKPtr<VkDevice>& device);
        VKRenderPass(const VKPtr<VkDevice>& device, const RenderPassDescriptor& desc);

        // (Re-)creates the render pass object.
        void CreateVkRenderPass(
            VkDevice                    device,
            const RenderPassDescriptor& desc
        );

        void CreateVkRenderPassWithDescriptors(
            VkDevice                        device,
            std::uint32_t                   numAttachments,
            std::uint32_t                   numColorAttachments,
            const VkAttachmentDescription*  attachmentDescs,
            VkSampleCountFlagBits           sampleCountBits
        );

        // Returns the Vulkan render pass object.
        inline VkRenderPass GetVkRenderPass() const
        {
            return renderPass_;
        }

        /*
        Returns the bitmask for all attachments that require a clear value.
        the least significant bit specifies whether the first attachment has a clear value or not.
        */
        inline std::uint64_t GetClearValuesMask() const
        {
            return clearValuesMask_;
        }

        // Returns the index of the depth-stencil attachment, or 0xFF if there is no depth-stencil attachment.
        inline std::uint8_t GetDepthStencilIndex() const
        {
            return depthStencilIndex_;
        }

        // Returns the number of clear values that are required to begin with this render pass.
        inline std::uint8_t GetNumClearValues() const
        {
            return numClearValues_;
        }

        // Returns the number of color attachments that where specified for this render pass.
        inline std::uint8_t GetNumColorAttachments() const
        {
            return numColorAttachments_;
        }

        // Returns the sample count flag bits for this render pass.
        inline VkSampleCountFlagBits GetSampleCountBits() const
        {
            return sampleCountBits_;
        }

    private:

        VKPtr<VkRenderPass>     renderPass_;

        std::uint64_t           clearValuesMask_        = 0;
        std::uint8_t            depthStencilIndex_      = 0xFFu;
        std::uint8_t            numClearValues_         = 0;
        std::uint8_t            numColorAttachments_    = 0;
        VkSampleCountFlagBits   sampleCountBits_        = VK_SAMPLE_COUNT_1_BIT;

};


} // /namespace LLGL


#endif



// ================================================================================
