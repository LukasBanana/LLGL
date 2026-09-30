/*
 * D3D12GraphicsPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12GraphicsPSO.h"
#include "D3D12PSOUtils.h"
#include "D3D12RenderPass.h"
#include "D3D12PipelineStateUtils.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12Shader.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Utils/ForRange.h>


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
        desc.rasterizer,
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
    commandContext.GetCommandList()->IASetPrimitiveTopology(primitiveTopology_);

    BindOutputMergerAndStaticStates(commandContext);
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

static void ReserveVertexAttribs(const GraphicsPipelineDescriptor& desc, LinearStringContainer& outVertexAttribNames)
{
    /* Reserve memory for the input element names */
    outVertexAttribNames.Clear();
    for (const VertexAttribute& attr : desc.inputVertexAttribs)
        outVertexAttribNames.Reserve(attr.name.size());
    for (const VertexAttribute& attr : desc.outputVertexAttribs)
        outVertexAttribNames.Reserve(attr.name.size());
}

/*
Converts a vertex attributes to a D3D12 input element descriptor
and stores the semantic name in the specified linear string container
*/
static void Convert(D3D12_INPUT_ELEMENT_DESC& dst, const VertexAttribute& src, LinearStringContainer& stringContainer)
{
    dst.SemanticName            = stringContainer.CopyString(src.name);
    dst.SemanticIndex           = src.semanticIndex;
    dst.Format                  = DXTypes::ToDXGIFormat(src.format);
    dst.InputSlot               = src.slot;
    dst.AlignedByteOffset       = src.offset;
    dst.InputSlotClass          = (src.instanceDivisor > 0 ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
    dst.InstanceDataStepRate    = src.instanceDivisor;
}

void D3D12GraphicsPSO::BuildInputLayout(
    LLGL::ArrayView<VertexAttribute>            inAttributes,
    DynamicVector<D3D12_INPUT_ELEMENT_DESC>&    outAttributes,
    LinearStringContainer&                      vertexAttribNames)
{
    const std::size_t numVertexAttribs = inAttributes.size();

    /* Build input element descriptors */
    outAttributes.resize(numVertexAttribs);
    for_range(i, numVertexAttribs)
        Convert(outAttributes[i], inAttributes[i], vertexAttribNames);
}

/*
Converts a vertex attributes to a D3D12 input element descriptor
and stores the semantic name in the specified linear string container
*/
static void ConvertSODeclEntry(D3D12_SO_DECLARATION_ENTRY& dst, const VertexAttribute& src, LinearStringContainer& stringContainer)
{
    const char* systemValueSemantic = DXTypes::SystemValueToString(src.systemValue);
    dst.Stream          = 0;
    dst.SemanticName    = (systemValueSemantic != nullptr ? systemValueSemantic : stringContainer.CopyString(src.name));
    dst.SemanticIndex   = src.semanticIndex;
    dst.StartComponent  = 0;
    dst.ComponentCount  = GetFormatAttribs(src.format).components;
    dst.OutputSlot      = src.slot;
}

void D3D12GraphicsPSO::BuildStreamOutput(
    LLGL::ArrayView<VertexAttribute>                    inAttributes,
    LLGL::DynamicVector<D3D12_SO_DECLARATION_ENTRY>&    outSODeclEntries,
    LLGL::DynamicVector<UINT>&                          outSOBufferStrides,
    LinearStringContainer&                              vertexAttribNames)
{
    if (inAttributes.empty())
       return;

    const std::size_t numStreamOutputAttribs = inAttributes.size();

    /* Reserve memory for the buffer strides */
    UINT maxSlot = 0;
    for_range(i, numStreamOutputAttribs)
        maxSlot = std::max<UINT>(maxSlot, inAttributes[i].slot);

    outSOBufferStrides.clear();
    outSOBufferStrides.resize(maxSlot + 1, 0);

    /* Build stream-output entries and buffer strides */
    outSODeclEntries.resize(numStreamOutputAttribs);
    for_range(i, numStreamOutputAttribs)
    {
        const VertexAttribute& attr = inAttributes[i];

        /* Convert vertex attribute to stream-output entry */
        ConvertSODeclEntry(outSODeclEntries[i], attr, vertexAttribNames);

        /* Store buffer stide */
        UINT& bufferStride = outSOBufferStrides[attr.slot];
        if (attr.stride == 0)
        {
            /* Error: vertex attribute must not have stride of zero */
            LLGL_TRAP(
                "buffer stride in stream-output attribute must not be zero: %s",
                attr.name.c_str()
            );
        }
        else if (bufferStride == 0)
        {
            /* Store new buffer stride */
            bufferStride = attr.stride;
        }
        else if (bufferStride != attr.stride)
        {
            LLGL_TRAP(
                "mismatch between buffer stride (%u) and stream-output attribute (%u): %s",
                bufferStride, attr.stride, attr.name.c_str()
            );
        }
    }

    /* Build buffer stride */
    for_range(i, outSOBufferStrides.size())
    {
        if (outSOBufferStrides[i] == 0)
            LLGL_TRAP("stream-output slot %zu is not specified in vertex attributes", i);
    }
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

    /* Convert input assembly state */
    DynamicVector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    DynamicVector<D3D12_SO_DECLARATION_ENTRY> soDeclEntries;
    DynamicVector<UINT> soBufferStrides;
    LinearStringContainer vertexAttribNames;

    ReserveVertexAttribs(desc, vertexAttribNames);
    BuildInputLayout(desc.inputVertexAttribs, inputElements, vertexAttribNames);
    BuildStreamOutput(desc.outputVertexAttribs, soDeclEntries, soBufferStrides, vertexAttribNames);

    /* Set input layout */
    if (!inputElements.empty())
    {
        stateDesc.InputLayout.pInputElementDescs = inputElements.data();
        stateDesc.InputLayout.NumElements        = static_cast<UINT>(inputElements.size());
    }
    else
    {
        // Deprecated feature support: Get input layout from the vertex shader if we failed to get it from the pipeline descriptor.
        LLGL_CAST(const D3D12Shader*, desc.vertexShader)->GetInputLayoutDesc(stateDesc.InputLayout);
    }

    /* Set stream output */
    if (!soDeclEntries.empty())
    {
        stateDesc.StreamOutput.pSODeclaration   = soDeclEntries.data();
        stateDesc.StreamOutput.NumEntries       = static_cast<UINT>(soDeclEntries.size());
        stateDesc.StreamOutput.pBufferStrides   = soBufferStrides.data();
        stateDesc.StreamOutput.NumStrides       = static_cast<UINT>(soBufferStrides.size());
        stateDesc.StreamOutput.RasterizedStream = 0;
    }
    else
    {
        // Deprecated feature support: Get stream output from the shaders if we failed to get it from the pipeline descriptor.
        stateDesc.StreamOutput = GetD3DStreamOutputDesc(desc.vertexShader, desc.tessEvaluationShader, desc.geometryShader);
    }

    /* Convert other states */
    const bool isStripTopology = IsPrimitiveTopologyStrip(desc.primitiveTopology);
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

    #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1
    /*
    When stream-based PSOs on ID3D12Device2 are available, always create them this way
    as mixing them with legacy PSOs can cause undefined behavior, especially in the WARP implementation of D3D12.
    This is primarily needed to support view instancing (aka multiview).
    */
    ComPtr<ID3D12Device2> device2;
    HRESULT hr = device->QueryInterface(IID_PPV_ARGS(device2.ReleaseAndGetAddressOf()));
    if (SUCCEEDED(hr))
    {
        const UINT numViews = (renderPass != nullptr ? renderPass->GetNumViews() : 1);

        /* Create primary PSO with 32-bit index cut off value */
        primaryPSO = CreateNativePSOWithStreamDesc(device2.Get(), stateDesc, numViews, desc.debugName);

        if (isStripTopology && desc.indexFormat == Format::Undefined)
        {
            /* Create secondary PSO with 16-bit index cut off value */
            stateDesc.IBStripCutValue   = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
            stateDesc.CachedPSO         = {};
            secondaryPSO_ = CreateNativePSOWithStreamDesc(device2.Get(), stateDesc, numViews, desc.debugName);
        }
    }
    else
    #endif // /LLGL_D3D12_ENABLE_FEATURELEVEL
    {
        /* Create primary PSO with 32-bit index cut off value */
        primaryPSO = CreateNativePSOWithDesc(device, stateDesc, desc.debugName);

        if (isStripTopology && desc.indexFormat == Format::Undefined)
        {
            /* Create secondary PSO with 16-bit index cut off value */
            stateDesc.IBStripCutValue   = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
            stateDesc.CachedPSO         = {};
            secondaryPSO_ = CreateNativePSOWithDesc(device, stateDesc, desc.debugName);
        }
    }

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

#if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1

/*
Stream layout for a view-instanced graphics PSO. Mirrors the fields of D3D12_GRAPHICS_PIPELINE_STATE_DESC and
appends the VIEW_INSTANCING subobject; the legacy descriptor is converted into this stream below.
*/
struct D3DGraphicsPipelineStream
{
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE,         ID3D12RootSignature*               > rootSignature;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS,                     D3D12_SHADER_BYTECODE              > vs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS,                     D3D12_SHADER_BYTECODE              > hs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS,                     D3D12_SHADER_BYTECODE              > ds;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS,                     D3D12_SHADER_BYTECODE              > gs;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS,                     D3D12_SHADER_BYTECODE              > ps;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT,          D3D12_STREAM_OUTPUT_DESC           > streamOutput;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND,                  D3D12_BLEND_DESC                   > blendDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK,            UINT                               > sampleMask;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER,             D3D12_RASTERIZER_DESC              > rasterizerDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL,          D3D12_DEPTH_STENCIL_DESC           > depthStencilDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT,           D3D12_INPUT_LAYOUT_DESC            > inputLayout;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE,     D3D12_INDEX_BUFFER_STRIP_CUT_VALUE > ibStripCut;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY,     D3D12_PRIMITIVE_TOPOLOGY_TYPE      > primitiveTopology;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS,  D3D12_RT_FORMAT_ARRAY              > renderTargetFormats;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT,   DXGI_FORMAT                        > depthStencilFormat;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC,            DXGI_SAMPLE_DESC                   > sampleDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK,              UINT                               > nodeMask; // Required in streamed-PSOs
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS,                  D3D12_PIPELINE_STATE_FLAGS         > flags;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO,             D3D12_CACHED_PIPELINE_STATE        > cachedPSO;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING,        D3D12_VIEW_INSTANCING_DESC         > viewInstancing;
};

ComPtr<ID3D12PipelineState> D3D12GraphicsPSO::CreateNativePSOWithStreamDesc(
    ID3D12Device2*                              device,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC&   desc,
    UINT                                        numViews,
    const char*                                 debugName)
{
    /* Route view instance i to render-target array layer i (viewport array index stays 0) */
    LLGL_ASSERT(numViews <= D3D12_MAX_VIEW_INSTANCE_COUNT, "number of views (%u) exceeds D3D12_MAX_VIEW_INSTANCE_COUNT (%u)", numViews, static_cast<UINT>(D3D12_MAX_VIEW_INSTANCE_COUNT));
    D3D12_VIEW_INSTANCE_LOCATION viewInstanceLocations[D3D12_MAX_VIEW_INSTANCE_COUNT] = {};
    for_range(i, numViews)
    {
        viewInstanceLocations[i].ViewportArrayIndex     = 0;
        viewInstanceLocations[i].RenderTargetArrayIndex = i;
    }

    /* Translate the equivalent graphics PSO descriptor into a pipeline state stream and append view instancing */
    D3DGraphicsPipelineStream stateDesc = {};

    stateDesc.nodeMask              = desc.NodeMask;
    stateDesc.rootSignature         = desc.pRootSignature;
    stateDesc.vs                    = desc.VS;
    stateDesc.hs                    = desc.HS;
    stateDesc.ds                    = desc.DS;
    stateDesc.gs                    = desc.GS;
    stateDesc.ps                    = desc.PS;
    stateDesc.streamOutput          = desc.StreamOutput;
    stateDesc.blendDesc             = desc.BlendState;
    stateDesc.sampleMask            = desc.SampleMask;
    stateDesc.rasterizerDesc        = desc.RasterizerState;
    stateDesc.depthStencilDesc      = desc.DepthStencilState;
    stateDesc.inputLayout           = desc.InputLayout;
    stateDesc.ibStripCut            = desc.IBStripCutValue;
    stateDesc.primitiveTopology     = desc.PrimitiveTopologyType;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    {
        rtvFormats.NumRenderTargets = desc.NumRenderTargets;
        for_range(i, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
            rtvFormats.RTFormats[i] = desc.RTVFormats[i];
    }
    stateDesc.renderTargetFormats = rtvFormats;

    stateDesc.depthStencilFormat    = desc.DSVFormat;
    stateDesc.sampleDesc            = desc.SampleDesc;
    stateDesc.flags                 = desc.Flags;
    stateDesc.cachedPSO             = desc.CachedPSO;

    D3D12_VIEW_INSTANCING_DESC viewInstancingDesc = {};
    {
        viewInstancingDesc.ViewInstanceCount        = numViews;
        viewInstancingDesc.pViewInstanceLocations   = viewInstanceLocations;
        viewInstancingDesc.Flags                    = D3D12_VIEW_INSTANCING_FLAG_NONE;
    }
    stateDesc.viewInstancing        = viewInstancingDesc;

    D3D12_PIPELINE_STATE_STREAM_DESC psoStreamDesc;
    {
        psoStreamDesc.SizeInBytes                   = sizeof(stateDesc);
        psoStreamDesc.pPipelineStateSubobjectStream = &stateDesc;
    }
    ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = device->CreatePipelineState(&psoStreamDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to create D3D12 graphics pipeline state [%s] with stream descriptor (HRESULT = %s)\n", GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
        return nullptr;
    }
    return pipelineState;
}

#endif // /LLGL_D3D12_ENABLE_FEATURELEVEL


} // /namespace LLGL



// ================================================================================
