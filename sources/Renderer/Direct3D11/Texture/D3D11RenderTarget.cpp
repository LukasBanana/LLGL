/*
 * D3D11RenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11RenderTarget.h"
#include "../D3D11RenderSystem.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../RenderState/D3D11RenderPass.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static const D3D11RenderPass* GetD3DRenderPass(const RenderPass* renderPass)
{
    return (renderPass != nullptr ? LLGL_CAST(const D3D11RenderPass*, renderPass) : nullptr);
}

D3D11RenderTarget::D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc) :
    resolution_ { desc.resolution                   },
    renderPass_ { GetD3DRenderPass(desc.renderPass) }
{
    if (desc.samples > 1)
        FindSuitableSampleDesc(device, desc);

    for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        if (IsAttachmentEnabled(desc.colorAttachments[i]))
            CreateRenderTargetView(device, desc.colorAttachments[i], desc.resolveAttachments[i]);
    }

    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        const UINT dsvFlags = (renderPass_ != nullptr ? renderPass_->GetAttachmentFlagsDSV() : 0);
        CreateDepthStencilView(device, desc.depthStencilAttachment, dsvFlags);
    }

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

D3D11RenderTarget::~D3D11RenderTarget()
{
    /* Release all render-target-views manually to avoid having a separate container with ComPtr */
    for (ID3D11RenderTargetView* rtv : renderTargetViews_)
    {
        LLGL_ASSERT_PTR(rtv);
        rtv->Release();
    }
}

void D3D11RenderTarget::SetDebugName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each RTV */
        for_range(i, renderTargetViews_.size())
        {
            const std::string subscript = ".RTV[" + std::to_string(i) + "]";
            D3D11SetObjectNameSubscript(renderTargetViews_[i], name, subscript.c_str());
        }

        /* Set label for DSV */
        if (depthStencilView_)
            D3D11SetObjectNameSubscript(depthStencilView_.Get(), name, ".DSV");

        /* Set lable for each internal texture */
        for_range(i, internalTextures_.size())
        {
            const std::string subscript = ".Tex2D[" + std::to_string(i) + "]";
            D3D11SetObjectNameSubscript(internalTextures_[i].Get(), name, subscript.c_str());
        }
    }
    else
    {
        /* Reset all labels */
        for (ID3D11RenderTargetView* rtv : GetRenderTargetViews())
            D3D11SetObjectName(rtv, nullptr);
        D3D11SetObjectName(depthStencilView_.Get(), nullptr);
        for (const auto& texD3D : internalTextures_)
            D3D11SetObjectName(texD3D.Get(), nullptr);
    }
}

Extent2D D3D11RenderTarget::GetResolution() const
{
    return resolution_;
}

std::uint32_t D3D11RenderTarget::GetSamples() const
{
    return sampleDesc_.Count;
}

std::uint32_t D3D11RenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(renderTargetViews_.size());
}

bool D3D11RenderTarget::HasDepthAttachment() const
{
    return (depthStencilView_.Get() != nullptr);
}

bool D3D11RenderTarget::HasStencilAttachment() const
{
    return (depthStencilView_.Get() != nullptr && DXTypes::HasStencilComponent(depthStencilFormat_));
}

const RenderPass* D3D11RenderTarget::GetRenderPass() const
{
    return renderPass_;
}

void D3D11RenderTarget::ResolveSubresources(ID3D11DeviceContext* context)
{
    for (const ResolveTarget& target : resolveTargets_)
    {
        context->ResolveSubresource(
            target.resolveDstTexture,
            target.resolveDstSubresource,
            target.multiSampledSrcTexture,
            0,
            target.format
        );
    }
}

void D3D11RenderTarget::CreateSubresourceDSV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11DepthStencilView**    dsvOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers,
    UINT                        dsvFlags)
{
    /* Create depth-stencil-view (DSV) for subresource */
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    {
        dsvDesc.Format  = DXTypes::ToDXGIFormatDSV(format);
        dsvDesc.Flags   = dsvFlags;

        switch (type)
        {
            case TextureType::Texture1D:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE1D;
                dsvDesc.Texture1D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture2D:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture1DArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                dsvDesc.Texture1DArray.MipSlice             = baseMipLevel;
                dsvDesc.Texture1DArray.FirstArraySlice      = baseArrayLayer;
                dsvDesc.Texture1DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture3D:
            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                dsvDesc.Texture2DArray.MipSlice             = baseMipLevel;
                dsvDesc.Texture2DArray.FirstArraySlice      = baseArrayLayer;
                dsvDesc.Texture2DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture2DMS:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                break;

            case TextureType::Texture2DMSArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                dsvDesc.Texture2DMSArray.FirstArraySlice    = baseArrayLayer;
                dsvDesc.Texture2DMSArray.ArraySize          = numArrayLayers;
                break;
        }
    }
    HRESULT hr = device->CreateDepthStencilView(resource, &dsvDesc, dsvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11DepthStencilView", "for texture subresource");
}

void D3D11RenderTarget::CreateSubresourceRTV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11RenderTargetView**    rtvOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers)
{
    /* Create depth-stencil-view (DSV) for subresource */
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    {
        rtvDesc.Format = DXTypes::ToDXGIFormatRTV(format);

        switch (type)
        {
            case TextureType::Texture1D:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture2D:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture1DArray:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                rtvDesc.Texture1DArray.MipSlice             = baseMipLevel;
                rtvDesc.Texture1DArray.FirstArraySlice      = baseArrayLayer;
                rtvDesc.Texture1DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice             = baseMipLevel;
                rtvDesc.Texture2DArray.FirstArraySlice      = baseArrayLayer;
                rtvDesc.Texture2DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture2DMS:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                break;

            case TextureType::Texture2DMSArray:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtvDesc.Texture2DMSArray.FirstArraySlice    = baseArrayLayer;
                rtvDesc.Texture2DMSArray.ArraySize          = numArrayLayers;
                break;

            case TextureType::Texture3D:
                rtvDesc.ViewDimension                       = D3D11_RTV_DIMENSION_TEXTURE3D;
                rtvDesc.Texture3D.MipSlice                  = baseMipLevel;
                rtvDesc.Texture3D.FirstWSlice               = baseArrayLayer;
                rtvDesc.Texture3D.WSize                     = numArrayLayers;
                break;
        }
    }
    HRESULT hr = device->CreateRenderTargetView(resource, &rtvDesc, rtvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11RenderTargetView", "for texture subresource");
}


/*
 * ======= Private: =======
 */

void D3D11RenderTarget::FindSuitableSampleDesc(ID3D11Device* device, const RenderTargetDescriptor& desc)
{
    /* Gather all attachment formats */
    SmallVector<DXGI_FORMAT, LLGL_MAX_NUM_ATTACHMENTS> formats;

    auto AppendAttachmentFormat = [&formats](const AttachmentDescriptor& attachment)
    {
        if (IsAttachmentEnabled(attachment))
        {
            const Format format = GetAttachmentFormat(attachment);
            formats.push_back(DXTypes::ToDXGIFormatRTV(DXTypes::ToDXGIFormat(format)));
        }
    };

    for (const auto& attachment : desc.colorAttachments)
        AppendAttachmentFormat(attachment);
    AppendAttachmentFormat(desc.depthStencilAttachment);

    /* Find least common denominator of suitable sample descriptor for all attachment formats */
    sampleDesc_ = D3D11RenderSystem::FindSuitableSampleDesc(device, formats.size(), formats.data(), desc.samples);
}

ID3D11Texture2D* D3D11RenderTarget::CreateInternalTexture(ID3D11Device* device, DXGI_FORMAT format, UINT bindFlags)
{
    /* Create internal 2D texture resource */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width           = resolution_.width;
        texDesc.Height          = resolution_.height;
        texDesc.MipLevels       = 1;
        texDesc.ArraySize       = 1;
        texDesc.Format          = format;
        texDesc.SampleDesc      = sampleDesc_;
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = bindFlags;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = 0;
    }
    ComPtr<ID3D11Texture2D> tex2D;
    HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, tex2D.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Texture2D", "for render-target internal texture");

    /* Append to container of internal textures and return reference to newly created texture */
    internalTextures_.push_back(std::move(tex2D));
    return internalTextures_.back().Get();
}

void D3D11RenderTarget::CreateRenderTargetView(
    ID3D11Device*               device,
    const AttachmentDescriptor& colorAttachment,
    const AttachmentDescriptor& resolveAttachment)
{
    DXGI_FORMAT             colorFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11Resource*         colorTarget = nullptr;
    ID3D11RenderTargetView* colorRTV    = nullptr;

    if (Texture* texture = colorAttachment.texture)
    {
        /* Get native D3D11 texture from color attachment */
        ValidateMipResolution(*texture, colorAttachment.mipLevel);
        auto* textureD3D = LLGL_CAST(D3D11Texture*, texture);
        colorFormat = textureD3D->GetBaseDXFormat();
        colorTarget = textureD3D->GetNative().resource.Get();

        /* Create RTV for color attachment */
        D3D11RenderTarget::CreateSubresourceRTV(
            /*device:*/         device,
            /*resource:*/       colorTarget,
            /*rtvOutput:*/      &colorRTV,
            /*type:*/           textureD3D->GetType(),
            /*format:*/         colorFormat,
            /*baseMipLevel:*/   colorAttachment.mipLevel,
            /*baseArrayLayer:*/ colorAttachment.arrayLayer
        );
    }
    else
    {
        /* Create internal texture for color attachment */
        colorFormat = DXTypes::ToDXGIFormat(colorAttachment.format);
        colorTarget = CreateInternalTexture(device, colorFormat, D3D11_BIND_RENDER_TARGET);

        /* Create RTV for color attachment */
        D3D11RenderTarget::CreateSubresourceRTV(
            /*device:*/         device,
            /*resource:*/       colorTarget,
            /*rtvOutput:*/      &colorRTV,
            /*type:*/           (HasMultiSampling() ? TextureType::Texture2DMS : TextureType::Texture2D),
            /*format:*/         colorFormat,
            /*baseMipLevel:*/   0,
            /*baseArrayLayer:*/ 0
        );
    }

    renderTargetViews_.push_back(colorRTV);

    /* Create resolve target if a resolve texture is specified */
    if (resolveAttachment.texture != nullptr && HasMultiSampling())
        CreateResolveTarget(device, resolveAttachment, colorFormat, colorTarget);
}

void D3D11RenderTarget::CreateDepthStencilView(
    ID3D11Device*               device,
    const AttachmentDescriptor& depthStencilAttachment,
    UINT                        dsvFlags)
{
    if (Texture* texture = depthStencilAttachment.texture)
    {
        /* Create DSV for target texture */
        ValidateMipResolution(*texture, depthStencilAttachment.mipLevel);
        auto* textureD3D = LLGL_CAST(D3D11Texture*, texture);
        depthStencilFormat_ = DXTypes::ToDXGIFormatDSV(textureD3D->GetBaseDXFormat());
        D3D11RenderTarget::CreateSubresourceDSV(
            /*device:*/         device,
            /*resource:*/       textureD3D->GetNative().resource.Get(),
            /*dsvOutput:*/      depthStencilView_.ReleaseAndGetAddressOf(),
            /*type:*/           textureD3D->GetType(),
            /*format:*/         depthStencilFormat_,
            /*baseMipLevel:*/   depthStencilAttachment.mipLevel,
            /*baseArrayLayer:*/ depthStencilAttachment.arrayLayer,
            /*numArrayLayers:*/ 1u,
            /*dsvFlags:*/       dsvFlags
        );
    }
    else
    {
        /* Create internal texture with DSV for depth-stencil attachment */
        depthStencilFormat_ = DXTypes::ToDXGIFormatDSV(DXTypes::ToDXGIFormat(depthStencilAttachment.format));
        ID3D11Texture2D* depthStencil = CreateInternalTexture(device, depthStencilFormat_, D3D11_BIND_DEPTH_STENCIL);

        /* Create DSV for internal depth-stencil buffer */
        HRESULT hr = device->CreateDepthStencilView(depthStencil, nullptr, depthStencilView_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11DepthStencilView", "for render-target depth-stencil");
    }
}

void D3D11RenderTarget::CreateResolveTarget(
    ID3D11Device*               device,
    const AttachmentDescriptor& resolveAttachment,
    DXGI_FORMAT                 format,
    ID3D11Resource*             multiSampledSrcTexture)
{
    LLGL_ASSERT_PTR(resolveAttachment.texture);

    ValidateMipResolution(*resolveAttachment.texture, resolveAttachment.mipLevel);
    auto* textureD3D = LLGL_CAST(D3D11Texture*, resolveAttachment.texture);

    ResolveTarget resolveTarget;
    {
        resolveTarget.resolveDstTexture         = textureD3D->GetNative().resource.Get();
        resolveTarget.resolveDstSubresource     = D3D11CalcSubresource(resolveAttachment.mipLevel, resolveAttachment.arrayLayer, textureD3D->GetNumMipLevels());
        resolveTarget.multiSampledSrcTexture    = multiSampledSrcTexture;
        resolveTarget.format                    = format;
    }
    resolveTargets_.push_back(resolveTarget);
}


} // /namespace LLGL



// ================================================================================
