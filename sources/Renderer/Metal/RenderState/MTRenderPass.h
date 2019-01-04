/*
 * MTRenderPass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_RENDER_PASS_H
#define LLGL_MT_RENDER_PASS_H


#import <Metal/Metal.h>

#include <LLGL/RenderPass.h>
#include <vector>


namespace LLGL
{


struct RenderPassDescriptor;
struct RenderTargetDescriptor;

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

    public:
    
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

    private:

        std::vector<MTAttachmentFormat> colorAttachments_;
        MTAttachmentFormat              depthAttachment_;
        MTAttachmentFormat              stencilAttachment_;

};


} // /namespace LLGL


#endif



// ================================================================================
