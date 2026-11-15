/*
 * VKRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_RENDER_PASS_H
#define LLGL_VK_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include "../Vulkan.h" // Configures the platform surface extension before <vulkan/vulkan.h> is pulled in
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


struct RenderPassDescriptor;
struct VKPhysicalDevicePropertiesExt;

class VKRenderPass final : public RenderPass
{

    public:

        VKRenderPass(VkDevice device);
        VKRenderPass(VkDevice device, const RenderPassDescriptor& desc);

        // (Re-)creates the render pass object.
        void CreateVkRenderPass(
            VkDevice                    device,
            const RenderPassDescriptor& desc
        );

        /*
        Creates the render pass from a flat array of native attachment descriptors, laid out as
        [0, numColorAttachments) color, [numColorAttachments] depth-stencil (if numColorAttachments < numAttachments),
        [numAttachments, numAttachments + numColorAttachments) color resolve targets (only read when multi-sampled),
        and [numAttachments + numColorAttachments] the depth-stencil resolve target (only read when 'hasDepthStencilResolve').
        */
        void CreateVkRenderPassWithDescriptors(
            VkDevice                        device,
            std::uint32_t                   numAttachments,
            std::uint32_t                   numColorAttachments,
            const VkAttachmentDescription*  attachmentDescs,
            VkSampleCountFlagBits           sampleCountBits,
            std::uint32_t                   numViews = 1,
            bool                            hasDepthStencilResolve = false,
            const ResolveMode*              colorResolveModes = nullptr,
            ResolveMode                     depthResolveMode = ResolveMode::Optimal,
            ResolveMode                     stencilResolveMode = ResolveMode::Optimal
        );

        void ValidateDepthStencilResolveModes(const VKPhysicalDevicePropertiesExt& physicalDeviceProperties, Format format) const;

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

        // Returns the resolve mode for the specified color attachment. For Vulkan, color attachments can only have ResolveMode::Average or ResolveMode::None.
        inline ResolveMode GetColorResolveMode(std::uint32_t index) const
        {
            return ((((colorResolveBitmask_ >> index) & 0x1) != 0) ? ResolveMode::Average : ResolveMode::None);
        }

        // Returns the resolve mode for the depth attachment.
        inline ResolveMode GetDepthResolveMode() const
        {
            return depthResolveMode_;
        }

        // Returns the resolve mode for the stencil attachment.
        inline ResolveMode GetStencilResolveMode() const
        {
            return stencilResolveMode_;
        }

        // Returns the sample count flag bits for this render pass.
        inline VkSampleCountFlagBits GetSampleCountBits() const
        {
            return sampleCountBits_;
        }

        // Returns the number of views for multiview rendering, or 1 if multiview is disabled for this render pass.
        inline std::uint32_t GetNumViews() const
        {
            return numViews_;
        }

    private:

        VKPtr<VkRenderPass>     renderPass_;

        std::uint64_t           clearValuesMask_        = 0;
        std::uint8_t            depthStencilIndex_      = 0xFFu;
        std::uint8_t            numClearValues_         = 0;
        std::uint8_t            numColorAttachments_    = 0;
        VkSampleCountFlagBits   sampleCountBits_        = VK_SAMPLE_COUNT_1_BIT;
        std::uint32_t           numViews_               = 1;

        std::uint32_t           colorResolveBitmask_    : 24;
        ResolveMode             depthResolveMode_       : 4;
        ResolveMode             stencilResolveMode_     : 4;

};


} // /namespace LLGL


#endif



// ================================================================================
