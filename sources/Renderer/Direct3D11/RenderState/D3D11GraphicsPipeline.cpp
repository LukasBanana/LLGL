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

    if (!shaderProgramD3D->GetInputLayout())
        throw std::runtime_error("can not create graphics pipeline while shader program has no D3D11 input layout");

    inputLayout_ = shaderProgramD3D->GetInputLayout();

    /* Store D3D primitive topology */
    primitiveTopology_ = D3D11Types::Map(desc.primitiveTopology);

    /* Store D3D stencil reference value */
    stencilRef_ = desc.stencil.front.reference;

    /* Store blend factor */
    blendFactor_ = desc.blend.blendFactor;

    /* Create D3D11 render state objects */
    CreateDepthStencilState(device, desc.depth, desc.stencil);
    CreateRasterizerState(device, desc.rasterizer);
    CreateBlendState(device, desc.blend);
}

void D3D11GraphicsPipeline::Bind(ID3D11DeviceContext* context)
{
    /* Setup input-assembly states */
    context->IASetPrimitiveTopology(primitiveTopology_);
    context->IASetInputLayout(inputLayout_.Get());

    /* Setup shader states */
    context->VSSetShader(vs_.Get(), nullptr, 0);
    context->HSSetShader(hs_.Get(), nullptr, 0);
    context->DSSetShader(ds_.Get(), nullptr, 0);
    context->GSSetShader(gs_.Get(), nullptr, 0);
    context->PSSetShader(ps_.Get(), nullptr, 0);

    /* Setup render states */
    context->RSSetState(rasterizerState_.Get());
    context->OMSetDepthStencilState(depthStencilState_.Get(), stencilRef_);
    context->OMSetBlendState(blendState_.Get(), blendFactor_.Ptr(), sampleMask_);
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPipeline::GetShaderObjects(D3D11ShaderProgram& shaderProgramD3D)
{
    if (shaderProgramD3D.GetVS()) { vs_ = shaderProgramD3D.GetVS()->GetHardwareShader().vs; }
    if (shaderProgramD3D.GetHS()) { hs_ = shaderProgramD3D.GetHS()->GetHardwareShader().hs; }
    if (shaderProgramD3D.GetDS()) { ds_ = shaderProgramD3D.GetDS()->GetHardwareShader().ds; }
    if (shaderProgramD3D.GetGS()) { gs_ = shaderProgramD3D.GetGS()->GetHardwareShader().gs; }
    if (shaderProgramD3D.GetPS()) { ps_ = shaderProgramD3D.GetPS()->GetHardwareShader().ps; }
}

static void Convert(D3D11_DEPTH_STENCILOP_DESC& to, const StencilFaceDescriptor& from)
{
    to.StencilFailOp        = D3D11Types::Map(from.stencilFailOp);
    to.StencilDepthFailOp   = D3D11Types::Map(from.depthFailOp);
    to.StencilPassOp        = D3D11Types::Map(from.depthPassOp);
    to.StencilFunc          = D3D11Types::Map(from.compareOp);
}

void D3D11GraphicsPipeline::CreateDepthStencilState(ID3D11Device* device, const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    D3D11_DEPTH_STENCIL_DESC stateDesc;
    {
        stateDesc.DepthEnable       = (depthDesc.testEnabled ? TRUE : FALSE);
        stateDesc.DepthWriteMask    = (depthDesc.writeEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
        stateDesc.DepthFunc         = D3D11Types::Map(depthDesc.compareOp);
        stateDesc.StencilEnable     = (stencilDesc.testEnabled ? TRUE : FALSE);
        stateDesc.StencilReadMask   = D3D11_DEFAULT_STENCIL_READ_MASK;
        stateDesc.StencilWriteMask  = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        Convert(stateDesc.FrontFace, stencilDesc.front);
        Convert(stateDesc.BackFace, stencilDesc.back);
    }
    auto hr = device->CreateDepthStencilState(&stateDesc, &depthStencilState_);
    DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil state");
}

void D3D11GraphicsPipeline::CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc)
{
    D3D11_RASTERIZER_DESC stateDesc;
    {
        stateDesc.FillMode              = D3D11Types::Map(desc.polygonMode);
        stateDesc.CullMode              = D3D11Types::Map(desc.cullMode);
        stateDesc.FrontCounterClockwise = (desc.frontCCW ? TRUE : FALSE);
        stateDesc.DepthBias             = desc.depthBias;
        stateDesc.DepthBiasClamp        = desc.depthBiasClamp;
        stateDesc.SlopeScaledDepthBias  = desc.slopeScaledDepthBias;
        stateDesc.DepthClipEnable       = (desc.depthClampEnabled ? TRUE : FALSE);
        stateDesc.ScissorEnable         = (desc.scissorTestEnabled ? TRUE : FALSE);
        stateDesc.MultisampleEnable     = (desc.multiSampleEnabled ? TRUE : FALSE);
        stateDesc.AntialiasedLineEnable = (desc.antiAliasedLineEnabled ? TRUE : FALSE);
    }
    auto hr = device->CreateRasterizerState(&stateDesc, &rasterizerState_);
    DXThrowIfFailed(hr, "failed to create D3D11 rasterizer state");
}

static UINT8 GetColorWriteMask(const ColorRGBAb& color)
{
    UINT8 mask = 0;

    if (color.r) { mask |= D3D11_COLOR_WRITE_ENABLE_RED;   }
    if (color.g) { mask |= D3D11_COLOR_WRITE_ENABLE_GREEN; }
    if (color.b) { mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;  }
    if (color.a) { mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA; }

    return mask;
}

void D3D11GraphicsPipeline::CreateBlendState(ID3D11Device* device, const BlendDescriptor& desc)
{
    D3D11_BLEND_DESC stateDesc;
    {
        stateDesc.AlphaToCoverageEnable  = FALSE;
        stateDesc.IndependentBlendEnable = (desc.targets.size() > 1 ? TRUE : FALSE);

        for (UINT i = 0, n = static_cast<UINT>(desc.targets.size()); i < 8u; ++i)
        {
            auto& targetState = stateDesc.RenderTarget[i];

            if (i < n)
            {
                const auto& targetDesc = desc.targets[i];

                targetState.BlendEnable             = desc.blendEnabled;
                targetState.SrcBlend                = D3D11Types::Map(targetDesc.srcColor);
                targetState.DestBlend               = D3D11Types::Map(targetDesc.destColor);
                targetState.BlendOp                 = D3D11Types::Map(targetDesc.colorArithmetic);
                targetState.SrcBlendAlpha           = D3D11Types::Map(targetDesc.srcAlpha);
                targetState.DestBlendAlpha          = D3D11Types::Map(targetDesc.destAlpha);
                targetState.BlendOpAlpha            = D3D11Types::Map(targetDesc.alphaArithmetic);
                targetState.RenderTargetWriteMask   = GetColorWriteMask(targetDesc.colorMask);
            }
            else
            {
                targetState.BlendEnable             = FALSE;
                targetState.SrcBlend                = D3D11_BLEND_ONE;
                targetState.DestBlend               = D3D11_BLEND_ZERO;
                targetState.BlendOp                 = D3D11_BLEND_OP_ADD;
                targetState.SrcBlendAlpha           = D3D11_BLEND_ONE;
                targetState.DestBlendAlpha          = D3D11_BLEND_ZERO;
                targetState.BlendOpAlpha            = D3D11_BLEND_OP_ADD;
                targetState.RenderTargetWriteMask   = D3D11_COLOR_WRITE_ENABLE_ALL;
            }
        }
    }
    auto hr = device->CreateBlendState(&stateDesc, &blendState_);
    DXThrowIfFailed(hr, "failed to create D3D11 blend state");
}


} // /namespace LLGL



// ================================================================================
