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
#include "D3D9DepthStencilState.h"
#include "D3D9RasterizerState.h"
#include "D3D9BlendState.h"
#include "../../DXCommon/ComPtr.h"
#include "../Direct3D9.h"


namespace LLGL
{


/*
Base class of a D3D9 pipeline state.
In D3D9, there are only graphics PSOs, but we distinguish between programmable- and legacy fixed-function pipelines.
*/
class D3D9PipelineState : public PipelineState
{

    public:

        void SetDebugName(const char* name) override final;

    public:

        D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline);
        ~D3D9PipelineState();

        virtual void Bind(D3D9StateManager& stateMngr);

        inline bool IsProgrammablePipeline() const
        {
            return isProgrammablePipeline_;
        }

        inline D3DPRIMITIVETYPE GetPrimitiveType() const
        {
            return primitiveType_;
        }

        inline IDirect3DVertexDeclaration9* GetVertexDeclaration()
        {
            return d3dVertexDecl_.Get();
        }

    private:

        const bool                          isProgrammablePipeline_ = true;
        const D3DPRIMITIVETYPE              primitiveType_          = D3DPT_TRIANGLELIST;
        ComPtr<IDirect3DVertexDeclaration9> d3dVertexDecl_;

        // State objects
        D3D9DepthStencilStateSPtr           depthStencilState_;
        D3D9RasterizerStateSPtr             rasterizerState_;
        D3D9BlendStateSPtr                  blendState_;

};


} // /namespace LLGL


#endif



// ================================================================================
