/*
 * D3D11GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11GraphicsPipeline.h"
#include "../D3D11RenderSystem.h"
#include "../D3D11Types.h"
#include "../Shader/D3D11ShaderProgram.h"
#include "../Shader/D3D11Shader.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../Assertion.h"
#include "../../../Core/Helper.h"
#include <algorithm>


namespace LLGL
{


D3D11GraphicsPipeline::D3D11GraphicsPipeline(
    ID3D11Device* device, const GraphicsPipelineDescriptor& desc)
{
    /* Validate pointers and get D3D shader objects */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(D3D11ShaderProgram*, desc.shaderProgram);
    GetShaderObjects(*shaderProgramD3D);

    /* Store D3D primitive topology */
    primitiveTopology_ = D3D11Types::Map(desc.primitiveTopology);

    /* Create D3D11 render state objects */
    CreateDepthStencilState(device, desc.depth, desc.stencil);
    CreateRasterizerState(device, desc.rasterizer);
    CreateBlendState(device, desc.blend);
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPipeline::GetShaderObjects(D3D11ShaderProgram& shaderProgramD3D)
{
    if (shaderProgramD3D.GetVS()) { vs_ = shaderProgramD3D.GetVS()->GetHardwareShader().vs; }
    if (shaderProgramD3D.GetPS()) { ps_ = shaderProgramD3D.GetPS()->GetHardwareShader().ps; }
    if (shaderProgramD3D.GetHS()) { hs_ = shaderProgramD3D.GetHS()->GetHardwareShader().hs; }
    if (shaderProgramD3D.GetDS()) { ds_ = shaderProgramD3D.GetDS()->GetHardwareShader().ds; }
    if (shaderProgramD3D.GetGS()) { gs_ = shaderProgramD3D.GetGS()->GetHardwareShader().gs; }
    if (shaderProgramD3D.GetCS()) { cs_ = shaderProgramD3D.GetCS()->GetHardwareShader().cs; }
}

void D3D11GraphicsPipeline::CreateDepthStencilState(ID3D11Device* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
}

void D3D11GraphicsPipeline::CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc)
{
}

void D3D11GraphicsPipeline::CreateBlendState(ID3D11Device* device, const BlendDescriptor& desc)
{
}


} // /namespace LLGL



// ================================================================================
