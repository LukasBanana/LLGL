/*
 * DbgPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        void SetDebugName(const char* name) override;
        const Report* GetReport() const override;

    public:

        DbgPipelineState(PipelineState& instance, const GraphicsPipelineDescriptor& desc);
        DbgPipelineState(PipelineState& instance, const ComputePipelineDescriptor& desc);
        ~DbgPipelineState();

        // Returns true if this PSO has a dynamic blend factor, i.e. BlendDescriptor::blendFactorDynamic is effectively enabled.
        bool HasDynamicBlendFactor() const;

        // Returns true if this PSO has a dynamic stencil reference, i.e. StencilDescriptor::referenceDynamic is effectively enabled.
        bool HasDynamicStencilRef() const;

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
