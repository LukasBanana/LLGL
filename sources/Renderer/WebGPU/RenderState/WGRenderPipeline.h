/*
 * WGPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RENDER_PIPELINE_H
#define LLGL_WG_RENDER_PIPELINE_H


#include "WGPipelineState.h"


namespace LLGL
{


class WGRenderPipeline final : public WGPipelineState
{

    public:

        WGRenderPipeline(WGPUDevice device, const GraphicsPipelineDescriptor& desc);

        // Returns the native WebGPU render pipeline object.
        inline WGPURenderPipeline GetNative() const
        {
            return renderPipeline_;
        }

    private:

        WGPURenderPipeline                  renderPipeline_             = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
