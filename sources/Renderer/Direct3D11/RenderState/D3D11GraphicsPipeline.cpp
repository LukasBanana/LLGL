/*
 * D3D11GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11GraphicsPipeline.h"
#include "D3D11StateManager.h"
#include "../D3D11RenderSystem.h"
#include "../D3D11Types.h"
#include "../Shader/D3D11ShaderProgram.h"
#include "../Shader/D3D11Shader.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <algorithm>
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


D3D11GraphicsPipeline::D3D11GraphicsPipeline(
    ID3D11Device* device, const GraphicsPipelineDescriptor& desc)
{
    /* Validate pointers and get D3D shader objects */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(D3D11ShaderProgram*, desc.shaderProgram);
    GetShaderObjects(*shaderProgramD3D);

    //if (!shaderProgramD3D->GetInputLayout())
    //    throw std::runtime_error("cannot create graphics pipeline while shader program has no D3D11 input layout");

    inputLayout_ = shaderProgramD3D->GetInputLayout();

    /* Store D3D primitive topology */
    primitiveTopology_ = D3D11Types::Map(desc.primitiveTopology);

    /* Store D3D stencil reference value */
    stencilRef_ = desc.stencil.front.reference;

    /* Store blend factor */
    blendFactor_[0] = desc.blend.blendFactor.r;
    blendFactor_[1] = desc.blend.blendFactor.g;
    blendFactor_[2] = desc.blend.blendFactor.b;
    blendFactor_[3] = desc.blend.blendFactor.a;

    /* Create D3D11 render state objects */
    CreateDepthStencilState(device, desc.depth, desc.stencil);
    CreateRasterizerState(device, desc.rasterizer);
    CreateBlendState(device, desc.blend);
}

void D3D11GraphicsPipeline::Bind(D3D11StateManager& stateMngr)
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

    /* Setup render states */
    stateMngr.SetRasterizerState(rasterizerState_.Get());
    stateMngr.SetDepthStencilState(depthStencilState_.Get(), stencilRef_);
    stateMngr.SetBlendState(blendState_.Get(), blendFactor_, sampleMask_);
}


/*
 * ======= Private: =======
 */

void D3D11GraphicsPipeline::GetShaderObjects(D3D11ShaderProgram& shaderProgramD3D)
{
    if (shaderProgramD3D.GetVS()) { vs_ = shaderProgramD3D.GetVS()->GetNative().vs; }
    if (shaderProgramD3D.GetHS()) { hs_ = shaderProgramD3D.GetHS()->GetNative().hs; }
    if (shaderProgramD3D.GetDS()) { ds_ = shaderProgramD3D.GetDS()->GetNative().ds; }
    if (shaderProgramD3D.GetGS()) { gs_ = shaderProgramD3D.GetGS()->GetNative().gs; }
    if (shaderProgramD3D.GetPS()) { ps_ = shaderProgramD3D.GetPS()->GetNative().ps; }
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
        stateDesc.DepthEnable       = DXBoolean(depthDesc.testEnabled);
        stateDesc.DepthWriteMask    = (depthDesc.writeEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
        stateDesc.DepthFunc         = D3D11Types::Map(depthDesc.compareOp);
        stateDesc.StencilEnable     = DXBoolean(stencilDesc.testEnabled);
        stateDesc.StencilReadMask   = static_cast<UINT8>(stencilDesc.front.readMask);
        stateDesc.StencilWriteMask  = static_cast<UINT8>(stencilDesc.front.writeMask);

        Convert(stateDesc.FrontFace, stencilDesc.front);
        Convert(stateDesc.BackFace, stencilDesc.back);
    }
    auto hr = device->CreateDepthStencilState(&stateDesc, depthStencilState_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil state");
}

void D3D11GraphicsPipeline::CreateRasterizerState(ID3D11Device* device, const RasterizerDescriptor& desc)
{
    D3D11_RASTERIZER_DESC stateDesc;
    {
        stateDesc.FillMode              = D3D11Types::Map(desc.polygonMode);
        stateDesc.CullMode              = D3D11Types::Map(desc.cullMode);
        stateDesc.FrontCounterClockwise = DXBoolean(desc.frontCCW);
        stateDesc.DepthBias             = static_cast<INT>(desc.depthBias.constantFactor);
        stateDesc.DepthBiasClamp        = desc.depthBias.clamp;
        stateDesc.SlopeScaledDepthBias  = desc.depthBias.slopeFactor;
        stateDesc.DepthClipEnable       = DXBoolean(!desc.depthClampEnabled);
        stateDesc.ScissorEnable         = DXBoolean(desc.scissorTestEnabled);
        stateDesc.MultisampleEnable     = DXBoolean(desc.multiSampling.enabled);
        stateDesc.AntialiasedLineEnable = DXBoolean(desc.antiAliasedLineEnabled);
    }
    auto hr = device->CreateRasterizerState(&stateDesc, rasterizerState_.ReleaseAndGetAddressOf());
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
        stateDesc.AlphaToCoverageEnable  = DXBoolean(desc.alphaToCoverageEnabled);
        stateDesc.IndependentBlendEnable = DXBoolean(desc.targets.size() > 1);

        for (UINT i = 0, n = static_cast<UINT>(desc.targets.size()); i < 8u; ++i)
        {
            auto& targetState = stateDesc.RenderTarget[i];

            if (i < n)
            {
                const auto& targetDesc = desc.targets[i];

                targetState.BlendEnable             = DXBoolean(desc.blendEnabled);
                targetState.SrcBlend                = D3D11Types::Map(targetDesc.srcColor);
                targetState.DestBlend               = D3D11Types::Map(targetDesc.dstColor);
                targetState.BlendOp                 = D3D11Types::Map(targetDesc.colorArithmetic);
                targetState.SrcBlendAlpha           = D3D11Types::Map(targetDesc.srcAlpha);
                targetState.DestBlendAlpha          = D3D11Types::Map(targetDesc.dstAlpha);
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
    auto hr = device->CreateBlendState(&stateDesc, blendState_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 blend state");
}


} // /namespace LLGL



// ================================================================================
