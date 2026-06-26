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

    /* Create native PSO */
    ComPtr<ID3D12PipelineState> primaryPSO;

    #ifdef LLGL_D3D12_ENABLE_DXCOMPILER
    /*
    Multiview (single-pass layered) rendering is implemented with D3D12 view instancing, which requires the
    stream-based pipeline state API. This path is taken only when the render pass has more than one view; all
    other pipelines keep the legacy CreateGraphicsPipelineState path untouched.
    */
    const UINT numViews = (renderPass != nullptr ? renderPass->GetNumViews() : 1);
    if (numViews > 1)
    {
        primaryPSO = CreateNativePSOAsViewInstanced(device, stateDesc, numViews, desc.debugName);

        if (isStripTopology && desc.indexFormat == Format::Undefined)
        {
            /* Create secondary PSO with 16-bit index cut off value */
            stateDesc.IBStripCutValue   = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
            stateDesc.CachedPSO         = {};
            secondaryPSO_ = CreateNativePSOAsViewInstanced(device, stateDesc, numViews, desc.debugName);
        }

        SetNativeAndUpdateCache(std::move(primaryPSO), pipelineCache);
        return;
    }
    #endif // /LLGL_D3D12_ENABLE_DXCOMPILER

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

#ifdef LLGL_D3D12_ENABLE_DXCOMPILER

template <D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TType, typename TObject>
struct alignas(void*) D3DPipelineStreamSubobject
{
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type    = { TType };
    TObject                             object  = {};

    D3DPipelineStreamSubobject() = default;

    inline D3DPipelineStreamSubobject& operator = (TObject object)
    {
        this->object = std::move(object);
        return *this;
    }
};

/*
Stream layout for a view-instanced graphics PSO. Mirrors the fields of D3D12_GRAPHICS_PIPELINE_STATE_DESC and
appends the VIEW_INSTANCING subobject; the legacy descriptor is converted into this stream below.
*/
struct D3DGraphicsPipelineStream
{
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE,         ID3D12RootSignature*        > rootSignature;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS,                     D3D12_SHADER_BYTECODE       > vs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS,                     D3D12_SHADER_BYTECODE       > hs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS,                     D3D12_SHADER_BYTECODE       > ds;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS,                     D3D12_SHADER_BYTECODE       > gs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS,                     D3D12_SHADER_BYTECODE       > ps;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT,          D3D12_STREAM_OUTPUT_DESC    > streamOutput;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND,                  D3D12_BLEND_DESC            > blendDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK,            UINT                        > sampleMask;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER,             D3D12_RASTERIZER_DESC       > rasterizerDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL,          D3D12_DEPTH_STENCIL_DESC    > depthStencilDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT,           D3D12_INPUT_LAYOUT_DESC     > inputLayout;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE,     D3D12_INDEX_BUFFER_STRIP_CUT_VALUE > ibStripCut;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY,     D3D12_PRIMITIVE_TOPOLOGY_TYPE > primitiveTopology;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS,  D3D12_RT_FORMAT_ARRAY       > renderTargetFormats;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT,   DXGI_FORMAT                 > depthStencilFormat;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC,            DXGI_SAMPLE_DESC            > sampleDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS,                  D3D12_PIPELINE_STATE_FLAGS  > flags;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO,             D3D12_CACHED_PIPELINE_STATE > cachedPSO;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING,        D3D12_VIEW_INSTANCING_DESC  > viewInstancing;
};

ComPtr<ID3D12PipelineState> D3D12GraphicsPSO::CreateNativePSOAsViewInstanced(
    ID3D12Device*                               device,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC&   desc,
    UINT                                        numViews,
    const char*                                 debugName)
{
    /* View instancing requires the stream-based pipeline state API on ID3D12Device2 */
    ComPtr<ID3D12Device2> device2;
    HRESULT hr = device->QueryInterface(IID_PPV_ARGS(device2.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to query ID3D12Device2 for view-instanced D3D12 pipeline state [%s] (HRESULT = %s)\n", GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
        return nullptr;
    }

    /* Route view instance i to render-target array layer i (viewport array index stays 0) */
    LLGL_ASSERT(numViews <= D3D12_MAX_VIEW_INSTANCE_COUNT, "number of views (%u) exceeds D3D12_MAX_VIEW_INSTANCE_COUNT (%u)", numViews, static_cast<UINT>(D3D12_MAX_VIEW_INSTANCE_COUNT));
    D3D12_VIEW_INSTANCE_LOCATION viewInstanceLocations[D3D12_MAX_VIEW_INSTANCE_COUNT] = {};
    for (UINT i = 0; i < numViews; ++i)
    {
        viewInstanceLocations[i].ViewportArrayIndex     = 0;
        viewInstanceLocations[i].RenderTargetArrayIndex = i;
    }

    /* Translate the equivalent graphics PSO descriptor into a pipeline state stream and append view instancing */
    D3DGraphicsPipelineStream stream;
    stream.rootSignature        = desc.pRootSignature;
    stream.vs                   = desc.VS;
    stream.hs                   = desc.HS;
    stream.ds                   = desc.DS;
    stream.gs                   = desc.GS;
    stream.ps                   = desc.PS;
    stream.streamOutput         = desc.StreamOutput;
    stream.blendDesc            = desc.BlendState;
    stream.sampleMask           = desc.SampleMask;
    stream.rasterizerDesc       = desc.RasterizerState;
    stream.depthStencilDesc     = desc.DepthStencilState;
    stream.inputLayout          = desc.InputLayout;
    stream.ibStripCut           = desc.IBStripCutValue;
    stream.primitiveTopology    = desc.PrimitiveTopologyType;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = desc.NumRenderTargets;
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        rtvFormats.RTFormats[i] = desc.RTVFormats[i];
    stream.renderTargetFormats  = rtvFormats;

    stream.depthStencilFormat   = desc.DSVFormat;
    stream.sampleDesc           = desc.SampleDesc;
    stream.flags                = desc.Flags;
    stream.cachedPSO            = desc.CachedPSO;

    D3D12_VIEW_INSTANCING_DESC viewInstancingDesc = {};
    viewInstancingDesc.ViewInstanceCount        = numViews;
    viewInstancingDesc.pViewInstanceLocations   = viewInstanceLocations;
    viewInstancingDesc.Flags                    = D3D12_VIEW_INSTANCING_FLAG_NONE;
    stream.viewInstancing       = viewInstancingDesc;

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    {
        streamDesc.SizeInBytes                      = sizeof(stream);
        streamDesc.pPipelineStateSubobjectStream    = &stream;
    }
    ComPtr<ID3D12PipelineState> pipelineState;
    hr = device2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to create view-instanced D3D12 graphics pipeline state [%s] (HRESULT = %s)\n", GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
        return nullptr;
    }
    return pipelineState;
}

#endif // /LLGL_D3D12_ENABLE_DXCOMPILER


} // /namespace LLGL



// ================================================================================
