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
#include <LLGL/Container/SmallVector.h>


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

static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(const PrimitiveTopology topology)
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
    stateDesc.PrimitiveTopologyType = GetPrimitiveTopologyType(desc.primitiveTopology);
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

    /* Pick up the render pass's view mask for multiview rendering (0 = no multiview). */
    const std::uint32_t viewMask = (renderPass != nullptr ? renderPass->GetViewMask() : 0u);

    /* Create native PSO */
    ComPtr<ID3D12PipelineState> primaryPSO;

    if (isStripTopology && desc.indexFormat == Format::Undefined)
    {
        /* Create primary PSO with 32-bit index cut off value */
        primaryPSO = CreateNativePSOWithDesc(device, stateDesc, desc.debugName, viewMask);

        /* Create secondary PSO with 16-bit index cut off value */
        stateDesc.IBStripCutValue   = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        stateDesc.CachedPSO         = {};
        secondaryPSO_ = CreateNativePSOWithDesc(device, stateDesc, desc.debugName, viewMask);
    }
    else
        primaryPSO = CreateNativePSOWithDesc(device, stateDesc, desc.debugName, viewMask);

    SetNativeAndUpdateCache(std::move(primaryPSO), pipelineCache);
}

// Minimal stream-PSO subobject helper. Each subobject in a D3D12_PIPELINE_STATE_STREAM_DESC is a
// (type-tag, value) pair, each starting on a void*-aligned boundary - which alignas(void*) gives us.
namespace
{
    template <typename Inner, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TypeTag>
    struct alignas(void*) StreamSubobject
    {
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type    = TypeTag;
        Inner                               value   {};

        StreamSubobject() = default;
        StreamSubobject(const Inner& v) : value(v) {}
        StreamSubobject& operator = (const Inner& v) { value = v; return *this; }
    };

    using Stream_RootSignature   = StreamSubobject<ID3D12RootSignature*,         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
    using Stream_VS              = StreamSubobject<D3D12_SHADER_BYTECODE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>;
    using Stream_PS              = StreamSubobject<D3D12_SHADER_BYTECODE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
    using Stream_DS              = StreamSubobject<D3D12_SHADER_BYTECODE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>;
    using Stream_HS              = StreamSubobject<D3D12_SHADER_BYTECODE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>;
    using Stream_GS              = StreamSubobject<D3D12_SHADER_BYTECODE,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>;
    using Stream_StreamOutput    = StreamSubobject<D3D12_STREAM_OUTPUT_DESC,     D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>;
    using Stream_Blend           = StreamSubobject<D3D12_BLEND_DESC,             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>;
    using Stream_SampleMask      = StreamSubobject<UINT,                         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>;
    using Stream_Rasterizer      = StreamSubobject<D3D12_RASTERIZER_DESC,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>;
    using Stream_DepthStencil    = StreamSubobject<D3D12_DEPTH_STENCIL_DESC,     D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>;
    using Stream_InputLayout     = StreamSubobject<D3D12_INPUT_LAYOUT_DESC,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>;
    using Stream_StripCut        = StreamSubobject<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>;
    using Stream_PrimitiveTopo   = StreamSubobject<D3D12_PRIMITIVE_TOPOLOGY_TYPE,D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>;
    using Stream_RTVFormats      = StreamSubobject<D3D12_RT_FORMAT_ARRAY,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
    using Stream_DSVFormat       = StreamSubobject<DXGI_FORMAT,                  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
    using Stream_SampleDesc      = StreamSubobject<DXGI_SAMPLE_DESC,             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>;
    using Stream_NodeMask        = StreamSubobject<UINT,                         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>;
    using Stream_CachedPSO       = StreamSubobject<D3D12_CACHED_PIPELINE_STATE,  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO>;
    using Stream_ViewInstancing  = StreamSubobject<D3D12_VIEW_INSTANCING_DESC,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING>;

    // Bundles every subobject we want to forward from the legacy desc, plus the new view instancing entry.
    struct GraphicsPipelineStream
    {
        Stream_RootSignature    rootSignature;
        Stream_VS               vs;
        Stream_PS               ps;
        Stream_DS               ds;
        Stream_HS               hs;
        Stream_GS               gs;
        Stream_StreamOutput     streamOutput;
        Stream_Blend            blend;
        Stream_SampleMask       sampleMask;
        Stream_Rasterizer       rasterizer;
        Stream_DepthStencil     depthStencil;
        Stream_InputLayout      inputLayout;
        Stream_StripCut         stripCut;
        Stream_PrimitiveTopo    primitiveTopology;
        Stream_RTVFormats       rtvFormats;
        Stream_DSVFormat        dsvFormat;
        Stream_SampleDesc       sampleDesc;
        Stream_NodeMask         nodeMask;
        Stream_CachedPSO        cachedPso;
        Stream_ViewInstancing   viewInstancing;
    };
}

ComPtr<ID3D12PipelineState> D3D12GraphicsPSO::CreateNativePSOWithDesc(
    ID3D12Device*                               device,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC&   desc,
    const char*                                 debugName,
    std::uint32_t                               viewMask)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    if (viewMask != 0)
    {
        // View Instancing is only available via the pipeline-state-stream API on ID3D12Device2.
        ComPtr<ID3D12Device2> device2;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(device2.GetAddressOf()))))
        {
            GetMutableReport().Errorf(
                "Failed to create D3D12 graphics pipeline state [%s]: multiview (View Instancing) requires ID3D12Device2 which is unavailable on this device\n",
                GetOptionalDebugName(debugName));
            return nullptr;
        }

        // Build one ViewInstanceLocation per set bit. The render-target-array-index is the bit position;
        // viewport-array-index stays at 0 (LLGL doesn't expose multi-viewport in PSO state today).
        std::uint32_t numLocations = 0;
        for (std::uint32_t mask = viewMask; mask != 0; mask &= (mask - 1))
            ++numLocations;
        SmallVector<D3D12_VIEW_INSTANCE_LOCATION, 8> locations;
        locations.reserve(numLocations);
        for (std::uint32_t bit = 0; bit < 32; ++bit)
        {
            if ((viewMask & (1u << bit)) != 0)
                locations.push_back({ /*ViewportArrayIndex=*/0u, /*RenderTargetArrayIndex=*/bit });
        }

        D3D12_VIEW_INSTANCING_DESC viDesc{};
        viDesc.ViewInstanceCount        = static_cast<UINT>(locations.size());
        viDesc.pViewInstanceLocations   = locations.data();
        viDesc.Flags                    = D3D12_VIEW_INSTANCING_FLAG_NONE;

        D3D12_RT_FORMAT_ARRAY rtvFormatArray{};
        rtvFormatArray.NumRenderTargets = desc.NumRenderTargets;
        for (UINT i = 0; i < desc.NumRenderTargets; ++i)
            rtvFormatArray.RTFormats[i] = desc.RTVFormats[i];

        GraphicsPipelineStream stream{};
        stream.rootSignature        = desc.pRootSignature;
        stream.vs                   = desc.VS;
        stream.ps                   = desc.PS;
        stream.ds                   = desc.DS;
        stream.hs                   = desc.HS;
        stream.gs                   = desc.GS;
        stream.streamOutput         = desc.StreamOutput;
        stream.blend                = desc.BlendState;
        stream.sampleMask           = desc.SampleMask;
        stream.rasterizer           = desc.RasterizerState;
        stream.depthStencil         = desc.DepthStencilState;
        stream.inputLayout          = desc.InputLayout;
        stream.stripCut             = desc.IBStripCutValue;
        stream.primitiveTopology    = desc.PrimitiveTopologyType;
        stream.rtvFormats           = rtvFormatArray;
        stream.dsvFormat            = desc.DSVFormat;
        stream.sampleDesc           = desc.SampleDesc;
        stream.nodeMask             = desc.NodeMask;
        stream.cachedPso            = desc.CachedPSO;
        stream.viewInstancing       = viDesc;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
        streamDesc.SizeInBytes                  = sizeof(stream);
        streamDesc.pPipelineStateSubobjectStream = &stream;

        HRESULT hr = device2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
        if (FAILED(hr))
        {
            GetMutableReport().Errorf(
                "Failed to create D3D12 graphics pipeline state [%s] with view instancing (HRESULT = %s)\n",
                GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
            return nullptr;
        }
        return pipelineState;
    }

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
