/*
 * WGPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_STATE_H
#define LLGL_WG_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGPipelineState : public PipelineState
{

    public:

        WGPipelineState(bool isRenderPipeline);

        const Report* GetReport() const override;

        inline bool IsRenderPipeline() const
        {
            return isRenderPipeline_;
        }

    protected:

        // Returns a mutable reference to the PSO report.
        inline Report& GetMutableReport()
        {
            return report_;
        }

    private:

        bool    isRenderPipeline_ = false;
        Report  report_;

};


} // /namespace LLGL


#endif



// ================================================================================
