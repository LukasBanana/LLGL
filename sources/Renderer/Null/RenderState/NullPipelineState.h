/*
 * NullPipelineState.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        void SetName(const char* name) override;
        const Report* GetReport() const override;

    public:

        NullPipelineState(const GraphicsPipelineDescriptor& desc);
        NullPipelineState(const ComputePipelineDescriptor& desc);
        ~NullPipelineState();

    public:

        const bool isGraphicsPSO;

        union
        {
            const GraphicsPipelineDescriptor    graphicsDesc;
            const ComputePipelineDescriptor     computeDesc;
        };

    private:

        std::string label_;
};


} // /namespace LLGL


#endif



// ================================================================================
