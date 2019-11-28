/*
 * D3D11RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderTarget.h"
#include "../D3D11RenderSystem.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11RenderTarget::D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc) :
    device_     { device          },
    resolution_ { desc.resolution },
    renderPass_ { desc.renderPass }
{
    if (desc.samples > 1)
        FindSuitableSampleDesc(desc);

    #if 0
    if (desc.attachments.empty())
    {
        //TODO...
    }
    else
    #endif
    {
        /* Initialize all attachments */
        for (const auto& attachment : desc.attachments)
            Attach(attachment);
    }
}

void D3D11RenderTarget::SetName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each RTV */
        std::uint32_t rtvIndex = 0;
        for (const auto& rtv : GetRenderTargetViews())
        {
            const std::string subscript = ".RTV[" + std::to_string(rtvIndex++) + "]";
            D3D11SetObjectNameSubscript(rtv, name, subscript.c_str());
        }

        /* Set labels for DS and DSV */
        if (depthStencil_)
            D3D11SetObjectNameSubscript(depthStencil_.Get(), name, ".DS");
        if (depthStencilView_)
            D3D11SetObjectNameSubscript(depthStencilView_.Get(), name, ".DSV");

        /* Set label for each multi-sampled texture */
        std::uint32_t attachmentIndex = 0;
        for (const auto& attachment : multiSampledAttachments_)
        {
            const std::string subscript = ".MS[" + std::to_string(attachmentIndex++) + "]";
            D3D11SetObjectNameSubscript(attachment.texture2DMS.Get(), name, subscript.c_str());
        }
    }
    else
    {
        /* Reset all labels */
        for (auto rtv : GetRenderTargetViews())
            D3D11SetObjectName(rtv, nullptr);

        D3D11SetObjectName(depthStencil_.Get(), nullptr);
        D3D11SetObjectName(depthStencilView_.Get(), nullptr);

        for (const auto& attachment : multiSampledAttachments_)
            D3D11SetObjectName(attachment.texture2DMS.Get(), nullptr);
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

/* ----- Extended Internal Functions ----- */

void D3D11RenderTarget::ResolveSubresources(ID3D11DeviceContext* context)
{
    for (const auto& attachment : multiSampledAttachments_)
    {
        context->ResolveSubresource(
            attachment.targetTexture,
            attachment.targetSubresourceIndex,
            attachment.texture2DMS.Get(),
            0,
            attachment.format
        );
    }
}


/*
 * ======= Private: =======
 */

void D3D11RenderTarget::FindSuitableSampleDesc(const RenderTargetDescriptor& desc)
{
    /* Gather all attachment formats */
    const auto numFormats = std::min(std::size_t(LLGL_MAX_NUM_ATTACHMENTS), desc.attachments.size());
    DXGI_FORMAT formats[LLGL_MAX_NUM_ATTACHMENTS];

    for (std::size_t i = 0; i < numFormats; ++i)
    {
        const auto& attachment = desc.attachments[i];
        if (auto texture = attachment.texture)
        {
            /* Get format from texture */
            auto textureD3D = LLGL_CAST(D3D11Texture*, texture);
            formats[i] = textureD3D->GetDXFormat();
        }
        else
        {
            /* Get format by type */
            switch (attachment.type)
            {
                case AttachmentType::Depth:
                    formats[i] = DXGI_FORMAT_D32_FLOAT;
                    break;
                case AttachmentType::DepthStencil:
                case AttachmentType::Stencil:
                    formats[i] = DXGI_FORMAT_D24_UNORM_S8_UINT;
                    break;
                default:
                    formats[i] = DXGI_FORMAT_UNKNOWN;
                    break;
            }
        }
    }

    /* Find least common denominator of suitable sample descriptor for all attachment formats */
    sampleDesc_ = D3D11RenderSystem::FindSuitableSampleDesc(device_, numFormats, formats, desc.samples);
}

void D3D11RenderTarget::Attach(const AttachmentDescriptor& attachmentDesc)
{
    if (auto texture = attachmentDesc.texture)
    {
        /* Attach texture */
        AttachTexture(*texture, attachmentDesc);
    }
    else
    {
        /* Attach (and create) depth-stencil buffer */
        switch (attachmentDesc.type)
        {
            case AttachmentType::Color:
                throw std::invalid_argument("cannot have color attachment in render target without a valid texture");
            case AttachmentType::Depth:
                AttachDepthBuffer();
                break;
            case AttachmentType::DepthStencil:
                AttachDepthStencilBuffer();
                break;
            case AttachmentType::Stencil:
                AttachStencilBuffer();
                break;
        }
    }
}

void D3D11RenderTarget::AttachDepthBuffer()
{
    CreateDepthStencilAndDSV(DXGI_FORMAT_D32_FLOAT);
}

void D3D11RenderTarget::AttachStencilBuffer()
{
    CreateDepthStencilAndDSV(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachDepthStencilBuffer()
{
    CreateDepthStencilAndDSV(DXGI_FORMAT_D24_UNORM_S8_UINT);
}

static void FillViewDescForTexture1D(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipSlice = attachmentDesc.mipLevel;
}

static void FillViewDescForTexture2D(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = attachmentDesc.mipLevel;
}

static void FillViewDescForTexture3D(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension          = D3D11_RTV_DIMENSION_TEXTURE3D;
    viewDesc.Texture3D.MipSlice     = attachmentDesc.mipLevel;
    viewDesc.Texture3D.FirstWSlice  = attachmentDesc.arrayLayer;
    viewDesc.Texture3D.WSize        = 1;
}

static void FillViewDescForTexture1DArray(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
    viewDesc.Texture1DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture1DArray.FirstArraySlice = attachmentDesc.arrayLayer;
    viewDesc.Texture1DArray.ArraySize       = 1;
}

static void FillViewDescForTexture2DArray(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture2DArray.FirstArraySlice = attachmentDesc.arrayLayer;
    viewDesc.Texture2DArray.ArraySize       = 1;
}

static void FillViewDescForTexture2DMS(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
}

static void FillViewDescForTexture2DArrayMS(const AttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                      = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    viewDesc.Texture2DMSArray.FirstArraySlice   = attachmentDesc.arrayLayer;
    viewDesc.Texture2DMSArray.ArraySize         = 1;
}

void D3D11RenderTarget::AttachTexture(Texture& texture, const AttachmentDescriptor& attachmentDesc)
{
    /* Get D3D texture object and apply resolution for MIP-map level */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    ValidateMipResolution(texture, attachmentDesc.mipLevel);

    if (attachmentDesc.type == AttachmentType::Color)
        AttachTextureColor(textureD3D, attachmentDesc);
    else
        AttachTextureDepthStencil(textureD3D, attachmentDesc);
}

void D3D11RenderTarget::AttachTextureColor(D3D11Texture& textureD3D, const AttachmentDescriptor& attachmentDesc)
{
    /* Initialize RTV descriptor with attachment procedure and create RTV */
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    InitMemory(rtvDesc);

    rtvDesc.Format = D3D11Types::ToDXGIFormatSRV(textureD3D.GetDXFormat());

    /*
    If this is a multi-sample render target, but the target texture is not a multi-sample texture,
    an intermediate multi-sample texture is required, which will be 'blitted' (or 'resolved') after the render target was rendered.
    */
    if (HasMultiSampling() && !IsMultiSampleTexture(textureD3D.GetType()))
    {
        /* Get RTV descriptor for intermediate multi-sample texture */
        switch (textureD3D.GetType())
        {
            case TextureType::Texture2D:
                FillViewDescForTexture2DMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::TextureCube:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
                FillViewDescForTexture2DArrayMS(attachmentDesc, rtvDesc);
                break;
            default:
                throw std::invalid_argument("failed to attach D3D11 texture to multi-sample render-target");
                break;
        }

        /* Recreate texture resource with multi-sampling */
        D3D11_TEXTURE2D_DESC texDesc;
        textureD3D.GetNative().tex2D->GetDesc(&texDesc);
        {
            texDesc.Width       = (texDesc.Width << attachmentDesc.mipLevel);
            texDesc.Height      = (texDesc.Height << attachmentDesc.mipLevel);
            texDesc.MipLevels   = 1;
            texDesc.SampleDesc  = sampleDesc_;
            texDesc.MiscFlags   = 0;
        }
        ComPtr<ID3D11Texture2D> tex2DMS;
        auto hr = device_->CreateTexture2D(&texDesc, nullptr, tex2DMS.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11Texture2D", "for multi-sampled render-target");

        /* Store multi-sampled texture, and reference to texture target */
        multiSampledAttachments_.push_back(
            {
                tex2DMS,
                textureD3D.GetNative().tex2D.Get(),
                D3D11CalcSubresource(attachmentDesc.mipLevel, attachmentDesc.arrayLayer, texDesc.MipLevels),
                texDesc.Format
            }
        );

        /* Create RTV for multi-sampled (intermediate) texture */
        CreateAndAppendRTV(tex2DMS.Get(), rtvDesc);
    }
    else
    {
        /* Get RTV descriptor for this the target texture */
        switch (textureD3D.GetType())
        {
            case TextureType::Texture1D:
                FillViewDescForTexture1D(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2D:
                FillViewDescForTexture2D(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture3D:
                FillViewDescForTexture3D(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture1DArray:
                FillViewDescForTexture1DArray(attachmentDesc, rtvDesc);
                break;
            case TextureType::TextureCube:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
                FillViewDescForTexture2DArray(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DMS:
                FillViewDescForTexture2DMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DMSArray:
                FillViewDescForTexture2DArrayMS(attachmentDesc, rtvDesc);
                break;
        }

        /* Create RTV for target texture */
        CreateAndAppendRTV(textureD3D.GetNative().resource.Get(), rtvDesc);
    }
}

void D3D11RenderTarget::AttachTextureDepthStencil(D3D11Texture& textureD3D, const AttachmentDescriptor& attachmentDesc)
{
    /* Create DSV for texture */
    textureD3D.CreateSubresourceDSV(
        device_,
        depthStencilView_.ReleaseAndGetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetDXFormat(),
        attachmentDesc.mipLevel,
        attachmentDesc.arrayLayer,
        1
    );
}

void D3D11RenderTarget::CreateDepthStencilAndDSV(DXGI_FORMAT format)
{
    /* Create depth-stencil resource */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width           = GetResolution().width;
        texDesc.Height          = GetResolution().height;
        texDesc.MipLevels       = 1;
        texDesc.ArraySize       = 1;
        texDesc.Format          = format;
        texDesc.SampleDesc      = sampleDesc_;
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = D3D11_BIND_DEPTH_STENCIL;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = 0;
    }
    auto hr = device_->CreateTexture2D(&texDesc, nullptr, depthStencil_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Texture2D", "for render-target depth-stencil");

    /* Create DSV */
    hr = device_->CreateDepthStencilView(depthStencil_.Get(), nullptr, depthStencilView_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11DepthStencilView",  "for render-target");

    /* Store native depth-stencil format */
    depthStencilFormat_ = format;
}

void D3D11RenderTarget::CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc)
{
    ComPtr<ID3D11RenderTargetView> rtv;

    auto hr = device_->CreateRenderTargetView(resource, &rtvDesc, rtv.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11RenderTargetView");

    renderTargetViews_.push_back(rtv);
    renderTargetViewsRef_.push_back(rtv.Get());
}

bool D3D11RenderTarget::HasMultiSampling() const
{
    return (sampleDesc_.Count > 1);
}


} // /namespace LLGL



// ================================================================================
