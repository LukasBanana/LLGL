/*
 * D3D12GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12GraphicsPipeline.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12PipelineLayout.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <LLGL/GraphicsPipelineFlags.h>
#include <algorithm>


namespace LLGL
{


// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPipeline::D3D12GraphicsPipeline(
    D3D12Device&                        device,
    ID3D12RootSignature*                defaultRootSignature,
    const GraphicsPipelineDescriptor&   desc)
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(const D3D12ShaderProgram*, desc.shaderProgram);

    if (auto pipelineLayout = desc.pipelineLayout)
    {
        /* Create pipeline state with root signature from pipeline layout */
        auto pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, pipelineLayout);
        CreatePipelineState(device, *shaderProgramD3D, pipelineLayoutD3D->GetRootSignature(), desc);
    }
    else
    {
        /* Create pipeline state with default root signature */
        CreatePipelineState(device, *shaderProgramD3D, defaultRootSignature, desc);
    }

    /* Store dynamic pipeline states */
    primitiveTopology_  = D3D12Types::Map(desc.primitiveTopology);
    stencilRef_         = desc.stencil.front.reference;
    blendFactor_[0]     = desc.blend.blendFactor.r;
    blendFactor_[1]     = desc.blend.blendFactor.g;
    blendFactor_[2]     = desc.blend.blendFactor.b;
    blendFactor_[3]     = desc.blend.blendFactor.a;
    scissorEnabled_     = desc.rasterizer.scissorTestEnabled;
}

void D3D12GraphicsPipeline::Bind(ID3D12GraphicsCommandList* commandList)
{
    /* Set root signature and pipeline state */
    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(pipelineState_.Get());

    /* Set dynamic pipeline states */
    commandList->IASetPrimitiveTopology(primitiveTopology_);
    commandList->OMSetBlendFactor(blendFactor_);
    commandList->OMSetStencilRef(stencilRef_);
}

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

    if (color.r) { mask |= D3D12_COLOR_WRITE_ENABLE_RED;   }
    if (color.g) { mask |= D3D12_COLOR_WRITE_ENABLE_GREEN; }
    if (color.b) { mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;  }
    if (color.a) { mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA; }

    return mask;
}

static void Convert(D3D12_DEPTH_STENCILOP_DESC& dst, const StencilFaceDescriptor& src)
{
    dst.StencilFailOp       = D3D12Types::Map(src.stencilFailOp);
    dst.StencilDepthFailOp  = D3D12Types::Map(src.depthFailOp);
    dst.StencilPassOp       = D3D12Types::Map(src.depthPassOp);
    dst.StencilFunc         = D3D12Types::Map(src.compareOp);
}

static void Convert(D3D12_RENDER_TARGET_BLEND_DESC& dst, const BlendTargetDescriptor& src, const BlendDescriptor& blendDesc)
{
    dst.BlendEnable             = DXBoolean(blendDesc.blendEnabled);
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12Types::Map(src.srcColor);
    dst.DestBlend               = D3D12Types::Map(src.dstColor);
    dst.BlendOp                 = D3D12Types::Map(src.colorArithmetic);
    dst.SrcBlendAlpha           = D3D12Types::Map(src.srcAlpha);
    dst.DestBlendAlpha          = D3D12Types::Map(src.dstAlpha);
    dst.BlendOpAlpha            = D3D12Types::Map(src.alphaArithmetic);
    dst.LogicOp                 = D3D12Types::Map(blendDesc.logicOp);
    dst.RenderTargetWriteMask   = GetColorWriteMask(src.colorMask);
}

static void SetBlendDescToDefault(D3D12_RENDER_TARGET_BLEND_DESC& dst)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = FALSE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

static void SetBlendDescToLogicOp(D3D12_RENDER_TARGET_BLEND_DESC& dst, D3D12_LOGIC_OP logicOp)
{
    dst.BlendEnable             = FALSE;
    dst.LogicOpEnable           = TRUE;
    dst.SrcBlend                = D3D12_BLEND_ONE;
    dst.DestBlend               = D3D12_BLEND_ZERO;
    dst.BlendOp                 = D3D12_BLEND_OP_ADD;
    dst.SrcBlendAlpha           = D3D12_BLEND_ONE;
    dst.DestBlendAlpha          = D3D12_BLEND_ZERO;
    dst.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
    dst.LogicOp                 = logicOp;
    dst.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveToplogyType(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineLoop:
        case PrimitiveTopology::LineListAdjacency:
        case PrimitiveTopology::LineStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::TriangleListAdjacency:
        case PrimitiveTopology::TriangleStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        default:
            if (topology >= PrimitiveTopology::Patches1 && topology <= PrimitiveTopology::Patches32)
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            break;
    }
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

void D3D12GraphicsPipeline::CreatePipelineState(
    D3D12Device&                        device,
    const D3D12ShaderProgram&           shaderProgram,
    ID3D12RootSignature*                rootSignature,
    const GraphicsPipelineDescriptor&   desc)
{
    /* Store used root signature */
    rootSignature_ = rootSignature;

    /* Get number of render-target attachments */
    UINT numAttachments = 1u; //TODO: get information from <RenderPass> interface

    /* Setup D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};

    stateDesc.pRootSignature = rootSignature;

    /* Get shader byte codes */
    stateDesc.VS = GetShaderByteCode(shaderProgram.GetVS());
    stateDesc.PS = GetShaderByteCode(shaderProgram.GetPS());
    stateDesc.DS = GetShaderByteCode(shaderProgram.GetDS());
    stateDesc.HS = GetShaderByteCode(shaderProgram.GetHS());
    stateDesc.GS = GetShaderByteCode(shaderProgram.GetGS());

    /* Initialize stream-output */
    #if 0//TODO
    stateDesc.StreamOutput.pSODeclaration   = nullptr;
    stateDesc.StreamOutput.NumEntries       = 0;
    stateDesc.StreamOutput.pBufferStrides   = nullptr;
    stateDesc.StreamOutput.NumStrides       = 0;
    stateDesc.StreamOutput.RasterizedStream = 0;
    #endif

    /* Initialize depth-stencil format */
    stateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    /* Convert blend state */
    stateDesc.BlendState.AlphaToCoverageEnable  = DXBoolean(desc.blend.alphaToCoverageEnabled);

    if (desc.blend.logicOp == LogicOp::Disabled)
    {
        /* Enable independent blend states when multiple targets are specified */
        stateDesc.BlendState.IndependentBlendEnable = DXBoolean(desc.blend.targets.size() > 1);

        for (UINT i = 0, n = std::min(numAttachments, static_cast<UINT>(desc.blend.targets.size())); i < 8u; ++i)
        {
            if (i < n)
            {
                /* Convert blend target descriptor */
                Convert(stateDesc.BlendState.RenderTarget[i], desc.blend.targets[i], desc.blend);
                stateDesc.RTVFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(stateDesc.BlendState.RenderTarget[i]);
                stateDesc.RTVFormats[i] = (i > 0 ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_B8G8R8A8_UNORM);
            }
        }
    }
    else
    {
        /* Independent blend states is not allowed when logic operations are used */
        stateDesc.BlendState.IndependentBlendEnable = FALSE;

        const auto logicOp = D3D12Types::Map(desc.blend.logicOp);

        for (UINT i = 0; i < 8u; ++i)
        {
            if (i < numAttachments)
            {
                /*
                Special output format required for logic operations
                see https://msdn.microsoft.com/en-us/library/windows/desktop/mt426648(v=vs.85).aspx
                */
                SetBlendDescToLogicOp(stateDesc.BlendState.RenderTarget[i], logicOp);
                stateDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UINT;
            }
            else
            {
                /* Initialize blend target to default values */
                SetBlendDescToDefault(stateDesc.BlendState.RenderTarget[i]);
                stateDesc.RTVFormats[i] = (i > 0 ? DXGI_FORMAT_UNKNOWN : DXGI_FORMAT_B8G8R8A8_UNORM);
            }
        }
    }

    /* Convert rasterizer state */
    stateDesc.RasterizerState.FillMode              = D3D12Types::Map(desc.rasterizer.polygonMode);
    stateDesc.RasterizerState.CullMode              = D3D12Types::Map(desc.rasterizer.cullMode);
    stateDesc.RasterizerState.FrontCounterClockwise = DXBoolean(desc.rasterizer.frontCCW);
    stateDesc.RasterizerState.DepthBias             = static_cast<INT>(desc.rasterizer.depthBias.constantFactor);
    stateDesc.RasterizerState.DepthBiasClamp        = desc.rasterizer.depthBias.clamp;
    stateDesc.RasterizerState.SlopeScaledDepthBias  = desc.rasterizer.depthBias.slopeFactor;
    stateDesc.RasterizerState.DepthClipEnable       = DXBoolean(!desc.rasterizer.depthClampEnabled);
    #if 1//TODO: currently not supported
    stateDesc.RasterizerState.MultisampleEnable     = FALSE; //!!!
    #else
    stateDesc.RasterizerState.MultisampleEnable     = DXBoolean(desc.rasterizer.multiSampling.enabled);
    #endif
    stateDesc.RasterizerState.AntialiasedLineEnable = DXBoolean(desc.rasterizer.antiAliasedLineEnabled);
    stateDesc.RasterizerState.ForcedSampleCount     = 0; // no forced sample count
    stateDesc.RasterizerState.ConservativeRaster    = GetConservativeRaster(desc.rasterizer.conservativeRasterization);

    /* Convert depth-stencil state */
    stateDesc.DepthStencilState.DepthEnable         = DXBoolean(desc.depth.testEnabled);
    stateDesc.DepthStencilState.DepthWriteMask      = (desc.depth.writeEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO);
    stateDesc.DepthStencilState.DepthFunc           = D3D12Types::Map(desc.depth.compareOp);
    stateDesc.DepthStencilState.StencilEnable       = DXBoolean(desc.stencil.testEnabled);
    stateDesc.DepthStencilState.StencilReadMask     = static_cast<UINT8>(desc.stencil.front.readMask);
    stateDesc.DepthStencilState.StencilWriteMask    = static_cast<UINT8>(desc.stencil.front.writeMask);

    Convert(stateDesc.DepthStencilState.FrontFace, desc.stencil.front);
    Convert(stateDesc.DepthStencilState.BackFace, desc.stencil.back);

    /* Convert other states */
    stateDesc.InputLayout           = shaderProgram.GetInputLayoutDesc();
    stateDesc.IBStripCutValue       = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    stateDesc.PrimitiveTopologyType = GetPrimitiveToplogyType(desc.primitiveTopology);
    stateDesc.SampleMask            = desc.rasterizer.multiSampling.sampleMask;
    stateDesc.NumRenderTargets      = numAttachments;
    #if 1//TODO: currently not supported
    stateDesc.SampleDesc.Count      = 1; //!!!
    #else
    stateDesc.SampleDesc.Count      = desc.rasterizer.multiSampling.SampleCount();
    #endif

    /* Create graphics pipeline state and graphics command list */
    pipelineState_ = device.CreateDXPipelineState(stateDesc);
}


} // /namespace LLGL



// ================================================================================
