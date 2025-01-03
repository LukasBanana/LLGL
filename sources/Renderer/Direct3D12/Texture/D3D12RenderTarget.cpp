/*
 * D3D12RenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RenderTarget.h"
#include "D3D12Texture.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12Device.h"
#include "../D3D12Types.h"
#include "../Command/D3D12CommandContext.h"
#include "../RenderState/D3D12DescriptorHeap.h"
#include "../RenderState/D3D12RenderPass.h"
#include "../../DXCommon/DXTypes.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Exception.h"
#include "../D3DX12/d3dx12.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


D3D12RenderTarget::D3D12RenderTarget(D3D12Device& device, const RenderTargetDescriptor& desc) :
    resolution_ { desc.resolution }
{
    ColorFormatVector colorFormats;
    const UINT numColorFormats = GatherAttachmentFormats(device, desc, colorFormats);

    CreateDescriptorHeaps(device.GetNative(), numColorFormats);
    CreateAttachments(device.GetNative(), desc, colorFormats);
    defaultRenderPass_.BuildAttachments(numColorFormats, colorFormats.data(), depthStencilFormat_, sampleDesc_);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D12RenderTarget::SetDebugName(const char* name)
{
    D3D12SetObjectNameSubscript(rtvDescHeap_.Get(), name, ".RTV");
    D3D12SetObjectNameSubscript(dsvDescHeap_.Get(), name, ".DSV");
}

Extent2D D3D12RenderTarget::GetResolution() const
{
    return resolution_;
}

std::uint32_t D3D12RenderTarget::GetSamples() const
{
    return sampleDesc_.Count;
}

std::uint32_t D3D12RenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(colorBuffers_.size());
}

bool D3D12RenderTarget::HasDepthAttachment() const
{
    return (dsvDescHeap_.Get() != nullptr);
}

bool D3D12RenderTarget::HasStencilAttachment() const
{
    return (dsvDescHeap_.Get() != nullptr && DXTypes::HasStencilComponent(depthStencilFormat_));
}

const RenderPass* D3D12RenderTarget::GetRenderPass() const
{
    return (&defaultRenderPass_);
}

void D3D12RenderTarget::TransitionToOutputMerger(D3D12CommandContext& commandContext)
{
    for (auto& resource : colorBuffers_)
        commandContext.TransitionResource(*resource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    if (depthStencil_ != nullptr)
        commandContext.TransitionResource(*depthStencil_, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    commandContext.FlushResourceBarriers();
}

void D3D12RenderTarget::ResolveSubresources(D3D12CommandContext& commandContext)
{
    if (HasMultiSampling())
    {
        for (const ResolveTarget& target : resolveTargets_)
        {
            commandContext.ResolveSubresource(
                *target.resolveDstTexture,
                target.resolveDstSubresource,
                *target.multiSampledSrcTexture,
                0,
                target.format
            );
        }
    }
    else
    {
        for (D3D12Resource* resource : colorBuffers_)
            commandContext.TransitionResource(*resource, resource->usageState);
    }

    if (depthStencil_ != nullptr)
        commandContext.TransitionResource(*depthStencil_, depthStencil_->usageState);

    commandContext.FlushResourceBarriers();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderTarget::GetCPUDescriptorHandleForRTV() const
{
    if (rtvDescHeap_)
        return rtvDescHeap_->GetCPUDescriptorHandleForHeapStart();
    else
        return {};
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderTarget::GetCPUDescriptorHandleForDSV() const
{
    if (dsvDescHeap_)
        return dsvDescHeap_->GetCPUDescriptorHandleForHeapStart();
    else
        return {};
}


/*
 * ======= Private: =======
 */

// Counts the number of internal textures required for the specified render-target attachments.
static std::size_t NumInternalTexturesForAttachments(const RenderTargetDescriptor& desc)
{
    std::size_t n = 0;

    auto AttachmentNeedsInternalTexture = [](const AttachmentDescriptor& attachmentDesc) -> bool
    {
        return (attachmentDesc.texture == nullptr);
    };

    for (unsigned i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS && IsAttachmentEnabled(desc.colorAttachments[i]); ++i)
    {
        if (AttachmentNeedsInternalTexture(desc.colorAttachments[i]))
            ++n;
    }

    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        if (AttachmentNeedsInternalTexture(desc.depthStencilAttachment))
            ++n;
    }

    return n;
}

UINT D3D12RenderTarget::GatherAttachmentFormats(D3D12Device& device, const RenderTargetDescriptor& desc, ColorFormatVector& outColorFormats)
{
    /* Gather color formats */
    for (const auto& colorAttachment : desc.colorAttachments)
    {
        if (IsAttachmentEnabled(colorAttachment))
        {
            const Format      format     = GetAttachmentFormat(colorAttachment);
            const DXGI_FORMAT formatDXGI = DXTypes::ToDXGIFormat(format);
            outColorFormats.push_back(formatDXGI);
        }
        else
            break;
    }

    /* Determine and store suitable sample descriptor */
    if (desc.samples > 1)
        sampleDesc_ = device.FindSuitableSampleDesc(outColorFormats.size(), outColorFormats.data(), desc.samples);

    /* Store depth-stencil format */
    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        /* Adopt attachment format from descriptor */
        const Format      format     = GetAttachmentFormat(desc.depthStencilAttachment);
        const DXGI_FORMAT formatDXGI = DXTypes::ToDXGIFormat(format);
        depthStencilFormat_ = DXTypes::ToDXGIFormatDSV(formatDXGI);
    }

    /* Pre-allocate containers to avoid dangling pointers after std::vector::push_back() */
    colorBuffers_.reserve(outColorFormats.size());
    internalTextures_.reserve(NumInternalTexturesForAttachments(desc));

    return static_cast<UINT>(outColorFormats.size());
}

void D3D12RenderTarget::CreateDescriptorHeaps(ID3D12Device* device, const UINT numColorTargets)
{
    /* Create RTV descriptor heap */
    if (numColorTargets > 0)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        {
            heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = numColorTargets;
            heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask       = 0;
        }
        rtvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(device, heapDesc);
    }

    /* Create DSV descriptor heap */
    if (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        {
            heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            heapDesc.NumDescriptors = 1;
            heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask       = 0;
        }
        dsvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(device, heapDesc);
    }
}

static const D3D12RenderPass* GetD3DRenderPass(const RenderPass* renderPass)
{
    return (renderPass != nullptr ? LLGL_CAST(const D3D12RenderPass*, renderPass) : nullptr);
}

void D3D12RenderTarget::CreateAttachments(
    ID3D12Device*                   device,
    const RenderTargetDescriptor&   desc,
    const ColorFormatVector&        colorFormats)
{
    if (rtvDescHeap_)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = rtvDescHeap_->GetCPUDescriptorHandleForHeapStart();
        const UINT rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for_range(i, colorFormats.size())
        {
            CreateColorAttachment(device, desc.colorAttachments[i], desc.resolveAttachments[i], colorFormats[i], cpuDescHandle);
            cpuDescHandle.ptr += rtvDescSize;
        }
    }
    if (dsvDescHeap_)
    {
        auto* renderPassD3D = GetD3DRenderPass(desc.renderPass);
        const D3D12_DSV_FLAGS dsvFlags = (renderPassD3D != nullptr ? renderPassD3D->GetAttachmentFlagsDSV() : D3D12_DSV_FLAG_NONE);
        CreateDepthStencilAttachment(device, desc.depthStencilAttachment, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart(), dsvFlags);
    }
}

void D3D12RenderTarget::CreateColorAttachment(
    ID3D12Device*                   device,
    const AttachmentDescriptor&     colorAttachment,
    const AttachmentDescriptor&     resolveAttachment,
    DXGI_FORMAT                     format,
    D3D12_CPU_DESCRIPTOR_HANDLE     cpuDescHandle)
{
    D3D12Resource* colorBuffer = nullptr;

    /* Create color attachment */
    if (Texture* texture = colorAttachment.texture)
    {
        ValidateMipResolution(*texture, colorAttachment.mipLevel);
        auto& textureD3D = LLGL_CAST(D3D12Texture&, *texture);
        colorBuffer = &(textureD3D.GetResource());
        CreateRenderTargetView(
            device,
            *colorBuffer,
            format,
            textureD3D.GetType(),
            colorAttachment.mipLevel,
            colorAttachment.arrayLayer,
            cpuDescHandle
        );
    }
    else
    {
        colorBuffer = CreateInternalTexture(
            device,
            format,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );
        device->CreateRenderTargetView(colorBuffer->Get(), nullptr, cpuDescHandle);
    }

    /* Create resolve target entry if multi-sampling is enabled */
    if (HasMultiSampling() && resolveAttachment.texture != nullptr)
        CreateResolveTarget(resolveAttachment, format, colorBuffer);

    LLGL_ASSERT_PTR(colorBuffer);
    LLGL_ASSERT_PTR(colorBuffer->native.Get());
    colorBuffers_.push_back(colorBuffer);
}

void D3D12RenderTarget::CreateDepthStencilAttachment(
    ID3D12Device*                   device,
    const AttachmentDescriptor&     depthStenciAttachment,
    D3D12_CPU_DESCRIPTOR_HANDLE     cpuDescHandle,
    D3D12_DSV_FLAGS                 dsvFlags)
{
    /* Create depth-stencil attachment */
    if (Texture* texture = depthStenciAttachment.texture)
    {
        ValidateMipResolution(*texture, depthStenciAttachment.mipLevel);
        auto& textureD3D = LLGL_CAST(D3D12Texture&, *texture);
        depthStencil_ = &(textureD3D.GetResource());
        CreateDepthStencilView(
            device,
            *depthStencil_,
            depthStencilFormat_,
            textureD3D.GetType(),
            depthStenciAttachment.mipLevel,
            depthStenciAttachment.arrayLayer,
            dsvFlags
        );
    }
    else
    {
        const CD3DX12_CLEAR_VALUE clearValue{ depthStencilFormat_, 1.0f, 0 };
        depthStencil_ = CreateInternalTexture(
            device,
            depthStencilFormat_,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE),
            &clearValue
        );
        device->CreateDepthStencilView(depthStencil_->Get(), nullptr, cpuDescHandle);
    }
}

D3D12Resource* D3D12RenderTarget::CreateInternalTexture(
    ID3D12Device*               device,
    DXGI_FORMAT                 format,
    D3D12_RESOURCE_STATES       initialState,
    D3D12_RESOURCE_FLAGS        flags,
    const D3D12_CLEAR_VALUE*    clearValue)
{
    /* Create internal texture with sampling descriptor and single MIP-level */
    auto tex2DDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        format,
        resolution_.width,
        resolution_.height,
        1, // arraySize
        1, // mipLevels
        sampleDesc_.Count,
        sampleDesc_.Quality,
        flags
    );

    /* Create render target resource */
    D3D12Resource tex2D;
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &tex2DDesc,
        initialState,
        clearValue,
        IID_PPV_ARGS(tex2D.native.GetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for render-target color buffer");

    /* Add resource to list of internal textures (only for auto-release) */
    tex2D.SetInitialState(initialState);
    internalTextures_.push_back(std::move(tex2D));
    return &(internalTextures_.back());
}

void D3D12RenderTarget::CreateRenderTargetView(
    ID3D12Device*               device,
    D3D12Resource&              resource,
    DXGI_FORMAT                 format,
    const TextureType           type,
    UINT                        mipLevel,
    UINT                        arrayLayer,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    /* Initialize D3D12 RTV descriptor */
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXTypes::ToDXGIFormatRTV(format);

    switch (type)
    {
        case TextureType::Texture1D:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice                  = mipLevel;
            break;

        case TextureType::Texture2D:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice                  = mipLevel;
            rtvDesc.Texture2D.PlaneSlice                = 0;
            break;

        case TextureType::Texture3D:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice                  = mipLevel;
            rtvDesc.Texture3D.FirstWSlice               = arrayLayer;
            rtvDesc.Texture3D.WSize                     = 1;
            break;

        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice             = mipLevel;
            rtvDesc.Texture2DArray.FirstArraySlice      = arrayLayer;
            rtvDesc.Texture2DArray.ArraySize            = 1;
            rtvDesc.Texture2DArray.PlaneSlice           = 0;
            break;

        case TextureType::Texture1DArray:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture1DArray.MipSlice             = mipLevel;
            rtvDesc.Texture1DArray.FirstArraySlice      = arrayLayer;
            rtvDesc.Texture1DArray.ArraySize            = 1;
            break;

        case TextureType::Texture2DMS:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE2DMS;
            break;

        case TextureType::Texture2DMSArray:
            rtvDesc.ViewDimension                       = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
            rtvDesc.Texture2DMSArray.FirstArraySlice    = arrayLayer;
            rtvDesc.Texture2DMSArray.ArraySize          = 1;
            break;

    }

    /* Create RTV and store reference to resource */
    device->CreateRenderTargetView(resource.Get(), &rtvDesc, cpuDescHandle);
}

void D3D12RenderTarget::CreateDepthStencilView(
    ID3D12Device*       device,
    D3D12Resource&      resource,
    DXGI_FORMAT         format,
    const TextureType   type,
    UINT                mipLevel,
    UINT                arrayLayer,
    D3D12_DSV_FLAGS     dsvFlags)
{
    /* Initialize D3D12 RTV descriptor */
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format  = DXTypes::ToDXGIFormatDSV(format);
    dsvDesc.Flags   = dsvFlags;

    switch (type)
    {
        case TextureType::Texture1D:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice                  = mipLevel;
            break;

        case TextureType::Texture2D:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice                  = mipLevel;
            break;

        case TextureType::Texture3D:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DArray:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice             = mipLevel;
            dsvDesc.Texture2DArray.FirstArraySlice      = arrayLayer;
            dsvDesc.Texture2DArray.ArraySize            = 1;
            break;

        case TextureType::Texture1DArray:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
            dsvDesc.Texture1DArray.MipSlice             = mipLevel;
            dsvDesc.Texture1DArray.FirstArraySlice      = arrayLayer;
            dsvDesc.Texture1DArray.ArraySize            = mipLevel;
            break;

        case TextureType::Texture2DMS:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE2DMS;
            break;

        case TextureType::Texture2DMSArray:
            dsvDesc.ViewDimension                       = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
            dsvDesc.Texture2DMSArray.FirstArraySlice    = arrayLayer;
            dsvDesc.Texture2DMSArray.ArraySize          = 1;
            break;
    }

    /* Create DSV and store reference to resource */
    device->CreateDepthStencilView(resource.Get(), &dsvDesc, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RenderTarget::CreateResolveTarget(
    const AttachmentDescriptor& resolveAttachment,
    DXGI_FORMAT                 format,
    D3D12Resource*              multiSampledSrcTexture)
{
    LLGL_ASSERT_PTR(resolveAttachment.texture);

    ValidateMipResolution(*resolveAttachment.texture, resolveAttachment.mipLevel);
    auto& textureD3D = LLGL_CAST(D3D12Texture&, *resolveAttachment.texture);

    ResolveTarget resolveTarget;
    {
        resolveTarget.resolveDstTexture         = &(textureD3D.GetResource());
        resolveTarget.resolveDstSubresource     = textureD3D.CalcSubresource(resolveAttachment.mipLevel, resolveAttachment.arrayLayer);
        resolveTarget.multiSampledSrcTexture    = multiSampledSrcTexture;
        resolveTarget.format                    = format;
    }
    resolveTargets_.push_back(resolveTarget);
}


} // /namespace LLGL



// ================================================================================
