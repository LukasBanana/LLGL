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

void D3D11RenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    AttachTexture(
        texture, TextureType::Texture1D, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE1D;
            desc.Texture1D.MipSlice = static_cast<UINT>(mipLevel);
        }
    );
}

//TODO -> multi-sampled texture (D3D11_RTV_DIMENSION_TEXTURE2DMS) must be created separately
//        and be blitted (ID3D11DeviceContext::CopyResource) to texture target
void D3D11RenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    AttachTexture(
        texture, TextureType::Texture2D, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            if (HasMultiSampling())
            {
                desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2DMS;

                /* Recreate resource with multi-sampling */
                D3D11_TEXTURE2D_DESC texDesc;
                textureD3D.GetHardwareTexture().tex2D->GetDesc(&texDesc);
                {
                    texDesc.MipLevels           = 1;
                    texDesc.SampleDesc.Count    = multiSamples_;
                }
                textureD3D.CreateTexture2D(renderSystem_.GetDevice(), texDesc);
            }
            else
            {
                desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = static_cast<UINT>(mipLevel);
            }
        }
    );
}

void D3D11RenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, TextureType::Texture3D, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            desc.ViewDimension          = D3D11_RTV_DIMENSION_TEXTURE3D;
            desc.Texture3D.MipSlice     = static_cast<UINT>(mipLevel);
            desc.Texture3D.FirstWSlice  = static_cast<UINT>(layer);
            desc.Texture3D.WSize        = 1;
        }
    );
}

void D3D11RenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    AttachTexture(
        texture, TextureType::TextureCube, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            desc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice        = static_cast<UINT>(mipLevel);
            desc.Texture2DArray.FirstArraySlice = static_cast<UINT>(cubeFace);
            desc.Texture2DArray.ArraySize       = 1;
        }
    );
}

void D3D11RenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, TextureType::Texture1DArray, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            desc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
            desc.Texture1DArray.MipSlice        = static_cast<UINT>(mipLevel);
            desc.Texture1DArray.FirstArraySlice = static_cast<UINT>(layer);
            desc.Texture1DArray.ArraySize       = 1;
        }
    );
}

void D3D11RenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, TextureType::Texture2DArray, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            if (HasMultiSampling())
            {
                desc.ViewDimension                      = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                desc.Texture2DMSArray.FirstArraySlice   = static_cast<UINT>(layer);
                desc.Texture2DMSArray.ArraySize         = 1;
            }
            else
            {
                desc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice        = static_cast<UINT>(mipLevel);
                desc.Texture2DArray.FirstArraySlice = static_cast<UINT>(layer);
                desc.Texture2DArray.ArraySize       = 1;
            }
        }
    );
}

void D3D11RenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    AttachTexture(
        texture, TextureType::TextureCubeArray, mipLevel,
        [&](D3D11Texture& textureD3D, D3D11_RENDER_TARGET_VIEW_DESC& desc)
        {
            desc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice        = static_cast<UINT>(mipLevel);
            desc.Texture2DArray.FirstArraySlice = static_cast<UINT>(layer*6) + static_cast<UINT>(cubeFace);
            desc.Texture2DArray.ArraySize       = 1;
        }
    );
}

void D3D11RenderTarget::DetachTextures()
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

void D3D11RenderTarget::CreateDepthStencilAndDSV(const Gs::Vector2i& size, DXGI_FORMAT format)
{
    /* Apply size to render target resolution, and create depth-stencil */
    ApplyResolution(size);

    renderSystem_.CreateDXDepthStencilAndDSV(
        static_cast<UINT>(size.x), static_cast<UINT>(size.y), multiSamples_, format,
        depthStencil_, depthStencilView_
    );
}

void D3D11RenderTarget::CreateAndAppendRTV(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
    ComPtr<ID3D11RenderTargetView> rtv;

    auto hr = renderSystem_.GetDevice()->CreateRenderTargetView(resource, &desc, &rtv);
    DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV)");

    renderTargetViews_.push_back(rtv);
    renderTargetViewsRef_.push_back(rtv.Get());
}

void D3D11RenderTarget::AttachTexture(Texture& texture, const TextureType type, int mipLevel, const AttachTextureCallback& attachmentProc)
{
    /* Validate texture type */
    if (texture.GetType() != type)
        throw std::invalid_argument("texture type mismatch in render target attachment");

    /* Get D3D texture object and apply resolution for MIP-map level */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    ApplyMipResolution(texture, mipLevel);

    /* Initialize RTV descriptor with attachment procedure and create RTV */
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    InitMemory(desc);
    {
        desc.Format = textureD3D.GetFormat();
        attachmentProc(textureD3D, desc);
    }
    CreateAndAppendRTV(textureD3D.GetHardwareTexture().resource.Get(), desc);
}

bool D3D11RenderTarget::HasMultiSampling() const
{
    return (multiSamples_ > 1);
}


} // /namespace LLGL



// ================================================================================
