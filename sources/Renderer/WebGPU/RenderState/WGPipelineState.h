/*
 * WGPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_STATE_H
#define LLGL_WG_PIPELINE_STATE_H


#include "WGPipelineLayoutPermutation.h"
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

        // Returns the bind group cache of this render pipeline's layout. This will be used by the command buffer to emplace descriptors.
        inline WGBindGroupCache* GetBindGroupCache() const
        {
            return pipelineLayoutPermutation_->GetBindGroupCache();
        }

    protected:

        // Creates a pipeline layout permutation for the specified resource reflection tables.
        WGPUPipelineLayout CreatePipelineLayoutPermutation(
            const PipelineLayout*                       pipelineLayout,
            WGPUDevice                                  device,
            ArrayView<const WGResourceReflectionTable*> resourceTables
        );

        // Returns a mutable reference to the PSO report.
        inline Report& GetMutableReport()
        {
            return report_;
        }

        // Returns the pipeline layout permutation for this pipeline state.
        inline WGPipelineLayoutPermutation* GetPipelineLayoutPermutation()
        {
            return pipelineLayoutPermutation_.get();
        }

    private:

        bool                            isRenderPipeline_           = false;
        Report                          report_;
        WGPipelineLayoutPermutationSPtr pipelineLayoutPermutation_;

};


} // /namespace LLGL


#endif



// ================================================================================
