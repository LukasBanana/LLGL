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
#include <LLGL/Container/SmallVector.h>
#include "../RenderState/MTRenderPass.h"


namespace LLGL
{


struct MTAttachmentFormat;
class MTRenderPass;

class MTRenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        MTRenderTarget(id<MTLDevice> device, const RenderTargetDescriptor& desc);
        ~MTRenderTarget();

    public:

        // Updates the native render pass descriptor with the specified clear values. Returns null on failure.
        MTLRenderPassDescriptor* GetAndUpdateNativeRenderPass(
            const MTRenderPass& renderPass,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

        // Returns the native render pass descriptor <MTLRenderPassDescriptor>.
        inline MTLRenderPassDescriptor* GetNativeRenderPass() const
        {
            return nativeRenderPass_;
        }

    private:

        void CreateAttachment(
            id<MTLDevice>                       device,
            MTLRenderPassAttachmentDescriptor*  outAttachment,
            const AttachmentDescriptor&         inAttachment,
            const AttachmentDescriptor*         inResolveAttachment,
            const MTAttachmentFormat&           fmt,
            std::uint32_t                       slot
        );

        MTLTextureDescriptor* CreateTextureDesc(
            id<MTLDevice>   device,
            MTLPixelFormat  pixelFormat,
            NSUInteger      sampleCount = 1u
        );

        id<MTLTexture> CreateAttachmentTexture(id<MTLDevice> device, MTLPixelFormat pixelFormat);
        id<MTLTexture> CreateAttachmentTextureView(id<MTLTexture> sourceTexture, MTLPixelFormat pixelFormat);

    private:

        Extent2D                        resolution_;
        MTLRenderPassDescriptor*        nativeRenderPass_           = nullptr; // Cannot be id<>
        MTLRenderPassDescriptor*        nativeMutableRenderPass_    = nullptr; // Cannot be id<>
        std::uint32_t                   numColorAttachments_        = 0;
        MTRenderPass                    renderPass_;
        SmallVector<id<MTLTexture>, 2>  internalTextures_; // List of internally created MTLTexture views

};


} // /namespace LLGL


#endif



// ================================================================================
