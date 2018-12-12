/*
 * MTRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_RENDER_CONTEXT_H
#define LLGL_MT_RENDER_CONTEXT_H


#import <MetalKit/MetalKit.h>

#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>


namespace LLGL
{


class MTRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        MTRenderContext(
            id<MTLDevice>                   device,
            RenderContextDescriptor         desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;
    
        Format QueryColorFormat() const override;
        Format QueryDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        /* ----- Extended functions ----- */
    
        void MakeCurrent(id<MTLCommandBuffer> cmdBuffer);
    
        // Returns the native MTKView object.
        inline MTKView* GetMTKView() const
        {
            return view_;
        }

    private:
    
        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        MTKView*                view_       = nullptr;
        id<MTLCommandBuffer>    cmdBuffer_  = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
