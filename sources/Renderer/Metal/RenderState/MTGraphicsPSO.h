/*
 * MTGraphicsPSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

class MTGraphicsPSO final : public MTPipelineState
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

        /*
        Returns the number of patch control points for graphics PSOs with a tessellator stage,
        or 0 if there was no post-tessellation vertex function specified.
        */
        inline NSUInteger GetNumPatchControlPoints() const
        {
            return numPatchControlPoints_;
        }

        // Returns the patch type of the post-tessellation vertex function.
        inline MTLPatchType GetPatchType() const
        {
            return patchType_;
        }

        // Returns the compute pipeline state for tessellation shaders.
        inline id<MTLComputePipelineState> GetTessPipelineState() const
        {
            return tessPipelineState_;
        }

        // Returns true if the blend color must be set independently of the PSO.
        inline bool IsBlendColorDynamic() const
        {
            return blendColorDynamic_;
        }

        // Returns true if the stencil reference must be set independently of the PSO.
        inline bool IsStencilRefDynamic() const
        {
            return stencilRefDynamic_;
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
        id<MTLComputePipelineState> tessPipelineState_      = nil;

        MTLCullMode                 cullMode_               = MTLCullModeNone;
        MTLWinding                  winding_                = MTLWindingClockwise;
        MTLTriangleFillMode         fillMode_               = MTLTriangleFillModeFill;
        MTLPrimitiveType            primitiveType_          = MTLPrimitiveTypeTriangle;
        MTLDepthClipMode            clipMode_               = MTLDepthClipModeClip;
        NSUInteger                  numPatchControlPoints_  = 0;
        MTLPatchType                patchType_              = MTLPatchTypeNone;

        float                       depthBias_              = 0.0f;
        float                       depthSlope_             = 0.0f;
        float                       depthClamp_             = 0.0f;

        bool                        blendColorDynamic_      = false;
        bool                        blendColorEnabled_      = false;
        float                       blendColor_[4]          = { 0.0f, 0.0f, 0.0f, 0.0f };

        bool                        stencilRefDynamic_      = false;
        std::uint32_t               stencilFrontRef_        = 0;
        std::uint32_t               stencilBackRef_         = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
