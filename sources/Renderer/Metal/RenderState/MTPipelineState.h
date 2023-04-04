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
#include "MTConstantsCache.h"
#include "../../../Core/BasicReport.h"
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

        // Returns the descriptor cache for this PSO or null if there is none.
        inline MTDescriptorCache* GetDescriptorCache() const
        {
            return descriptorCache_.get();
        }

        // Resets and returns the descriptor cache for this PSO or null if there is none.
        MTDescriptorCache* ResetAndGetDescriptorCache() const;

        // Returns the constants cache for this PSO or null if there is none.
        inline MTConstantsCache* GetConstantsCache() const
        {
            return constantsCache_.get();
        }

        // Resets and returns the constants cache for this PSO or null if there is none.
        MTConstantsCache* ResetAndGetConstantsCache() const;

    protected:

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);

        // Returns true if this PSO needs a constants cache.
        bool NeedsConstantsCache() const;

        // Creates the constants cache for the specified PSO reflection.
        void CreateConstantsCacheForRenderPipeline(MTLRenderPipelineReflection* reflection);
        void CreateConstantsCacheForComputePipeline(MTLComputePipelineReflection* reflection);

        // Returns the pipeline layout this PSO was created with. May also be null.
        inline const MTPipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

    private:

        const bool                          isGraphicsPSO_      = false;
        const MTPipelineLayout*             pipelineLayout_     = nullptr;
        std::unique_ptr<MTDescriptorCache>  descriptorCache_;
        std::unique_ptr<MTConstantsCache>   constantsCache_;
        BasicReport                         report_;

};


} // /namespace LLGL


#endif



// ================================================================================
