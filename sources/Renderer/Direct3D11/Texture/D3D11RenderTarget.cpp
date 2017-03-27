/*
 * D3D11RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderTarget.h"
#include "../D3D11RenderSystem.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11RenderTarget::D3D11RenderTarget(ID3D11Device* device, const RenderTargetDescriptor& desc) :
    device_         ( device                           ),
    multiSamples_   ( desc.multiSampling.SampleCount() )
{
}

void D3D11RenderTarget::AttachDepthBuffer(const Gs::Vector2ui& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachStencilBuffer(const Gs::Vector2ui& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachDepthStencilBuffer(const Gs::Vector2ui& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

static void FillViewDescForTexture1D(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipSlice = attachmentDesc.mipLevel;
}

static void FillViewDescForTexture2D(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = attachmentDesc.mipLevel;
}

static void FillViewDescForTexture3D(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension          = D3D11_RTV_DIMENSION_TEXTURE3D;
    viewDesc.Texture3D.MipSlice     = attachmentDesc.mipLevel;
    viewDesc.Texture3D.FirstWSlice  = attachmentDesc.layer;
    viewDesc.Texture3D.WSize        = 1;
}

static void FillViewDescForTextureCube(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(attachmentDesc.cubeFace);
    viewDesc.Texture2DArray.ArraySize       = 1;
}

static void FillViewDescForTexture1DArray(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
    viewDesc.Texture1DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture1DArray.FirstArraySlice = attachmentDesc.layer;
    viewDesc.Texture1DArray.ArraySize       = 1;
}

static void FillViewDescForTexture2DArray(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture2DArray.FirstArraySlice = attachmentDesc.layer;
    viewDesc.Texture2DArray.ArraySize       = 1;
}

static void FillViewDescForTextureCubeArray(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MipSlice        = attachmentDesc.mipLevel;
    viewDesc.Texture2DArray.FirstArraySlice = attachmentDesc.layer * 6 + static_cast<UINT>(attachmentDesc.cubeFace);
    viewDesc.Texture2DArray.ArraySize       = 1;
}

static void FillViewDescForTexture2DMS(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
}

static void FillViewDescForTextureCubeMS(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                      = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    viewDesc.Texture2DMSArray.FirstArraySlice   = static_cast<UINT>(attachmentDesc.cubeFace);
    viewDesc.Texture2DMSArray.ArraySize         = 1;
}

static void FillViewDescForTexture2DArrayMS(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                      = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    viewDesc.Texture2DMSArray.FirstArraySlice   = attachmentDesc.layer;
    viewDesc.Texture2DMSArray.ArraySize         = 1;
}

static void FillViewDescForTextureCubeArrayMS(const RenderTargetAttachmentDescriptor& attachmentDesc, D3D11_RENDER_TARGET_VIEW_DESC& viewDesc)
{
    viewDesc.ViewDimension                      = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    viewDesc.Texture2DMSArray.FirstArraySlice   = attachmentDesc.layer * 6 + static_cast<UINT>(attachmentDesc.cubeFace);
    viewDesc.Texture2DMSArray.ArraySize         = 1;
}

void D3D11RenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc)
{
    /* Get D3D texture object and apply resolution for MIP-map level */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    ApplyMipResolution(texture, attachmentDesc.mipLevel);

    /* Initialize RTV descriptor with attachment procedure and create RTV */
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    InitMemory(rtvDesc);
    
    rtvDesc.Format = textureD3D.GetFormat();

    /*
    If this is a multi-sample render target, but the target texture is not a multi-sample texture,
    an intermediate multi-sample texture is required, which will be 'blitted' (or 'resolved') after the render target was rendered.
    */
    if (HasMultiSampling() && !IsMultiSampleTexture(texture.GetType()))
    {
        /* Get RTV descriptor for intermediate multi-sample texture */
        switch (texture.GetType())
        {
            case TextureType::Texture2D:
                FillViewDescForTexture2DMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::TextureCube:
                FillViewDescForTextureCubeMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DArray:
                FillViewDescForTexture2DArrayMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::TextureCubeArray:
                FillViewDescForTextureCubeArrayMS(attachmentDesc, rtvDesc);
                break;
            default:
                throw std::invalid_argument("failed to attach D3D11 texture to multi-sample render-target");
                break;
        }

        /* Recreate texture resource with multi-sampling */
        D3D11_TEXTURE2D_DESC texDesc;
        textureD3D.GetHardwareTexture().tex2D->GetDesc(&texDesc);
        {
            texDesc.Width               = (texDesc.Width << attachmentDesc.mipLevel);
            texDesc.Height              = (texDesc.Height << attachmentDesc.mipLevel);
            texDesc.MipLevels           = 1;
            texDesc.SampleDesc.Count    = multiSamples_;
            texDesc.MiscFlags           = 0;
        }
        ComPtr<ID3D11Texture2D> tex2DMS;
        auto hr = device_->CreateTexture2D(&texDesc, nullptr, tex2DMS.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 multi-sampled 2D-texture for render-target");

        /* Store multi-sampled texture, and reference to texture target */
        multiSampledAttachments_.push_back(
            {
                tex2DMS,
                textureD3D.GetHardwareTexture().tex2D.Get(),
                D3D11CalcSubresource(attachmentDesc.mipLevel, attachmentDesc.layer, texDesc.MipLevels),
                texDesc.Format
            }
        );

        /* Create RTV for multi-sampled (intermediate) texture */
        CreateAndAppendRTV(tex2DMS.Get(), rtvDesc);
    }
    else
    {
        /* Get RTV descriptor for this the target texture */
        switch (texture.GetType())
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
            case TextureType::TextureCube:
                FillViewDescForTextureCube(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture1DArray:
                FillViewDescForTexture1DArray(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DArray:
                FillViewDescForTexture2DArray(attachmentDesc, rtvDesc);
                break;
            case TextureType::TextureCubeArray:
                FillViewDescForTextureCubeArray(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DMS:
                FillViewDescForTexture2DMS(attachmentDesc, rtvDesc);
                break;
            case TextureType::Texture2DMSArray:
                FillViewDescForTexture2DArrayMS(attachmentDesc, rtvDesc);
                break;
        }
    
        /* Create RTV for target texture */
        CreateAndAppendRTV(textureD3D.GetHardwareTexture().resource.Get(), rtvDesc);
    }
}

void D3D11RenderTarget::DetachAll()
{
    /* Reset all ComPtr instances */
    ResetResolution();

    renderTargetViews_.clear();
    renderTargetViewsRef_.clear();

    depthStencil_.Reset();
    depthStencilView_.Reset();

    multiSampledAttachments_.clear();
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

void D3D11RenderTarget::CreateDepthStencilAndDSV(const Gs::Vector2ui& size, DXGI_FORMAT format)
{
    HRESULT hr = 0;

    /* Apply size to render target resolution, and create depth-stencil */
    ApplyResolution(size);

    /* Create depth stencil texture */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width               = size.x;
        texDesc.Height              = size.y;
        texDesc.MipLevels           = 1;
        texDesc.ArraySize           = 1;
        texDesc.Format              = format;
        texDesc.SampleDesc.Count    = std::max(1u, multiSamples_);
        texDesc.SampleDesc.Quality  = 0;
        texDesc.Usage               = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
        texDesc.CPUAccessFlags      = 0;
        texDesc.MiscFlags           = 0;
    }
    hr = device_->CreateTexture2D(&texDesc, nullptr, depthStencil_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 depth-texture for render-target");

    /* Create DSV */
    hr = device_->CreateDepthStencilView(depthStencil_.Get(), nullptr, depthStencilView_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil-view (DSV) for render-target");
}

void D3D11RenderTarget::CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc)
{
    ComPtr<ID3D11RenderTargetView> rtv;

    auto hr = device_->CreateRenderTargetView(resource, &rtvDesc, rtv.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV)");

    renderTargetViews_.push_back(rtv);
    renderTargetViewsRef_.push_back(rtv.Get());
}

bool D3D11RenderTarget::HasMultiSampling() const
{
    return (multiSamples_ > 1);
}


} // /namespace LLGL



// ================================================================================
