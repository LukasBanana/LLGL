/*
 * D3D11RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderTarget.h"
#include "../D3D11RenderSystem.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11RenderTarget::D3D11RenderTarget(D3D11RenderSystem& renderSystem, unsigned int multiSamples) :
    renderSystem_( renderSystem ),
    multiSamples_( multiSamples )
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
    D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
    InitMemory(viewDesc);
    {
        viewDesc.Format = textureD3D.GetFormat();

        if (HasMultiSampling())
        {
            switch (texture.GetType())
            {
                case TextureType::Texture2D:
                    FillViewDescForTexture2DMS(attachmentDesc, viewDesc);
                    break;
                case TextureType::TextureCube:
                    FillViewDescForTextureCubeMS(attachmentDesc, viewDesc);
                    break;
                case TextureType::Texture2DArray:
                    FillViewDescForTexture2DArrayMS(attachmentDesc, viewDesc);
                    break;
                case TextureType::TextureCubeArray:
                    FillViewDescForTextureCubeArrayMS(attachmentDesc, viewDesc);
                    break;
                default:
                    throw std::invalid_argument("only 2D-textures can be attached to a D3D11 multi-sample render-target");
                    break;
            }

            //TODO -> multi-sampled texture (D3D11_RTV_DIMENSION_TEXTURE2DMS) must be created separately
            //        and be blitted (ID3D11DeviceContext::CopyResource) to texture target

            /* Recreate resource with multi-sampling */
            D3D11_TEXTURE2D_DESC texDesc;
            textureD3D.GetHardwareTexture().tex2D->GetDesc(&texDesc);
            {
                texDesc.MipLevels           = 1;
                texDesc.SampleDesc.Count    = multiSamples_;
                texDesc.MiscFlags           = 0;
            }
            textureD3D.CreateTexture2D(renderSystem_.GetDevice(), texDesc);
        }
        else
        {
            switch (texture.GetType())
            {
                case TextureType::Texture1D:
                    FillViewDescForTexture1D(attachmentDesc, viewDesc);
                    break;
                case TextureType::Texture2D:
                    FillViewDescForTexture2D(attachmentDesc, viewDesc);
                    break;
                case TextureType::Texture3D:
                    FillViewDescForTexture3D(attachmentDesc, viewDesc);
                    break;
                case TextureType::TextureCube:
                    FillViewDescForTextureCube(attachmentDesc, viewDesc);
                    break;
                case TextureType::Texture1DArray:
                    FillViewDescForTexture1DArray(attachmentDesc, viewDesc);
                    break;
                case TextureType::Texture2DArray:
                    FillViewDescForTexture2DArray(attachmentDesc, viewDesc);
                    break;
                case TextureType::TextureCubeArray:
                    FillViewDescForTextureCubeArray(attachmentDesc, viewDesc);
                    break;
            }
        }
    }
    CreateAndAppendRTV(textureD3D.GetHardwareTexture().resource.Get(), viewDesc);
}

void D3D11RenderTarget::DetachAll()
{
    /* Reset all ComPtr instances */
    ResetResolution();

    renderTargetViews_.clear();
    renderTargetViewsRef_.clear();

    depthStencil_.Reset();
    depthStencilView_.Reset();
}


/*
 * ======= Private: =======
 */

void D3D11RenderTarget::CreateDepthStencilAndDSV(const Gs::Vector2ui& size, DXGI_FORMAT format)
{
    /* Apply size to render target resolution, and create depth-stencil */
    ApplyResolution(size);
    renderSystem_.CreateDXDepthStencilAndDSV(size.x, size.y, multiSamples_, format, depthStencil_, depthStencilView_);
}

void D3D11RenderTarget::CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
    ComPtr<ID3D11RenderTargetView> rtv;

    auto hr = renderSystem_.GetDevice()->CreateRenderTargetView(resource, &desc, &rtv);
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
