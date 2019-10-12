/*
 * MTGraphicsPSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_GRAPHICS_PSO_H
#define LLGL_MT_GRAPHICS_PSO_H


#include "MTPipelineState.h"
#include <cstdint>


namespace LLGL
{


class MTRenderPass;
struct GraphicsPipelineDescriptor;

class MTGraphicsPSO : public MTPipelineState
{

    public:

        MTGraphicsPSO(
            id<MTLDevice>                       device,
            const GraphicsPipelineDescriptor&   desc,
            const MTRenderPass*                 defaultRenderPass
        );

        // Binds the render pipeline state, depth-stencil states, and sets the remaining parameters with the specified command encoder.
        void Bind(id<MTLRenderCommandEncoder> renderEncoder);

        // Returns the native primitive type.
        inline MTLPrimitiveType GetMTLPrimitiveType() const
        {
            return primitiveType_;
        }

    private:

        void CreateRenderPipelineState(
            id<MTLDevice>                       device,
            const GraphicsPipelineDescriptor&   desc,
            const MTRenderPass*                 defaultRenderPass
        );

        void CreateDepthStencilState(
            id<MTLDevice>                       device,
            const GraphicsPipelineDescriptor&   desc
        );

    private:

        id<MTLRenderPipelineState>  renderPipelineState_    = nil;
        id<MTLDepthStencilState>    depthStencilState_      = nil;
        MTLCullMode                 cullMode_               = MTLCullModeNone;
        MTLWinding                  winding_                = MTLWindingClockwise;
        MTLTriangleFillMode         fillMode_               = MTLTriangleFillModeFill;
        MTLPrimitiveType            primitiveType_          = MTLPrimitiveTypeTriangle;
        MTLDepthClipMode            clipMode_               = MTLDepthClipModeClip;
        float                       depthBias_              = 0.0f;
        float                       depthSlope_             = 0.0f;
        float                       depthClamp_             = 0.0f;
        bool                        blendColorEnabled_      = false;
        float                       blendColor_[4]          = { 0.0f, 0.0f, 0.0f, 0.0f };
        std::uint32_t               stencilFrontRef_        = 0;
        std::uint32_t               stencilBackRef_         = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
