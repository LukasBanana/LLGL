/*
 * D3D9Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Texture.h"
#include "../../TextureUtils.h"
#include "../D3D9Core.h"
#include "../D3D9Types.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/ImageUtils.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


D3D9Texture::D3D9Texture(IDirect3DDevice9* device, const TextureDescriptor& desc, const ImageView* initialImage) :
    Texture { desc.type, desc.bindFlags }
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture2D:
            CreateD3DTexture(device, desc);
            break;
        case TextureType::Texture3D:
            CreateD3DVolumeTexture(device, desc);
            break;
        case TextureType::TextureCube:
            CreateD3DCubeTexture(device, desc);
            break;
        default:
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("array and multi-sample textures");
            break;
    }
    if (initialImage != nullptr)
        Write(TextureRegion{ TextureSubresource{}, Offset3D{}, desc.extent }, *initialImage);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9Texture::SetDebugName(const char* name)
{
    //TODO
}

bool D3D9Texture::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

TextureDescriptor D3D9Texture::GetDesc() const
{
    TextureDescriptor desc;
    desc.type = GetType();

    switch (baseTexture_->GetType())
    {
        case D3DRTYPE_TEXTURE:
        {
            /* Get surface description of standard D3D9 texture */
            ComPtr<IDirect3DTexture9> d3dTexture;
            baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dTexture));
            D3DSURFACE_DESC surfaceDesc;
            d3dTexture->GetLevelDesc(0, &surfaceDesc);

            desc.format         = D3D9Types::ToFormat(surfaceDesc.Format);
            desc.extent.width   = surfaceDesc.Width;
            desc.extent.height  = surfaceDesc.Height;
            desc.extent.depth   = 1;
            desc.mipLevels      = d3dTexture->GetLevelCount();
        }
        break;

        case D3DRTYPE_VOLUMETEXTURE:
        {
            /* Get surface description of D3D9 volume texture */
            ComPtr<IDirect3DVolumeTexture9> d3dVolumeTexture;
            baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dVolumeTexture));
            D3DVOLUME_DESC volumeDesc;
            d3dVolumeTexture->GetLevelDesc(0, &volumeDesc);

            desc.format         = D3D9Types::ToFormat(volumeDesc.Format);
            desc.extent.width   = volumeDesc.Width;
            desc.extent.height  = volumeDesc.Height;
            desc.extent.depth   = volumeDesc.Depth;
            desc.mipLevels      = d3dVolumeTexture->GetLevelCount();
        }
        break;

        case D3DRTYPE_CUBETEXTURE:
        {
            /* Get surface description of D3D9 cube texture */
            ComPtr<IDirect3DCubeTexture9> d3dCubeTexture;
            baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dCubeTexture));
            D3DSURFACE_DESC surfaceDesc;
            d3dCubeTexture->GetLevelDesc(0, &surfaceDesc);

            desc.format         = D3D9Types::ToFormat(surfaceDesc.Format);
            desc.extent.width   = surfaceDesc.Width;
            desc.extent.height  = surfaceDesc.Height;
            desc.extent.depth   = 1;
            desc.mipLevels      = d3dCubeTexture->GetLevelCount();
            desc.arrayLayers    = 6;
        }
        break;
    }

    return desc;
}

Format D3D9Texture::GetFormat() const
{
    return GetDesc().format;
}

Extent3D D3D9Texture::GetMipExtent(std::uint32_t mipLevel) const
{
    const TextureDescriptor desc = GetDesc();
    return LLGL::GetMipExtent(GetType(), desc.extent, std::min(mipLevel, desc.mipLevels - 1));
}

SubresourceFootprint D3D9Texture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    const TextureDescriptor desc = GetDesc();
    return CalcPackedSubresourceFootprint(GetType(), GetFormat(), desc.extent, mipLevel, desc.arrayLayers);
}

HRESULT D3D9Texture::Write(const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    HRESULT hr = S_OK;

    if (textureRegion.subresource.numMipLevels   != 1 ||
        textureRegion.subresource.baseArrayLayer != 0 ||
        textureRegion.subresource.numArrayLayers != 1)
    {
        return E_INVALIDARG;
    }

    if (IsCompressedFormat(GetFormat()))
        return E_NOTIMPL; //TODO: support compressed formats

    const FormatAttributes formatAttribs = GetFormatAttribs(GetFormat());
    if (formatAttribs.format   != srcImageView.format ||
        formatAttribs.dataType != srcImageView.dataType)
    {
        return E_NOTIMPL; //TODO: convert image data
    }

    UINT mipLevel = textureRegion.subresource.baseMipLevel;
    D3DLOCKED_RECT lockedRect = {};

    RECT inRect;
    {
        inRect.left     = textureRegion.offset.x;
        inRect.top      = textureRegion.offset.y;
        inRect.right    = textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width);
        inRect.bottom   = textureRegion.offset.y + static_cast<LONG>(textureRegion.extent.height);
    }

    switch (baseTexture_->GetType())
    {
        case D3DRTYPE_TEXTURE:
        {
            /* Get surface description of standard D3D9 texture */
            ComPtr<IDirect3DTexture9> d3dTexture;
            hr = baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dTexture));
            if (SUCCEEDED(hr))
            {
                hr = d3dTexture->LockRect(mipLevel, &lockedRect, &inRect, D3DLOCK_DISCARD);
                if (SUCCEEDED(hr))
                {
                    BitBlit(
                        textureRegion.extent,
                        formatAttribs.bitSize / 8,
                        static_cast<char*>(lockedRect.pBits),
                        lockedRect.Pitch,
                        0,
                        static_cast<const char*>(srcImageView.data),
                        srcImageView.rowStride,
                        srcImageView.layerStride
                    );
                    d3dTexture->UnlockRect(mipLevel);
                }
            }
        }
        break;

        case D3DRTYPE_VOLUMETEXTURE:
        {
            /* Get surface description of D3D9 volume texture */
            ComPtr<IDirect3DVolumeTexture9> d3dVolumeTexture;
            hr = baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dVolumeTexture));
            if (SUCCEEDED(hr))
            {
                //TODO
            }
        }
        break;

        case D3DRTYPE_CUBETEXTURE:
        {
            /* Get surface description of D3D9 cube texture */
            ComPtr<IDirect3DCubeTexture9> d3dCubeTexture;
            hr = baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dCubeTexture));
            if (SUCCEEDED(hr))
            {
                //TODO
            }
        }
        break;
    }

    return hr;
}

HRESULT D3D9Texture::Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    return E_NOTIMPL; //TODO
}


/*
 * ======= Private: =======
 */

static DWORD GetD3DTextureUsage(long bindFlags)
{
    DWORD usage = 0;
    if ((bindFlags & BindFlags::ColorAttachment) != 0)
        usage |= D3DUSAGE_RENDERTARGET;
    else if ((bindFlags & BindFlags::DepthStencilAttachment) != 0)
        usage |= D3DUSAGE_DEPTHSTENCIL;
    return usage;
}

void D3D9Texture::CreateD3DTexture(IDirect3DDevice9* device, const TextureDescriptor& desc)
{
    ComPtr<IDirect3DTexture9> d3dTex;
    HRESULT hr = device->CreateTexture(
        desc.extent.width,
        desc.extent.height,
        desc.mipLevels,
        0,//GetD3DTextureUsage(desc.bindFlags),
        D3D9Types::ToD3DFormat(desc.format),
        D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,
        &d3dTex,
        nullptr
    );
    D3DThrowIfCreateFailed(hr, "IDirect3DTexture9");
    baseTexture_ = d3dTex;
}

void D3D9Texture::CreateD3DVolumeTexture(IDirect3DDevice9* device, const TextureDescriptor& desc)
{
    ComPtr<IDirect3DVolumeTexture9> d3dTex;
    HRESULT hr = device->CreateVolumeTexture(
        desc.extent.width,
        desc.extent.height,
        desc.extent.depth,
        desc.mipLevels,
        0,
        D3D9Types::ToD3DFormat(desc.format),
        D3DPOOL_DEFAULT,
        &d3dTex,
        nullptr
    );
    D3DThrowIfCreateFailed(hr, "IDirect3DVolumeTexture9");
    baseTexture_ = d3dTex;
}

void D3D9Texture::CreateD3DCubeTexture(IDirect3DDevice9* device, const TextureDescriptor& desc)
{
    LLGL_ASSERT(desc.extent.width == desc.extent.height);
    ComPtr<IDirect3DCubeTexture9> d3dTex;
    HRESULT hr = device->CreateCubeTexture(
        desc.extent.width,
        desc.mipLevels,
        0,
        D3D9Types::ToD3DFormat(desc.format),
        D3DPOOL_DEFAULT,
        &d3dTex,
        nullptr
    );
    D3DThrowIfCreateFailed(hr, "IDirect3DCubeTexture9");
    baseTexture_ = d3dTex;
}


} // /namespace LLGL



// ================================================================================
