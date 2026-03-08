/*
 * D3D9PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineState.h"
#include "../D3D9Types.h"
#include "../Shader/D3D9VertexShader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D9PipelineState::D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline) :
    isProgrammablePipeline_ { isProgrammablePipeline                                },
    primitiveType_          { D3D9Types::ToD3DPrimitiveType(desc.primitiveTopology) }
{
    if (desc.vertexShader != nullptr)
    {
        auto* vertexShaderD3D = LLGL_CAST(D3D9VertexShader*, desc.vertexShader);
        d3dVertexDecl_ = ComPtr<IDirect3DVertexDeclaration9>(vertexShaderD3D->GetVertexDeclaration());
    }
}

void D3D9PipelineState::SetDebugName(const char* name)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
