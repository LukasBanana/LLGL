/*
 * D3D12RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderTarget.h"
#include "D3D12Texture.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12Device.h"
#include "../D3D12Types.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXTypes.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../D3DX12/d3dx12.h"


namespace LLGL
{


D3D12RenderTarget::D3D12RenderTarget(D3D12Device& device, const RenderTargetDescriptor& desc) :
    resolution_ { desc.resolution }
{
    CreateDescriptorHeaps(device, desc);
    CreateAttachments(device.GetNative(), desc);
    defaultRenderPass_.BuildAttachments(
        static_cast<UINT>(desc.attachments.size()),
        desc.attachments.data(),
        depthStencilFormat_,
        sampleDesc_
    );
}

void D3D12RenderTarget::SetName(const char* name)
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
    return static_cast<std::uint32_t>(colorFormats_.size());
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

    commandContext.FlushResourceBarrieres();
}

//TODO: incomplete
void D3D12RenderTarget::ResolveRenderTarget(D3D12CommandContext& commandContext)
{
    if (HasMultiSampling())
    {
        for (std::size_t i = 0; i < colorBuffersMS_.size(); ++i)
        {
            commandContext.ResolveRenderTarget(
                *colorBuffers_[i],
                colorBuffersMS_[i].dstSubresource,
                colorBuffersMS_[i].resource,
                0,
                colorFormats_[i]
            );
        }
    }
    else
    {
        for (auto& resource : colorBuffers_)
            commandContext.TransitionResource(*resource, resource->usageState);
    }

    if (depthStencil_ != nullptr)
        commandContext.TransitionResource(*depthStencil_, depthStencil_->usageState);

    commandContext.FlushResourceBarrieres();
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

bool D3D12RenderTarget::HasMultiSampling() const
{
    return (sampleDesc_.Count > 1);
}


/*
 * ======= Private: =======
 */

void D3D12RenderTarget::CreateDescriptorHeaps(D3D12Device& device, const RenderTargetDescriptor& desc)
{
    /* Determine number of resource views */
    colorFormats_.reserve(desc.attachments.size());

    for (const auto& attachment : desc.attachments)
    {
        if (auto texture = attachment.texture)
        {
            /* Get texture format */
            auto& textureD3D = LLGL_CAST(D3D12Texture&, *texture);
            auto format = textureD3D.GetDXFormat();

            /* Store color or depth-stencil format */
            if (attachment.type == AttachmentType::Color)
                colorFormats_.push_back(format);
            else
                depthStencilFormat_ = format;
        }
        else
        {
            switch (attachment.type)
            {
                case AttachmentType::Color:
                    throw std::invalid_argument("cannot have color attachment in render target without a valid texture");
                case AttachmentType::Depth:
                    depthStencilFormat_ = DXGI_FORMAT_D32_FLOAT;
                    break;
                case AttachmentType::DepthStencil:
                case AttachmentType::Stencil:
                    depthStencilFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
                    break;
            }
        }
    }

    /* Determine and store suitable sample descriptor */
    if (desc.samples > 1)
        sampleDesc_ = device.FindSuitableSampleDesc(colorFormats_.size(), colorFormats_.data(), desc.samples);

    /* Create RTV descriptor heap */
    if (!colorFormats_.empty())
    {
        colorBuffers_.reserve(colorFormats_.size());
        auto numRenderTargets = static_cast<UINT>(colorFormats_.size());

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        {
            heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = (HasMultiSampling() ? numRenderTargets * 2 : numRenderTargets);
            heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask       = 0;
        }
        rtvDescHeap_ = device.CreateDXDescriptorHeap(heapDesc);
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
        dsvDescHeap_ = device.CreateDXDescriptorHeap(heapDesc);
    }
}

void D3D12RenderTarget::CreateAttachments(ID3D12Device* device, const RenderTargetDescriptor& desc)
{
    /* Get CPU descriptor heap start for RTVs */
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle;
    if (rtvDescHeap_)
    {
        cpuDescHandle   = rtvDescHeap_->GetCPUDescriptorHandleForHeapStart();
        rtvDescSize_    = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    /* Create multi-sampled texture resources first */
    if (HasMultiSampling())
        CreateColorBuffersMS(device, desc, cpuDescHandle);

    /* Create all attachments */
    for (const auto& attachment : desc.attachments)
    {
        if (auto texture = attachment.texture)
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, *texture);
            CreateSubresource(
                device,
                attachment.type,
                textureD3D.GetResource(),
                textureD3D.GetDXFormat(),
                textureD3D.GetType(),
                attachment.mipLevel,
                attachment.arrayLayer,
                cpuDescHandle
            );
        }
        else
        {
            /* Create internal depth-stencil buffer and default DSV */
            CreateDepthStencil(device, depthStencilFormat_);
            device->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
            depthStencil_ = &depthStencilBuffer_;
        }
    }
}

void D3D12RenderTarget::CreateColorBuffersMS(ID3D12Device* device, const RenderTargetDescriptor& desc, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle)
{
    colorBuffersMS_.resize(colorFormats_.size());

    std::size_t idx = 0;

    for (const auto& attachment : desc.attachments)
    {
        if (attachment.type != AttachmentType::Color || attachment.texture == nullptr)
            continue;

        auto&   colorBufferMS   = colorBuffersMS_[idx];
        auto    format          = colorFormats_[idx];
        auto    textureD3D      = LLGL_CAST(const D3D12Texture*, attachment.texture);

        /* Create multi-sampled render targets */
        auto tex2DMSDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            format,
            resolution_.width,
            resolution_.height,
            1, // arraySize
            1, // mipLevels
            sampleDesc_.Count,
            sampleDesc_.Quality,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        /* Create render target resource */
        auto hr = device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &tex2DMSDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            nullptr,
            IID_PPV_ARGS(colorBufferMS.resource.native.ReleaseAndGetAddressOf())
        );
        DXThrowIfCreateFailed(hr, "ID3D12Resource", "for multi-sampled render-target");

        colorBufferMS.resource.SetInitialState(D3D12_RESOURCE_STATE_RENDER_TARGET);

        /* Store level and layer to resolve multi-sampled color buffer into destination texture */
        colorBufferMS.dstSubresource = textureD3D->CalcSubresource(attachment.mipLevel, attachment.arrayLayer);

        /* Create RTV subresource */
        device->CreateRenderTargetView(colorBufferMS.resource.native.Get(), nullptr, cpuDescHandle);
        cpuDescHandle.ptr += rtvDescSize_;

        ++idx;
    }
}

void D3D12RenderTarget::CreateDepthStencil(ID3D12Device* device, DXGI_FORMAT format)
{
    /* Create depth-stencil buffer */
    auto tex2DDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthStencilFormat_,
        resolution_.width,
        resolution_.height,
        1, // arraySize
        1, // mipLevels
        sampleDesc_.Count,
        sampleDesc_.Quality,
        (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
    );

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &tex2DDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &CD3DX12_CLEAR_VALUE(depthStencilFormat_, 1.0f, 0),
        IID_PPV_ARGS(depthStencilBuffer_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for render-target depth-stencil buffer");
}

void D3D12RenderTarget::CreateSubresource(
    ID3D12Device*                   device,
    const AttachmentType            attachmentType,
    D3D12Resource&                  resource,
    DXGI_FORMAT                     format,
    const TextureType               textureType,
    UINT                            mipLevel,
    UINT                            arrayLayer,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle)
{
    if (attachmentType == AttachmentType::Color)
    {
        CreateSubresourceRTV(device, resource, format, textureType, mipLevel, arrayLayer, cpuDescHandle);
        cpuDescHandle.ptr += rtvDescSize_;
    }
    else
        CreateSubresourceDSV(device, resource, format, textureType, mipLevel, arrayLayer);
}

void D3D12RenderTarget::CreateSubresourceRTV(
    ID3D12Device*                       device,
    D3D12Resource&                      resource,
    DXGI_FORMAT                         format,
    const TextureType                   type,
    UINT                                mipLevel,
    UINT                                arrayLayer,
    const D3D12_CPU_DESCRIPTOR_HANDLE&  cpuDescHandle)
{
    /* Initialize D3D12 RTV descriptor */
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = format;

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
    colorBuffers_.push_back(&resource);
}

void D3D12RenderTarget::CreateSubresourceDSV(
    ID3D12Device*       device,
    D3D12Resource&      resource,
    DXGI_FORMAT         format,
    const TextureType   type,
    UINT                mipLevel,
    UINT                arrayLayer)
{
    /* Initialize D3D12 RTV descriptor */
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format  = D3D12Types::ToDXGIFormatDSV(format);
    dsvDesc.Flags   = D3D12_DSV_FLAG_NONE;

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
    depthStencil_ = &resource;
}


} // /namespace LLGL



// ================================================================================
