/*
 * MTGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_GRAPHICS_PIPELINE_H
#define LLGL_MT_GRAPHICS_PIPELINE_H


#import <Metal/Metal.h>

#include <LLGL/GraphicsPipeline.h>
#include <LLGL/ForwardDecls.h>


namespace LLGL
{


class MTGraphicsPipeline : public GraphicsPipeline
{

    public:

        MTGraphicsPipeline(id<MTLDevice> device, const GraphicsPipelineDescriptor& desc);

        void Bind(id<MTLRenderCommandEncoder> cmdEncoder);
    
        inline MTLPrimitiveType GetMTLPrimitiveType() const
        {
            return primitiveType_;
        }

    private:

        id<MTLRenderPipelineState>  renderPipelineState_    = nil;
        id<MTLDepthStencilState>    depthStencilState_      = nil;
        MTLPrimitiveType            primitiveType_          = MTLPrimitiveTypeTriangle;

};


} // /namespace LLGL


#endif



// ================================================================================
