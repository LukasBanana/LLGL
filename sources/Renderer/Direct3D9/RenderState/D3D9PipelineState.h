/*
 * D3D9PipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_PIPELINE_STATE_H
#define LLGL_D3D9_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9PipelineState : public PipelineState
{

    public:

        void SetDebugName(const char* name) override final;

    public:

        D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline);

        inline bool IsProgrammablePipeline() const
        {
            return isProgrammablePipeline_;
        }

        inline D3DPRIMITIVETYPE GetPrimitiveType() const
        {
            return primitiveType_;
        }

    private:

        const bool          isProgrammablePipeline_ = true;

        D3DPRIMITIVETYPE    primitiveType_          = D3DPT_TRIANGLELIST;

};


} // /namespace LLGL


#endif



// ================================================================================
