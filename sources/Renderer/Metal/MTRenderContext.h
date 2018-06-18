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

        ~MTRenderContext();

        void Present() override;
    
        /* ----- Extended functions ----- */
    
        void MakeCurrent(id<MTLCommandBuffer> cmdBuffer, id<MTLRenderCommandEncoder> renderCmdEncoder);
    
        // Returns the native MTKView object.
        inline MTKView* GetMTKView() const
        {
            return view_;
        }

    private:
    
        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsync(const VsyncDescriptor& vsyncDesc) override;

        MTKView*                    view_               = nullptr;
        id<MTLCommandBuffer>        cmdBuffer_          = nil;
        id<MTLRenderCommandEncoder> renderCmdEncoder_   = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
