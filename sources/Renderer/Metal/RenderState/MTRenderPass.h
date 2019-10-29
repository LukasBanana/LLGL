/*
 * MTRenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_RENDER_PASS_H
#define LLGL_MT_RENDER_PASS_H


#import <Metal/Metal.h>

#include <LLGL/RenderPass.h>
#include <LLGL/ForwardDecls.h>
#include <vector>


namespace LLGL
{


struct MTAttachmentFormat
{
    MTLPixelFormat  pixelFormat = MTLPixelFormatInvalid;
    MTLLoadAction   loadAction  = MTLLoadActionDontCare;
    MTLStoreAction  storeAction = MTLStoreActionDontCare;
};

// Stores the native attachment formats and load/store actions.
class MTRenderPass final : public RenderPass
{

    public:

        MTRenderPass(const RenderPassDescriptor& desc);
        MTRenderPass(const RenderTargetDescriptor& desc);
        MTRenderPass(const RenderContextDescriptor& desc);

    public:

        // Returns the combined depth-stencil format.
        MTLPixelFormat GetDepthStencilFormat() const;

        inline const std::vector<MTAttachmentFormat>& GetColorAttachments() const
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

        std::vector<MTAttachmentFormat> colorAttachments_;
        MTAttachmentFormat              depthAttachment_;
        MTAttachmentFormat              stencilAttachment_;
        NSUInteger                      sampleCount_        = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
