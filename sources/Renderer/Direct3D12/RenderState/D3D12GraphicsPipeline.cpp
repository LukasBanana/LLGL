/*
 * D3D12GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12GraphicsPipeline.h"
#include "../D3D12RenderSystem.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "../D3DX12/d3dx12.h"
#include "../DXCore.h"
#include "../DXTypes.h"
#include "../../CheckedCast.h"
#include "../../Assertion.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPipeline::D3D12GraphicsPipeline(
    D3D12RenderSystem& renderSystem, const GraphicsPipelineDescriptor& desc)
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(D3D12ShaderProgram*, desc.shaderProgram);

    /* Store D3D primitive topology */
    primitiveTopology_ = DXTypes::Map(desc.primitiveTopology);

    /* Create root signature and graphics pipeline state  */
    CreateRootSignature(renderSystem, *shaderProgramD3D, desc);
    CreatePipelineState(renderSystem, *shaderProgramD3D, desc);
}

/*void D3D12GraphicsPipeline::Bind(D3D12StateManager& stateMngr)
{
}*/

void D3D12GraphicsPipeline::CreateRootSignature(
    D3D12RenderSystem& renderSystem, D3D12ShaderProgram& shaderProgram, const GraphicsPipelineDescriptor& desc)
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

    AddSignatureRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, shaderProgram.GetNumConstantBuffers());
    AddSignatureRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, shaderProgram.GetNumStorageBuffers());

    CD3DX12_ROOT_SIGNATURE_DESC signatureDesc;
    if (!signatureRange.empty())
    {
        CD3DX12_ROOT_PARAMETER signatureParam;
        signatureParam.InitAsDescriptorTable(signatureRange.size(), signatureRange.data(), D3D12_SHADER_VISIBILITY_ALL);

        signatureDesc.Init(1, &signatureParam, 0, nullptr, signatureFlags);
    }
    else
        signatureDesc.Init(0, nullptr, 0, nullptr, signatureFlags);

    /* Create serialized root signature */
    HRESULT             hr          = 0;
    ComPtr<ID3DBlob>    signature;
    ComPtr<ID3DBlob>    error;

    hr = D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    
    if (FAILED(hr) && error)
    {
        auto errorStr = DXGetBlobString(error.Get());
        throw std::runtime_error("failed to serialize D3D12 root signature: " + errorStr);
    }

    DXThrowIfFailed(hr, "failed to serialize D3D12 root signature");

    /* Create actual root signature */
    hr = renderSystem.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
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

static void Convert(D3D12_DEPTH_STENCILOP_DESC& to, const StencilFaceDescriptor& from)
{
    to.StencilFailOp        = DXTypes::Map(from.stencilFailOp);
    to.StencilDepthFailOp   = DXTypes::Map(from.depthFailOp);
    to.StencilPassOp        = DXTypes::Map(from.depthPassOp);
    to.StencilFunc          = DXTypes::Map(from.compareOp);
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
    /* Setup D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc;
    InitMemory(stateDesc);

    stateDesc.pRootSignature = rootSignature_.Get();

    /* Get shader byte codes */
    stateDesc.VS = GetShaderByteCode(shaderProgram.GetVS());
    stateDesc.PS = GetShaderByteCode(shaderProgram.GetPS());
    stateDesc.DS = GetShaderByteCode(shaderProgram.GetDS());
    stateDesc.HS = GetShaderByteCode(shaderProgram.GetHS());
    stateDesc.GS = GetShaderByteCode(shaderProgram.GetGS());

    /* Convert blend state */
    stateDesc.BlendState.AlphaToCoverageEnable  = FALSE;
    stateDesc.BlendState.IndependentBlendEnable = (desc.blend.targets.size() > 1 ? TRUE : FALSE);

    for (UINT i = 0, n = static_cast<UINT>(desc.blend.targets.size()); i < 8u; ++i)
    {
        auto& targetState = stateDesc.BlendState.RenderTarget[i];

        if (i < n)
        {
            const auto& targetDesc = desc.blend.targets[i];

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
        else
        {
            targetState.BlendEnable             = FALSE;
            targetState.LogicOpEnable           = FALSE;
            targetState.SrcBlend                = D3D12_BLEND_ONE;
            targetState.DestBlend               = D3D12_BLEND_ZERO;
            targetState.BlendOp                 = D3D12_BLEND_OP_ADD;
            targetState.SrcBlendAlpha           = D3D12_BLEND_ONE;
            targetState.DestBlendAlpha          = D3D12_BLEND_ZERO;
            targetState.BlendOpAlpha            = D3D12_BLEND_OP_ADD;
            targetState.LogicOp                 = D3D12_LOGIC_OP_NOOP;
            targetState.RenderTargetWriteMask   = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
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
    stateDesc.InputLayout           = shaderProgram.GetInputLayoutDesc();
    stateDesc.IBStripCutValue       = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    stateDesc.PrimitiveTopologyType = GetPrimitiveToplogyType(desc.primitiveTopology);
    stateDesc.SampleMask            = UINT_MAX;
    stateDesc.NumRenderTargets      = 1;//8;
    stateDesc.SampleDesc.Count      = desc.rasterizer.samples;
    
    for (UINT i = 0; i < 8u; ++i)
        stateDesc.RTVFormats[i] = (i < stateDesc.NumRenderTargets ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_UNKNOWN);

    /* Create graphics pipeline state and graphics command list */
    pipelineState_ = renderSystem.CreateDXGfxPipelineState(stateDesc);
}


} // /namespace LLGL



// ================================================================================
