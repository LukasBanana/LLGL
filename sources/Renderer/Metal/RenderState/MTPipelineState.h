/*
 * MTPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_PIPELINE_STATE_H
#define LLGL_MT_PIPELINE_STATE_H


#import <Metal/Metal.h>

#include <LLGL/PipelineState.h>
#include "MTDescriptorCache.h"
#include "MTConstantsCacheLayout.h"
#include <LLGL/Report.h>
#include <LLGL/Container/ArrayView.h>
#include <memory>


namespace LLGL
{


class Shader;
class PipelineLayout;
class MTPipelineLayout;

class MTPipelineState : public PipelineState
{

    public:

        MTPipelineState(bool isGraphicsPSO, const PipelineLayout* pipelineLayout);

        const Report* GetReport() const override final;

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

        // Returns the pipeline layout this PSO was created with. May also be null.
        inline const MTPipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

        // Returns the constants cache for this PSO or null if there is none.
        inline const MTConstantsCacheLayout* GetConstantsCacheLayout() const
        {
            return constantsCacheLayout_.get();
        }

    protected:

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);

        // Returns true if this PSO needs a constants cache.
        bool NeedsConstantsCache() const;

        // Creates the constants cache for the specified PSO reflection.
        void CreateConstantsCacheForRenderPipeline(MTLRenderPipelineReflection* reflection);
        void CreateConstantsCacheForComputePipeline(MTLComputePipelineReflection* reflection);

        // Returns a mutable reference to the PSO report.
        inline Report& GetMutableReport()
        {
            return report_;
        }

    private:

        const bool                              isGraphicsPSO_          = false;
        const MTPipelineLayout*                 pipelineLayout_         = nullptr;
        std::unique_ptr<MTConstantsCacheLayout> constantsCacheLayout_;
        Report                                  report_;

};


} // /namespace LLGL


#endif



// ================================================================================
