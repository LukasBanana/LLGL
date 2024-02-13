/*
 * MTRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_RENDER_PASS_H
#define LLGL_MT_RENDER_PASS_H


#import <Metal/Metal.h>

#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Constants.h>
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


struct ClearValue;

struct MTAttachmentFormat
{
    MTLPixelFormat  pixelFormat = MTLPixelFormatInvalid;
    MTLLoadAction   loadAction  = MTLLoadActionDontCare;
    MTLStoreAction  storeAction = MTLStoreActionDontCare;
};

using MTColorAttachmentFormatVector = SmallVector<MTAttachmentFormat, LLGL_MAX_NUM_COLOR_ATTACHMENTS>;

// Stores the native attachment formats and load/store actions.
class MTRenderPass final : public RenderPass
{

    public:

        MTRenderPass(id<MTLDevice> device, const RenderPassDescriptor& desc);
        MTRenderPass(id<MTLDevice> device, const RenderTargetDescriptor& desc);
        MTRenderPass(id<MTLDevice> device, const SwapChainDescriptor& desc);

    public:

        // Returns the combined depth-stencil format.
        MTLPixelFormat GetDepthStencilFormat() const;

        /*
        Updates the native render pass descriptor with the attachment operators of this render pass and the specified clear values.
        The input render pass descriptor must have the same number of attachments as this render pass.
        Returns the number of read clear values.
        */
        std::uint32_t UpdateNativeRenderPass(
            MTLRenderPassDescriptor*    nativeRenderPass,
            std::uint32_t               numClearValues,
            const ClearValue*           clearValues
        ) const;

        inline const MTColorAttachmentFormatVector& GetColorAttachments() const
        {
            return colorAttachments_;
        }

        inline const MTAttachmentFormat& GetDepthAttachment() const
        {
            return depthAttachment_;
        }

        inline const MTAttachmentFormat& GetStencilAttachment() const
        {
            return stencilAttachment_;
        }

        inline NSUInteger GetSampleCount() const
        {
            return sampleCount_;
        }

    private:

        MTColorAttachmentFormatVector   colorAttachments_;
        MTAttachmentFormat              depthAttachment_;
        MTAttachmentFormat              stencilAttachment_;
        NSUInteger                      sampleCount_        = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
