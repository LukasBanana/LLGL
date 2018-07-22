/*
 * D3D11GraphicsPipelineBase.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11GraphicsPipelineBase.h"
#include "D3D11StateManager.h"
#include "../D3D11Types.h"
#include "../Shader/D3D11ShaderProgram.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


void D3D11GraphicsPipelineBase::Bind(D3D11StateManager& stateMngr)
{
    /* Setup input-assembly states */
    stateMngr.SetPrimitiveTopology(primitiveTopology_);
    stateMngr.SetInputLayout(inputLayout_.Get());

    /* Setup shader states */
    stateMngr.SetVertexShader   (vs_.Get());
    stateMngr.SetHullShader     (hs_.Get());
    stateMngr.SetDomainShader   (ds_.Get());
    stateMngr.SetGeometryShader (gs_.Get());
    stateMngr.SetPixelShader    (ps_.Get());
}


/*
 * ======= Protected: =======
 */

D3D11GraphicsPipelineBase::D3D11GraphicsPipelineBase(const GraphicsPipelineDescriptor& desc)
{
    /* Validate pointers and get D3D shader objects */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(const D3D11ShaderProgram*, desc.shaderProgram);
    StoreShaderObjects(*shaderProgramD3D);

    inputLayout_ = shaderProgramD3D->GetInputLayout();

    /* Store dynamic pipeline states */
    primitiveTopology_  = D3D11Types::Map(desc.primitiveTopology);
    stencilRef_         = desc.stencil.front.reference;
    blendFactor_[0]     = desc.blend.blendFactor.r;
    blendFactor_[1]     = desc.blend.blendFactor.g;
    blendFactor_[2]     = desc.blend.blendFactor.b;
    blendFactor_[3]     = desc.blend.blendFactor.a;
    sampleMask_         = desc.rasterizer.multiSampling.sampleMask;
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPipelineBase::StoreShaderObjects(const D3D11ShaderProgram& shaderProgramD3D)
{
    if (shaderProgramD3D.GetVS()) { vs_ = shaderProgramD3D.GetVS()->GetNative().vs; }
    if (shaderProgramD3D.GetHS()) { hs_ = shaderProgramD3D.GetHS()->GetNative().hs; }
    if (shaderProgramD3D.GetDS()) { ds_ = shaderProgramD3D.GetDS()->GetNative().ds; }
    if (shaderProgramD3D.GetGS()) { gs_ = shaderProgramD3D.GetGS()->GetNative().gs; }
    if (shaderProgramD3D.GetPS()) { ps_ = shaderProgramD3D.GetPS()->GetNative().ps; }
}


} // /namespace LLGL



// ================================================================================
