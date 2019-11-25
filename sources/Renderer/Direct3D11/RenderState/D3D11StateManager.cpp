/*
 * D3D11StateManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11StateManager.h"
#include "../../../Core/HelperMacros.h"
#include <algorithm>
#include <cstddef>


namespace LLGL
{


static const UINT g_cbufferChunkSize = 4096u;

D3D11StateManager::D3D11StateManager(ID3D11Device* device, ComPtr<ID3D11DeviceContext>& context) :
    context_                 { context                                                               },
    intermediateCbufferPool_ { device, context.Get(), g_cbufferChunkSize, D3D11_BIND_CONSTANT_BUFFER }
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    context_->QueryInterface(IID_PPV_ARGS(&context1_));
    #endif
}

// Check if D3D11_VIEWPORT and Viewport structures can be safely reinterpret-casted
static constexpr bool IsCompatibleToD3DViewport()
{
    return
    (
        sizeof(D3D11_VIEWPORT)             == sizeof(Viewport)             &&
        offsetof(D3D11_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
        offsetof(D3D11_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
        offsetof(D3D11_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
        offsetof(D3D11_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
        offsetof(D3D11_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
        offsetof(D3D11_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth)
    );
}

void D3D11StateManager::SetViewports(std::uint32_t numViewports, const Viewport* viewportArray)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    /* Check if D3D11_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if (IsCompatibleToD3DViewport())
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        context_->RSSetViewports(numViewports, reinterpret_cast<const D3D11_VIEWPORT*>(viewportArray));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D11_VIEWPORT viewportsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewportArray[i];
            auto& dst       = viewportsD3D[i];

            dst.TopLeftX    = src.x;
            dst.TopLeftY    = src.y;
            dst.Width       = src.width;
            dst.Height      = src.height;
            dst.MinDepth    = src.minDepth;
            dst.MaxDepth    = src.maxDepth;
        }

        context_->RSSetViewports(numViewports, viewportsD3D);
    }
}

void D3D11StateManager::SetScissors(std::uint32_t numScissors, const Scissor* scissorArray)
{
    numScissors = std::min(numScissors, std::uint32_t(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    D3D11_RECT scissorsD3D[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    for (std::uint32_t i = 0; i < numScissors; ++i)
    {
        const auto& src = scissorArray[i];
        auto& dst       = scissorsD3D[i];

        dst.left        = src.x;
        dst.top         = src.y;
        dst.right       = src.x + src.width;
        dst.bottom      = src.y + src.height;
    }

    context_->RSSetScissorRects(numScissors, scissorsD3D);
}

void D3D11StateManager::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    if (inputAssemblyState_.primitiveTopology != primitiveTopology)
    {
        inputAssemblyState_.primitiveTopology = primitiveTopology;
        context_->IASetPrimitiveTopology(primitiveTopology);
    }
}

void D3D11StateManager::SetInputLayout(ID3D11InputLayout* inputLayout)
{
    if (inputAssemblyState_.inputLayout != inputLayout)
    {
        inputAssemblyState_.inputLayout = inputLayout;
        context_->IASetInputLayout(inputLayout);
    }
}

void D3D11StateManager::SetVertexShader(ID3D11VertexShader* shader)
{
    if (shaderState_.vs != shader)
    {
        shaderState_.vs = shader;
        context_->VSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetHullShader(ID3D11HullShader* shader)
{
    if (shaderState_.hs != shader)
    {
        shaderState_.hs = shader;
        context_->HSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetDomainShader(ID3D11DomainShader* shader)
{
    if (shaderState_.ds != shader)
    {
        shaderState_.ds = shader;
        context_->DSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetGeometryShader(ID3D11GeometryShader* shader)
{
    if (shaderState_.gs != shader)
    {
        shaderState_.gs = shader;
        context_->GSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetPixelShader(ID3D11PixelShader* shader)
{
    if (shaderState_.ps != shader)
    {
        shaderState_.ps = shader;
        context_->PSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetComputeShader(ID3D11ComputeShader* shader)
{
    if (shaderState_.cs != shader)
    {
        shaderState_.cs = shader;
        context_->CSSetShader(shader, nullptr, 0);
    }
}

void D3D11StateManager::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
    if (renderState_.rasterizerState != rasterizerState)
    {
        renderState_.rasterizerState = rasterizerState;
        context_->RSSetState(rasterizerState);
    }
}

void D3D11StateManager::SetDepthStencilState(ID3D11DepthStencilState* depthStencilState)
{
    if (renderState_.depthStencilState != depthStencilState)
    {
        renderState_.depthStencilState = depthStencilState;
        context_->OMSetDepthStencilState(depthStencilState, renderState_.stencilRef);
    }
}

void D3D11StateManager::SetDepthStencilState(ID3D11DepthStencilState* depthStencilState, UINT stencilRef)
{
    if (renderState_.depthStencilState != depthStencilState || renderState_.stencilRef != stencilRef)
    {
        renderState_.depthStencilState  = depthStencilState;
        renderState_.stencilRef         = stencilRef;
        context_->OMSetDepthStencilState(depthStencilState, stencilRef);
    }
}

void D3D11StateManager::SetStencilRef(UINT stencilRef)
{
    if (renderState_.stencilRef != stencilRef)
    {
        renderState_.stencilRef = stencilRef;
        context_->OMSetDepthStencilState(renderState_.depthStencilState, stencilRef);
    }
}

static bool EqualsBlendFactors(const FLOAT* lhs, const FLOAT* rhs)
{
    return
    (
        lhs[0] == rhs[0] &&
        lhs[1] == rhs[1] &&
        lhs[2] == rhs[2] &&
        lhs[3] == rhs[3]
    );
}

void D3D11StateManager::SetBlendState(ID3D11BlendState* blendState, UINT sampleMask)
{
    if (renderState_.blendState != blendState || renderState_.sampleMask != sampleMask)
    {
        renderState_.blendState     = blendState;
        renderState_.sampleMask     = sampleMask;
        context_->OMSetBlendState(blendState, renderState_.blendFactor, sampleMask);
    }
}

void D3D11StateManager::SetBlendState(ID3D11BlendState* blendState, const FLOAT* blendFactor, UINT sampleMask)
{
    if (renderState_.blendState != blendState || !EqualsBlendFactors(renderState_.blendFactor, blendFactor) || renderState_.sampleMask != sampleMask)
    {
        renderState_.blendState     = blendState;
        renderState_.blendFactor[0] = blendFactor[0];
        renderState_.blendFactor[1] = blendFactor[1];
        renderState_.blendFactor[2] = blendFactor[2];
        renderState_.blendFactor[3] = blendFactor[3];
        renderState_.sampleMask     = sampleMask;
        context_->OMSetBlendState(blendState, blendFactor, sampleMask);
    }
}

void D3D11StateManager::SetBlendFactor(const FLOAT* blendFactor)
{
    if (!EqualsBlendFactors(renderState_.blendFactor, blendFactor))
    {
        renderState_.blendFactor[0] = blendFactor[0];
        renderState_.blendFactor[1] = blendFactor[1];
        renderState_.blendFactor[2] = blendFactor[2];
        renderState_.blendFactor[3] = blendFactor[3];
        context_->OMSetBlendState(renderState_.blendState, blendFactor, renderState_.sampleMask);
    }
}

void D3D11StateManager::SetConstantBuffers(
    UINT                    startSlot,
    UINT                    count,
    ID3D11Buffer* const*    buffers,
    long                    stageFlags)
{
    if (LLGL_VS_STAGE(stageFlags)) { context_->VSSetConstantBuffers(startSlot, count, buffers); }
    if (LLGL_HS_STAGE(stageFlags)) { context_->HSSetConstantBuffers(startSlot, count, buffers); }
    if (LLGL_DS_STAGE(stageFlags)) { context_->DSSetConstantBuffers(startSlot, count, buffers); }
    if (LLGL_GS_STAGE(stageFlags)) { context_->GSSetConstantBuffers(startSlot, count, buffers); }
    if (LLGL_PS_STAGE(stageFlags)) { context_->PSSetConstantBuffers(startSlot, count, buffers); }
    if (LLGL_CS_STAGE(stageFlags)) { context_->CSSetConstantBuffers(startSlot, count, buffers); }
}

void D3D11StateManager::SetConstantBuffersRange(
    UINT                    startSlot,
    UINT                    count,
    ID3D11Buffer* const*    buffers,
    const UINT*             firstConstants,
    const UINT*             numConstants,
    long                    stageFlags)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (context1_ != nullptr)
    {
        /* Bind buffer range to shader stage */
        if (LLGL_VS_STAGE(stageFlags)) { context1_->VSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
        if (LLGL_HS_STAGE(stageFlags)) { context1_->HSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
        if (LLGL_DS_STAGE(stageFlags)) { context1_->DSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
        if (LLGL_GS_STAGE(stageFlags)) { context1_->GSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
        if (LLGL_PS_STAGE(stageFlags)) { context1_->PSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
        if (LLGL_CS_STAGE(stageFlags)) { context1_->CSSetConstantBuffers1(startSlot, count, buffers, firstConstants, numConstants); }
    }
    else
    #endif
    {
        /* Buffer range is not supported for D3D 11.0 */
        for (UINT i = 0; i < count; ++i)
        {
            if (firstConstants[i] > 0)
                throw std::runtime_error("constant buffer range is only supported with Direct3D 11.1 or later");
        }

        /* Bind buffer to shader stage */
        if (LLGL_VS_STAGE(stageFlags)) { context_->VSSetConstantBuffers(startSlot, count, buffers); }
        if (LLGL_HS_STAGE(stageFlags)) { context_->HSSetConstantBuffers(startSlot, count, buffers); }
        if (LLGL_DS_STAGE(stageFlags)) { context_->DSSetConstantBuffers(startSlot, count, buffers); }
        if (LLGL_GS_STAGE(stageFlags)) { context_->GSSetConstantBuffers(startSlot, count, buffers); }
        if (LLGL_PS_STAGE(stageFlags)) { context_->PSSetConstantBuffers(startSlot, count, buffers); }
        if (LLGL_CS_STAGE(stageFlags)) { context_->CSSetConstantBuffers(startSlot, count, buffers); }
    }
}

void D3D11StateManager::SetShaderResources(
    UINT                                startSlot,
    UINT                                count,
    ID3D11ShaderResourceView* const*    views,
    long                                stageFlags)
{
    if (LLGL_VS_STAGE(stageFlags)) { context_->VSSetShaderResources(startSlot, count, views); }
    if (LLGL_HS_STAGE(stageFlags)) { context_->HSSetShaderResources(startSlot, count, views); }
    if (LLGL_DS_STAGE(stageFlags)) { context_->DSSetShaderResources(startSlot, count, views); }
    if (LLGL_GS_STAGE(stageFlags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (LLGL_PS_STAGE(stageFlags)) { context_->PSSetShaderResources(startSlot, count, views); }
    if (LLGL_CS_STAGE(stageFlags)) { context_->CSSetShaderResources(startSlot, count, views); }
}

void D3D11StateManager::SetSamplers(
    UINT                        startSlot,
    UINT                        count,
    ID3D11SamplerState* const*  samplers,
    long                        stageFlags)
{
    if (LLGL_VS_STAGE(stageFlags)) { context_->VSSetSamplers(startSlot, count, samplers); }
    if (LLGL_HS_STAGE(stageFlags)) { context_->HSSetSamplers(startSlot, count, samplers); }
    if (LLGL_DS_STAGE(stageFlags)) { context_->DSSetSamplers(startSlot, count, samplers); }
    if (LLGL_GS_STAGE(stageFlags)) { context_->GSSetSamplers(startSlot, count, samplers); }
    if (LLGL_PS_STAGE(stageFlags)) { context_->PSSetSamplers(startSlot, count, samplers); }
    if (LLGL_CS_STAGE(stageFlags)) { context_->CSSetSamplers(startSlot, count, samplers); }
}

void D3D11StateManager::SetUnorderedAccessViews(
    UINT                                startSlot,
    UINT                                count,
    ID3D11UnorderedAccessView* const*   views,
    const UINT*                         initialCounts,
    long                                stageFlags)
{
    if (LLGL_PS_STAGE(stageFlags))
    {
        /* Set UAVs for pixel shader stage */
        context_->OMSetRenderTargetsAndUnorderedAccessViews(
            /*NumRTVs:*/                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
            /*ppRenderTargetViews:*/    nullptr,
            /*pDepthStencilView:*/      nullptr,
            /*UAVStartSlot:*/           startSlot,
            /*NumUAVs:*/                count,
            /*ppUnorderedAccessViews:*/ views,
            /*pUAVInitialCounts:*/      initialCounts
        );
    }

    if (LLGL_CS_STAGE(stageFlags))
    {
        /* Set UAVs for compute shader stage */
        context_->CSSetUnorderedAccessViews(startSlot, count, views, initialCounts);
    }
}

void D3D11StateManager::SetConstants(std::uint32_t slot, const void* data, std::uint16_t dataSize, long stageFlags)
{
    /* Write data to intermediate constant buffer */
    auto bufferRange = intermediateCbufferPool_.Write(data, dataSize);

    /* Bind intermediate buffer to buffer range */
    const UINT firstConstants[] = { bufferRange.offset / 16 };
    const UINT numConstants[]   = { bufferRange.size / 16 };

    SetConstantBuffersRange(slot, 1, &(bufferRange.native), firstConstants, numConstants, stageFlags);
}

void D3D11StateManager::DispatchBuiltin(const D3D11BuiltinShader builtinShader, UINT numWorkGroupsX, UINT numWorkGroupsY, UINT numWorkGroupsZ)
{
    ID3D11ComputeShader* cs = D3D11BuiltinShaderFactory::Get().GetBulitinShader(builtinShader).cs.Get();
    SetComputeShader(cs);
    context_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D11StateManager::ResetIntermediateBufferPools()
{
    intermediateCbufferPool_.Reset();
}


} // /namespace LLGL



// ================================================================================
