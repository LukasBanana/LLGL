/*
 * D3D12GraphicsPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12GraphicsPSO.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12RenderPass.h"
#include "D3D12PipelineStateUtils.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770370(v=vs.85).aspx
D3D12GraphicsPSO::D3D12GraphicsPSO(
    ID3D12Device*                       device,
    D3D12PipelineLayout&                defaultPipelineLayout,
    const GraphicsPipelineDescriptor&   desc,
    const D3D12RenderPass*              defaultRenderPass,
    PipelineCache*                      pipelineCache)
:
    D3D12RenderPSOBase
    {
        D3D12PipelineType::Graphics,
        desc.stencil,
        desc.blend,
        desc.rasterizer.scissorTestEnabled,
        desc.viewports,
        desc.scissors,
        desc.pipelineLayout,
        GetShadersAsArray(desc),
        defaultPipelineLayout
    }
{
    /* Validate pointers and get D3D shader program */
    if (desc.vertexShader == nullptr)
    {
        ResetReport("cannot create D3D graphics PSO without vertex shader", true);
        return;
    }

    /* Use either default render pass or from descriptor */
    const D3D12RenderPass* renderPassD3D = nullptr;
    if (desc.renderPass != nullptr)
        renderPassD3D = LLGL_CAST(const D3D12RenderPass*, desc.renderPass);
    else
        renderPassD3D = defaultRenderPass;

    /* Store dynamic pipeline states */
    primitiveTopology_  = DXTypes::ToD3DPrimitiveTopology(desc.primitiveTopology);

    /* Get D3D pipeline layout */
    const D3D12PipelineLayout* pipelineLayoutD3D = nullptr;
    if (desc.pipelineLayout != nullptr)
        pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
    else
        pipelineLayoutD3D = &defaultPipelineLayout;

    /* Create native graphics PSO */
    if (pipelineCache != nullptr)
    {
        auto* pipelineCacheD3D = LLGL_CAST(D3D12PipelineCache*, pipelineCache);
        CreateNativePSO(device, *pipelineLayoutD3D, renderPassD3D, desc, pipelineCacheD3D);
    }
    else
        CreateNativePSO(device, *pipelineLayoutD3D, renderPassD3D, desc);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D12GraphicsPSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetGraphicsRootSignature(GetRootSignature());
    if (secondaryPSO_)
        commandContext.SetDeferredPipelineState(GetNative(), secondaryPSO_.Get());
    else
        commandContext.SetPipelineState(GetNative());

    /* Set dynamic pipeline states */
    ID3D12GraphicsCommandList* commandList = commandContext.GetCommandList();

    commandList->IASetPrimitiveTopology(primitiveTopology_);

    BindOutputMergerAndStaticStates(commandList);
}

static D3D12_SHADER_BYTECODE GetD3DShaderByteCode(const Shader* shader)
{
    if (shader != nullptr)
        return LLGL_CAST(const D3D12Shader*, shader)->GetByteCode();
    else
        return D3D12_SHADER_BYTECODE{ nullptr, 0 };
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveToplogyType(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineListAdjacency:
        case PrimitiveTopology::LineStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
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

static D3D12_INPUT_LAYOUT_DESC GetD3DInputLayoutDesc(const Shader* vs)
{
    D3D12_INPUT_LAYOUT_DESC desc = {};
    LLGL_CAST(const D3D12Shader*, vs)->GetInputLayoutDesc(desc);
    return desc;
}

static D3D12_STREAM_OUTPUT_DESC GetD3DStreamOutputDesc(const Shader* vs, const Shader* ds, const Shader* gs)
{
    D3D12_STREAM_OUTPUT_DESC desc = {};
    if (gs != nullptr)
        LLGL_CAST(const D3D12Shader*, gs)->GetStreamOutputDesc(desc);
    else if (ds != nullptr)
        LLGL_CAST(const D3D12Shader*, ds)->GetStreamOutputDesc(desc);
    else if (vs != nullptr)
        LLGL_CAST(const D3D12Shader*, vs)->GetStreamOutputDesc(desc);
    return desc;
}

static D3D12_INDEX_BUFFER_STRIP_CUT_VALUE GetIndexFormatStripCutValue(Format format)
{
    return (format == Format::R16UInt ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF);
}

void D3D12GraphicsPSO::CreateNativePSO(
    ID3D12Device*                       device,
    const D3D12PipelineLayout&          pipelineLayout,
    const D3D12RenderPass*              renderPass,
    const GraphicsPipelineDescriptor&   desc,
    D3D12PipelineCache*                 pipelineCache)
{
    /* Get number of render-target attachments */
    const UINT numAttachments = (renderPass != nullptr ? renderPass->GetNumColorAttachments() : 1);

    /* Initialize D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};
    stateDesc.pRootSignature = GetRootSignature();

    /* Get shader byte codes */
    stateDesc.VS = GetD3DShaderByteCode(desc.vertexShader);
    stateDesc.HS = GetD3DShaderByteCode(desc.tessControlShader);
    stateDesc.DS = GetD3DShaderByteCode(desc.tessEvaluationShader);
    stateDesc.GS = GetD3DShaderByteCode(desc.geometryShader);
    stateDesc.PS = GetD3DShaderByteCode(desc.fragmentShader);

    /* Convert blend state and depth-stencil format */
    if (renderPass != nullptr)
    {
        stateDesc.DSVFormat = renderPass->GetDSVFormat();
        D3DConvertBlendDesc(stateDesc.BlendState, stateDesc.RTVFormats, desc.blend, *renderPass);
    }
    else
    {
        stateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        D3DConvertBlendDesc(stateDesc.BlendState, stateDesc.RTVFormats, desc.blend, numAttachments);
    }

    /* Convert rasterizer state */
    D3DConvertRasterizerDesc(stateDesc.RasterizerState, desc.rasterizer);

    /* Convert depth-stencil state */
    D3DConvertDepthStencilDesc(stateDesc.DepthStencilState, desc.depth, desc.stencil);

    /* Convert other states */
    const bool isStripTopology = IsPrimitiveTopologyStrip(desc.primitiveTopology);
    stateDesc.InputLayout           = GetD3DInputLayoutDesc(desc.vertexShader);
    stateDesc.StreamOutput          = GetD3DStreamOutputDesc(desc.vertexShader, desc.tessEvaluationShader, desc.geometryShader);
    stateDesc.IBStripCutValue       = (isStripTopology ? GetIndexFormatStripCutValue(desc.indexFormat) : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED);
    stateDesc.PrimitiveTopologyType = GetPrimitiveToplogyType(desc.primitiveTopology);
    stateDesc.SampleMask            = desc.blend.sampleMask;
    stateDesc.NumRenderTargets      = numAttachments;
    stateDesc.SampleDesc.Count      = (renderPass != nullptr ? renderPass->GetSampleDesc().Count : 1);
    stateDesc.SampleDesc.Quality    = 0;

    /* If rasterizer stage is discarded, don't sent stream-output data to the rasterizer */
    if (desc.rasterizer.discardEnabled)
        stateDesc.StreamOutput.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

    /* Set PSO cache if specified */
    if (pipelineCache != nullptr)
        stateDesc.CachedPSO = pipelineCache->GetCachedPSO();

    /* Create native PSO */
    ComPtr<ID3D12PipelineState> primaryPSO;

    if (isStripTopology && desc.indexFormat == Format::Undefined)
    {
        /* Create primary PSO with 32-bit index cut off value */
        primaryPSO = CreateNativePSOWithDesc(device, stateDesc, desc.debugName);

        /* Create secondary PSO with 16-bit index cut off value */
        stateDesc.IBStripCutValue   = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        stateDesc.CachedPSO         = {};
        secondaryPSO_ = CreateNativePSOWithDesc(device, stateDesc, desc.debugName);
    }
    else
        primaryPSO = CreateNativePSOWithDesc(device, stateDesc, desc.debugName);

    SetNativeAndUpdateCache(std::move(primaryPSO), pipelineCache);
}

ComPtr<ID3D12PipelineState> D3D12GraphicsPSO::CreateNativePSOWithDesc(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const char* debugName)
{
    ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to create D3D12 graphics pipeline state [%s] (HRESULT = %s)\n", GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
        return nullptr;
    }
    return pipelineState;
}


} // /namespace LLGL



// ================================================================================
