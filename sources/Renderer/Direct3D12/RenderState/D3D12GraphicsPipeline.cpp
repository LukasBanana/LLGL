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
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <algorithm>
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12RenderSystem& renderSystem, const GraphicsPipelineDescriptor& desc) :
    primitiveTopology_ { D3D12Types::Map(desc.primitiveTopology) },
    scissorEnabled_    { desc.rasterizer.scissorTestEnabled      }
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(D3D12ShaderProgram*, desc.shaderProgram);

    /* Create root signature and graphics pipeline state  */
    CreateRootSignature(renderSystem, *shaderProgramD3D, desc);
    CreatePipelineState(renderSystem, *shaderProgramD3D, desc);
}

void D3D12GraphicsPipeline::CreateRootSignature(
    D3D12RenderSystem& renderSystem, D3D12ShaderProgram& shaderProgram, const GraphicsPipelineDescriptor& /*desc*/)
{
    /* Setup root signature flags */
    D3D12_ROOT_SIGNATURE_FLAGS signatureFlags =
    (
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT/* |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS   |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS*/
    );

    /* Setup descritpor structures for root signature */
    std::vector<CD3DX12_DESCRIPTOR_RANGE> signatureRange;

    auto AddSignatureRange = [&](D3D12_DESCRIPTOR_RANGE_TYPE type, UINT count)
    {
        if (count > 0)
        {
            CD3DX12_DESCRIPTOR_RANGE rangeDesc;
            rangeDesc.Init(type, count, 0);
            signatureRange.push_back(rangeDesc);
        }
    };

    AddSignatureRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shaderProgram.GetNumSRV());
    AddSignatureRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, shaderProgram.GetNumCBV());
    AddSignatureRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, shaderProgram.GetNumUAV());

    CD3DX12_ROOT_SIGNATURE_DESC signatureDesc;
    if (!signatureRange.empty())
    {
        CD3DX12_ROOT_PARAMETER signatureParam;
        signatureParam.InitAsDescriptorTable(static_cast<UINT>(signatureRange.size()), signatureRange.data(), D3D12_SHADER_VISIBILITY_ALL);

        signatureDesc.Init(1, &signatureParam, 0, nullptr, signatureFlags);
    }
    else
        signatureDesc.Init(0, nullptr, 0, nullptr, signatureFlags);

    /* Create serialized root signature */
    HRESULT             hr          = 0;
    ComPtr<ID3DBlob>    signature;
    ComPtr<ID3DBlob>    error;

    hr = D3D12SerializeRootSignature(
        &signatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.ReleaseAndGetAddressOf(),
        error.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr) && error)
    {
        auto errorStr = DXGetBlobString(error.Get());
        throw std::runtime_error("failed to serialize D3D12 root signature: " + errorStr);
    }

    DXThrowIfFailed(hr, "failed to serialize D3D12 root signature");

    /* Create actual root signature */
    hr = renderSystem.GetDevice()->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf())
    );

    DXThrowIfFailed(hr, "failed to create D3D12 root signature");
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
    D3D12RenderSystem& renderSystem, D3D12ShaderProgram& shaderProgram, const GraphicsPipelineDescriptor& desc)
{
    /* Get number of render-target attachments */
    UINT numAttachments = 1u; //TODO

    /* Setup D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};

    stateDesc.pRootSignature = rootSignature_.Get();

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
                stateDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
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
                stateDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
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
    stateDesc.SampleMask            = UINT_MAX;
    stateDesc.NumRenderTargets      = numAttachments;
    #if 1//TODO: currently not supported
    stateDesc.SampleDesc.Count      = 1; //!!!
    #else
    stateDesc.SampleDesc.Count      = desc.rasterizer.multiSampling.SampleCount();
    #endif

    /* Create graphics pipeline state and graphics command list */
    pipelineState_ = renderSystem.CreateDXGfxPipelineState(stateDesc);
}


} // /namespace LLGL



// ================================================================================
