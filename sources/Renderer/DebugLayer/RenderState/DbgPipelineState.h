/*
 * DbgPipelineState.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_PIPELINE_STATE_H
#define LLGL_DBG_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <string>


namespace LLGL
{


class DbgPipelineLayout;

class DbgPipelineState final : public PipelineState
{

    public:

        void SetName(const char* name) override;
        const Report* GetReport() const override;

    public:

        DbgPipelineState(PipelineState& instance, const GraphicsPipelineDescriptor& desc);
        DbgPipelineState(PipelineState& instance, const ComputePipelineDescriptor& desc);
        ~DbgPipelineState();

    public:

        PipelineState&                  instance;
        std::string                     label;
        const DbgPipelineLayout* const  pipelineLayout  = nullptr;
        const bool                      isGraphicsPSO   = false;

        union
        {
            const GraphicsPipelineDescriptor    graphicsDesc;
            const ComputePipelineDescriptor     computeDesc;
        };

};


} // /namespace LLGL


#endif



// ================================================================================
