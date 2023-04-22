/*
 * MTRenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_RENDER_TARGET_H
#define LLGL_MT_RENDER_TARGET_H


#import <Metal/Metal.h>

#include <LLGL/RenderTarget.h>
#include "../RenderState/MTRenderPass.h"


namespace LLGL
{


struct MTAttachmentFormat;
class MTRenderPass;

class MTRenderTarget final : public RenderTarget
{

    public:

        MTRenderTarget(id<MTLDevice> device, const RenderTargetDescriptor& desc);
        ~MTRenderTarget();

        Extent2D GetResolution() const override;
        std::uint32_t GetSamples() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

        // Updates the native render pass descriptor with the specified clear values. Returns null on failure.
        MTLRenderPassDescriptor* GetAndUpdateNativeRenderPass(
            const MTRenderPass& renderPass,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

    public:

        // Returns the native render pass descriptor <MTLRenderPassDescriptor>.
        inline MTLRenderPassDescriptor* GetNativeRenderPass() const
        {
            return nativeRenderPass_;
        }

    private:

        void CreateAttachment(
            id<MTLDevice>                       device,
            MTLRenderPassAttachmentDescriptor*  attachment,
            const AttachmentDescriptor&         desc,
            const MTAttachmentFormat&           fmt,
            std::uint32_t                       slot
        );

        MTLTextureDescriptor* CreateTextureDesc(
            id<MTLDevice>   device,
            MTLPixelFormat  pixelFormat,
            NSUInteger      sampleCount = 1u
        );

        id<MTLTexture> CreateAttachmentTexture(id<MTLDevice> device, MTLPixelFormat pixelFormat);

    private:

        Extent2D                    resolution_;
        MTLRenderPassDescriptor*    nativeRenderPass_           = nullptr; // Cannot be id<>
        MTLRenderPassDescriptor*    nativeMutableRenderPass_    = nullptr; // Cannot be id<>
        std::uint32_t               numColorAttachments_        = 0;
        MTRenderPass                renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
