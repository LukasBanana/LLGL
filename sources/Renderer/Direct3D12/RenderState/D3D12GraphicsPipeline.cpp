/*
 * D3D12GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12GraphicsPipeline.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "../DXCore.h"
#include "../DXTypes.h"
#include "../../CheckedCast.h"
#include "../../Assertion.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


static D3D12_CONSERVATIVE_RASTERIZATION_MODE GetConservativeRaster(bool enabled)
{
    return (enabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}

static D3D12_SHADER_BYTECODE GetShaderByteCode(D3D12Shader* shader)
{
    return (shader != nullptr ? shader->GetByteCode() : D3D12_SHADER_BYTECODE{ nullptr, 0 });
}

static UINT8 GetColorWriteMask(const ColorRGBAb& color)
{
    UINT8 mask = 0;

    if (color.r)
        mask |= D3D12_COLOR_WRITE_ENABLE_RED;
    if (color.g)
        mask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    if (color.b)
        mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    if (color.a)
        mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

    return mask;
}

static void Convert(D3D12_DEPTH_STENCILOP_DESC& to, const StencilFaceDescriptor& from)
{
    to.StencilFailOp        = DXTypes::Map(from.stencilFailOp);
    to.StencilDepthFailOp   = DXTypes::Map(from.depthFailOp);
    to.StencilPassOp        = DXTypes::Map(from.depthPassOp);
    to.StencilFunc          = DXTypes::Map(from.compareOp);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPipeline::D3D12GraphicsPipeline(
    ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3D12CommandAllocator* commandAlloc, const GraphicsPipelineDescriptor& desc)
{
    /* Validate pointers */
    //LLGL_ASSERT_PTR(rootSignature);
    //LLGL_ASSERT_PTR(commandAlloc);
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(D3D12ShaderProgram*, desc.shaderProgram);

    /* Setup D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc;
    InitMemory(stateDesc);

    stateDesc.pRootSignature = rootSignature;

    /* Get shader byte codes */
    stateDesc.VS = GetShaderByteCode(shaderProgramD3D->GetVS());
    stateDesc.PS = GetShaderByteCode(shaderProgramD3D->GetPS());
    stateDesc.DS = GetShaderByteCode(shaderProgramD3D->GetDS());
    stateDesc.HS = GetShaderByteCode(shaderProgramD3D->GetHS());
    stateDesc.GS = GetShaderByteCode(shaderProgramD3D->GetGS());

    /* Convert blend state */
  //stateDesc.BlendState.AlphaToCoverageEnable      = FALSE;
    stateDesc.BlendState.IndependentBlendEnable     = (desc.blend.targets.size() > 1 ? TRUE : FALSE);

    for (UINT i = 0, n = std::min(8u, static_cast<UINT>(desc.blend.targets.size())); i < n; ++i)
    {
        const auto& targetDesc = desc.blend.targets[i];
        auto& targetState = stateDesc.BlendState.RenderTarget[i];

        targetState.BlendEnable             = desc.blend.blendEnabled;
        targetState.LogicOpEnable           = FALSE;
        targetState.SrcBlend                = DXTypes::Map(targetDesc.srcColor);
        targetState.DestBlend               = DXTypes::Map(targetDesc.destColor);
        targetState.BlendOp                 = DXTypes::Map(targetDesc.colorArithmetic);
        targetState.SrcBlendAlpha           = DXTypes::Map(targetDesc.srcAlpha);
        targetState.DestBlendAlpha          = DXTypes::Map(targetDesc.destAlpha);
        targetState.BlendOpAlpha            = DXTypes::Map(targetDesc.alphaArithmetic);
        targetState.LogicOp                 = D3D12_LOGIC_OP_NOOP;
        targetState.RenderTargetWriteMask   = GetColorWriteMask(targetDesc.colorMask);
    }

    /* Convert rasterizer state */
    stateDesc.RasterizerState.FillMode              = DXTypes::Map(desc.rasterizer.polygonMode);
    stateDesc.RasterizerState.CullMode              = DXTypes::Map(desc.rasterizer.cullMode);
    stateDesc.RasterizerState.FrontCounterClockwise = (desc.rasterizer.frontCCW ? TRUE : FALSE);
    stateDesc.RasterizerState.DepthBias             = desc.rasterizer.depthBias;
    stateDesc.RasterizerState.DepthBiasClamp        = desc.rasterizer.depthBiasClamp;
    stateDesc.RasterizerState.SlopeScaledDepthBias  = desc.rasterizer.slopeScaledDepthBias;
    stateDesc.RasterizerState.DepthClipEnable       = (desc.rasterizer.depthClampEnabled ? TRUE : FALSE);
    stateDesc.RasterizerState.MultisampleEnable     = (desc.rasterizer.multiSampleEnabled ? TRUE : FALSE);
    stateDesc.RasterizerState.AntialiasedLineEnable = (desc.rasterizer.antiAliasedLineEnabled ? TRUE : FALSE);
    stateDesc.RasterizerState.ForcedSampleCount     = 0; // no forced sample count
    stateDesc.RasterizerState.ConservativeRaster    = GetConservativeRaster(desc.rasterizer.conservativeRasterization);

    /* Convert depth-stencil state */
    stateDesc.DepthStencilState.DepthEnable         = (desc.depth.testEnabled ? TRUE : FALSE);
    stateDesc.DepthStencilState.DepthWriteMask      = (desc.depth.writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO);
    stateDesc.DepthStencilState.DepthFunc           = DXTypes::Map(desc.depth.compareOp);
    stateDesc.DepthStencilState.StencilEnable       = (desc.stencil.testEnabled ? TRUE : FALSE);
    stateDesc.DepthStencilState.StencilReadMask     = D3D12_DEFAULT_STENCIL_READ_MASK;
    stateDesc.DepthStencilState.StencilWriteMask    = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    
    Convert(stateDesc.DepthStencilState.FrontFace, desc.stencil.front);
    Convert(stateDesc.DepthStencilState.BackFace, desc.stencil.back);

    /* Convert other states */
    stateDesc.InputLayout           = shaderProgramD3D->GetInputLayoutDesc();
  //stateDesc.IBStripCutValue       = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    stateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    stateDesc.SampleMask            = UINT_MAX;
    stateDesc.NumRenderTargets      = 8;
    stateDesc.SampleDesc.Count      = desc.rasterizer.samples;

    for (UINT i = 0; i < stateDesc.NumRenderTargets; ++i)
        stateDesc.RTVFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;

    /* Create D3D12 graphics pipeline sate */
    auto hr = device->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(&pipelineState_));
    DXThrowIfFailed(hr, "failed to create D3D12 graphics pipeline state");

    /* Create command list */
    //hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc, pipelineState_, IID_PPV_ARGS(&commandList_));
    //DXThrowIfFailed(hr, "failed to create D3D12 command list");
}

D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
{
    SafeRelease(pipelineState_);
    SafeRelease(commandList_);
}

/*void D3D12GraphicsPipeline::Bind(D3D12StateManager& stateMngr)
{
}*/


} // /namespace LLGL



// ================================================================================
