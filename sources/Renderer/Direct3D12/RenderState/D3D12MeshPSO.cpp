/*
 * D3D12MeshPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1

#include "D3D12MeshPSO.h"
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


D3D12MeshPSO::D3D12MeshPSO(
    ID3D12Device2*                  device,
    D3D12PipelineLayout&            defaultPipelineLayout,
    const MeshPipelineDescriptor&   desc,
    const D3D12RenderPass*          defaultRenderPass,
    PipelineCache*                  pipelineCache)
:
    D3D12RenderPSOBase
    {
        D3D12PipelineType::Mesh,
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
    if (desc.meshShader == nullptr)
    {
        ResetReport("cannot create D3D mesh PSO without mesh shader", true);
        return;
    }

    /* Use either default render pass or from descriptor */
    const D3D12RenderPass* renderPassD3D = nullptr;
    if (desc.renderPass != nullptr)
        renderPassD3D = LLGL_CAST(const D3D12RenderPass*, desc.renderPass);
    else
        renderPassD3D = defaultRenderPass;

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

void D3D12MeshPSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetGraphicsRootSignature(GetRootSignature());
    commandContext.SetPipelineState(GetNative());

    /* Set dynamic pipeline states */
    BindOutputMergerAndStaticStates(commandContext.GetCommandList());
}

template <D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TType, typename TObject>
struct alignas(void*) D3DPipelineStreamSubobject
{
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type    = { TType };
    TObject                             object  = {};

    D3DPipelineStreamSubobject() = default;

    inline D3DPipelineStreamSubobject(TObject object) :
        object { std::move(object) }
    {
    }

    inline D3DPipelineStreamSubobject& operator = (TObject object)
    {
        this->object = std::move(object);
        return *this;
    }

    inline operator TObject& ()
    {
        return object;
    }

    inline TObject& operator * ()
    {
        return object;
    }

    inline TObject* operator -> ()
    {
        return &object;
    }
};

struct D3DMeshPipelineStream
{
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE,         ID3D12RootSignature*        > rootSignature;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS,                     D3D12_SHADER_BYTECODE       > amplificationShader;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS,                     D3D12_SHADER_BYTECODE       > meshShader;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS,                     D3D12_SHADER_BYTECODE       > pixelShader;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL,          D3D12_DEPTH_STENCIL_DESC    > depthStencilDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER,             D3D12_RASTERIZER_DESC       > rasterizerDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND,                  D3D12_BLEND_DESC            > blendDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT,   DXGI_FORMAT                 > depthStencilFormat;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS,  D3D12_RT_FORMAT_ARRAY       > renderTargetFormats;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK,            UINT                        > sampleMask;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC,            DXGI_SAMPLE_DESC            > sampleDesc;
    D3DPipelineStreamSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO,             D3D12_CACHED_PIPELINE_STATE > cachedPSO;
};

void D3D12MeshPSO::CreateNativePSO(
    ID3D12Device2*                  device,
    const D3D12PipelineLayout&      pipelineLayout,
    const D3D12RenderPass*          renderPass,
    const MeshPipelineDescriptor&   desc,
    D3D12PipelineCache*             pipelineCache)
{
    /* Get number of render-target attachments */
    const UINT numRenderTargets = (renderPass != nullptr ? renderPass->GetNumColorAttachments() : 1);

    /* Initialize D3D12 graphics pipeline descriptor */
    D3DMeshPipelineStream stateDesc = {};

    stateDesc.rootSignature = GetRootSignature();

    /* Get shader byte codes */
    stateDesc.amplificationShader   = GetD3DShaderByteCode(desc.taskShader);
    stateDesc.meshShader            = GetD3DShaderByteCode(desc.meshShader);
    stateDesc.pixelShader           = GetD3DShaderByteCode(desc.fragmentShader);

    /* Convert blend state and depth-stencil format */
    if (renderPass != nullptr)
    {
        stateDesc.depthStencilFormat = renderPass->GetDSVFormat();
        D3DConvertBlendDesc(stateDesc.blendDesc, stateDesc.renderTargetFormats->RTFormats, desc.blend, *renderPass);
    }
    else
    {
        stateDesc.depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        D3DConvertBlendDesc(stateDesc.blendDesc, stateDesc.renderTargetFormats->RTFormats, desc.blend, numRenderTargets);
    }
    stateDesc.renderTargetFormats->NumRenderTargets = numRenderTargets;

    /* Convert rasterizer state */
    D3DConvertRasterizerDesc(stateDesc.rasterizerDesc, desc.rasterizer);

    /* Convert depth-stencil state */
    D3DConvertDepthStencilDesc(stateDesc.depthStencilDesc, desc.depth, desc.stencil);

    /* Convert other states */
    stateDesc.sampleMask            = desc.blend.sampleMask;
    stateDesc.sampleDesc->Count     = (renderPass != nullptr ? renderPass->GetSampleDesc().Count : 1);
    stateDesc.sampleDesc->Quality   = 0;

    /* Set PSO cache if specified */
    if (pipelineCache != nullptr)
        stateDesc.cachedPSO = pipelineCache->GetCachedPSO();

    /* Create native PSO */
    D3D12_PIPELINE_STATE_STREAM_DESC psoStreamDesc;
    {
        psoStreamDesc.SizeInBytes                   = sizeof(stateDesc);
        psoStreamDesc.pPipelineStateSubobjectStream = &stateDesc;
    }
    ComPtr<ID3D12PipelineState> pso = CreateNativePSOWithDesc(device, psoStreamDesc, desc.debugName);

    SetNativeAndUpdateCache(std::move(pso), pipelineCache);
}

ComPtr<ID3D12PipelineState> D3D12MeshPSO::CreateNativePSOWithDesc(ID3D12Device2* device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc, const char* debugName)
{
    ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = device->CreatePipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to create D3D12 mesh pipeline state [%s] (HRESULT = %s)\n", GetOptionalDebugName(debugName), DXErrorToStrOrHex(hr));
        return nullptr;
    }
    return pipelineState;
}


} // /namespace LLGL

#endif // /LLGL_D3D12_ENABLE_FEATURELEVEL



// ================================================================================
