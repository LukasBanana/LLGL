/*
 * NullPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_PIPELINE_STATE_H
#define LLGL_NULL_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <string>


namespace LLGL
{


class NullPipelineState final : public PipelineState
{

    public:

        void SetDebugName(const char* name) override;
        const Report* GetReport() const override;

    public:

        NullPipelineState(const GraphicsPipelineDescriptor& desc);
        NullPipelineState(const ComputePipelineDescriptor& desc);

    public:

        const bool                          isGraphicsPSO;
        const GraphicsPipelineDescriptor    graphicsDesc;
        const ComputePipelineDescriptor     computeDesc;

    private:

        std::string label_;
};


} // /namespace LLGL


#endif



// ================================================================================
