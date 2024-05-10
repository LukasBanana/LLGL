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

    /* Allocate buffer for native handles */
    renderTargetHandles_.Allocate(NumActiveColorAttachments(desc), IsAttachmentEnabled(desc.depthStencilAttachment));

    /* Create native render-target (RTV) and depth-stencil views (DSV) */
    D3D11BindingLocator* bindingLocator = nullptr;
    D3D11SubresourceRange subresourceRange = {};

    for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        if (IsAttachmentEnabled(desc.colorAttachments[i]))
        {
            ComPtr<ID3D11RenderTargetView> rtv = CreateRenderTargetView(
                device,
                desc.colorAttachments[i],
                desc.resolveAttachments[i],
                bindingLocator,
                subresourceRange
            );
            renderTargetHandles_.SetRenderTargetView(i, rtv.Get(), bindingLocator, subresourceRange);
        }
        else
            break;
    }

    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        const UINT dsvFlags = (renderPass_ != nullptr ? renderPass_->GetAttachmentFlagsDSV() : 0);
        ComPtr<ID3D11DepthStencilView> dsv = CreateDepthStencilView(
            device,
            desc.depthStencilAttachment,
            dsvFlags,
            bindingLocator
        );
        renderTargetHandles_.SetDepthStencilView(dsv.Get(), bindingLocator);
    }

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D11RenderTarget::SetDebugName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each RTV */
        for_range(i, renderTargetHandles_.GetNumRenderTargetViews())
        {
            const std::string subscript = ".RTV[" + std::to_string(i) + "]";
            D3D11SetObjectNameSubscript(renderTargetHandles_.GetRenderTargetViews()[i], name, subscript.c_str());
        }

        /* Set label for DSV */
        if (ID3D11DepthStencilView* dsv = renderTargetHandles_.GetDepthStencilView())
            D3D11SetObjectNameSubscript(dsv, name, ".DSV");

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
        for (auto it = renderTargetHandles_.GetRenderTargetViews(), itEnd = it + renderTargetHandles_.GetNumRenderTargetViews(); it != itEnd; ++it)
            D3D11SetObjectName(*it, nullptr);
        D3D11SetObjectName(renderTargetHandles_.GetDepthStencilView(), nullptr);
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
    return static_cast<std::uint32_t>(renderTargetHandles_.GetNumRenderTargetViews());
}

bool D3D11RenderTarget::HasDepthAttachment() const
{
    return renderTargetHandles_.HasDepthStencilView();
}

bool D3D11RenderTarget::HasStencilAttachment() const
{
    return (renderTargetHandles_.HasDepthStencilView() && DXTypes::HasStencilComponent(depthStencilFormat_));
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

    for (const auto& attachment : desc.colorAttachments)
    {
        if (IsAttachmentEnabled(attachment))
        {
            const Format format = GetAttachmentFormat(attachment);
            formats.push_back(DXTypes::ToDXGIFormatRTV(DXTypes::ToDXGIFormat(format)));
        }
    }

    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        const Format format = GetAttachmentFormat(desc.depthStencilAttachment);
        formats.push_back(DXTypes::ToDXGIFormatDSV(DXTypes::ToDXGIFormat(format)));
    }

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

LLGL_NODISCARD
ComPtr<ID3D11RenderTargetView> D3D11RenderTarget::CreateRenderTargetView(
    ID3D11Device*               device,
    const AttachmentDescriptor& colorAttachment,
    const AttachmentDescriptor& resolveAttachment,
    D3D11BindingLocator*&       outBindingLocator,
    D3D11SubresourceRange&      outSubresourceRange)
{
    ComPtr<ID3D11RenderTargetView> rtv;
    DXGI_FORMAT colorFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11Resource* colorTarget = nullptr;

    if (Texture* texture = colorAttachment.texture)
    {
        /* Get native D3D11 texture from color attachment */
        ValidateMipResolution(*texture, colorAttachment.mipLevel);
        auto* textureD3D = LLGL_CAST(D3D11Texture*, texture);
        colorFormat = textureD3D->GetBaseDXFormat();
        colorTarget = textureD3D->GetNative();

        /* Create RTV for color attachment */
        D3D11RenderTarget::CreateSubresourceRTV(
            /*device:*/         device,
            /*resource:*/       colorTarget,
            /*rtvOutput:*/      rtv.GetAddressOf(),
            /*type:*/           textureD3D->GetType(),
            /*format:*/         colorFormat,
            /*baseMipLevel:*/   colorAttachment.mipLevel,
            /*baseArrayLayer:*/ colorAttachment.arrayLayer
        );

        /* Return locator and subresource range for texture */
        const UINT rtvSubresource = textureD3D->CalcSubresource(colorAttachment.mipLevel, colorAttachment.arrayLayer);
        outBindingLocator   = textureD3D->GetBindingLocator();
        outSubresourceRange = { rtvSubresource, rtvSubresource + 1 };
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
            /*rtvOutput:*/      rtv.GetAddressOf(),
            /*type:*/           (HasMultiSampling() ? TextureType::Texture2DMS : TextureType::Texture2D),
            /*format:*/         colorFormat,
            /*baseMipLevel:*/   0,
            /*baseArrayLayer:*/ 0
        );

        /* Reset locator and subresource range */
        outBindingLocator   = nullptr;
        outSubresourceRange = {};
    }

    /* Create resolve target if a resolve texture is specified */
    if (resolveAttachment.texture != nullptr && HasMultiSampling())
        CreateResolveTarget(device, resolveAttachment, colorFormat, colorTarget);

    return rtv;
}

LLGL_NODISCARD
ComPtr<ID3D11DepthStencilView> D3D11RenderTarget::CreateDepthStencilView(
    ID3D11Device*               device,
    const AttachmentDescriptor& depthStencilAttachment,
    UINT                        dsvFlags,
    D3D11BindingLocator*&       outBindingLocator)
{
    ComPtr<ID3D11DepthStencilView> dsv;

    if (Texture* texture = depthStencilAttachment.texture)
    {
        /* Create DSV for target texture */
        ValidateMipResolution(*texture, depthStencilAttachment.mipLevel);
        auto* textureD3D = LLGL_CAST(D3D11Texture*, texture);
        depthStencilFormat_ = DXTypes::ToDXGIFormatDSV(textureD3D->GetBaseDXFormat());
        D3D11RenderTarget::CreateSubresourceDSV(
            /*device:*/         device,
            /*resource:*/       textureD3D->GetNative(),
            /*dsvOutput:*/      dsv.GetAddressOf(),
            /*type:*/           textureD3D->GetType(),
            /*format:*/         depthStencilFormat_,
            /*baseMipLevel:*/   depthStencilAttachment.mipLevel,
            /*baseArrayLayer:*/ depthStencilAttachment.arrayLayer,
            /*numArrayLayers:*/ 1u,
            /*dsvFlags:*/       dsvFlags
        );

        /* Return locator and subresource range for texture */
        outBindingLocator = textureD3D->GetBindingLocator();
    }
    else
    {
        /* Create internal texture with DSV for depth-stencil attachment */
        depthStencilFormat_ = DXTypes::ToDXGIFormatDSV(DXTypes::ToDXGIFormat(depthStencilAttachment.format));
        ID3D11Texture2D* depthStencil = CreateInternalTexture(device, depthStencilFormat_, D3D11_BIND_DEPTH_STENCIL);

        /* Create DSV for internal depth-stencil buffer */
        HRESULT hr = device->CreateDepthStencilView(depthStencil, nullptr, dsv.GetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11DepthStencilView", "for render-target depth-stencil");

        /* Reset locator and subresource range */
        outBindingLocator = nullptr;
    }

    return dsv;
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
        resolveTarget.resolveDstTexture         = textureD3D->GetNative();
        resolveTarget.resolveDstSubresource     = D3D11CalcSubresource(resolveAttachment.mipLevel, resolveAttachment.arrayLayer, textureD3D->GetNumMipLevels());
        resolveTarget.multiSampledSrcTexture    = multiSampledSrcTexture;
        resolveTarget.format                    = format;
    }
    resolveTargets_.push_back(resolveTarget);
}


} // /namespace LLGL



// ================================================================================
