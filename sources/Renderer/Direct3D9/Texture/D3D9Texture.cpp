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


static DWORD GetD3DTextureUsage(long bindFlags)
{
    DWORD usage = 0;
    if ((bindFlags & BindFlags::ColorAttachment) != 0)
        usage |= D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP;
    else if ((bindFlags & BindFlags::DepthStencilAttachment) != 0)
        usage |= D3DUSAGE_DEPTHSTENCIL;
    return usage;
}

static ComPtr<IDirect3DTexture9> CreateD3DTexture(
    IDirect3DDevice9* device, const Extent2D& extent, UINT mipLevels, DWORD usage, D3DFORMAT format, D3DPOOL pool = D3DPOOL_DEFAULT)
{
    ComPtr<IDirect3DTexture9> d3dTex;
    HRESULT hr = device->CreateTexture(extent.width, extent.height, mipLevels, usage, format, pool, &d3dTex, nullptr);
    D3DThrowIfCreateFailed(hr, "IDirect3DTexture9");
    return d3dTex;
}

static ComPtr<IDirect3DVolumeTexture9> CreateD3DVolumeTexture(
    IDirect3DDevice9* device, const Extent3D& extent, UINT mipLevels, DWORD usage, D3DFORMAT format, D3DPOOL pool = D3DPOOL_DEFAULT)
{
    ComPtr<IDirect3DVolumeTexture9> d3dTex;
    HRESULT hr = device->CreateVolumeTexture(extent.width, extent.height, extent.depth, mipLevels, usage, format, pool, &d3dTex, nullptr);
    D3DThrowIfCreateFailed(hr, "IDirect3DVolumeTexture9");
    return d3dTex;
}

static ComPtr<IDirect3DCubeTexture9> CreateD3DCubeTexture(
    IDirect3DDevice9* device, const Extent2D& extent, UINT mipLevels, DWORD usage, D3DFORMAT format, D3DPOOL pool = D3DPOOL_DEFAULT)
{
    LLGL_ASSERT(extent.width == extent.height, "D3D9 cube texture must have squared extent, but %u x %u was specified", extent.width, extent.height);
    const UINT edgeLength = extent.width;
    ComPtr<IDirect3DCubeTexture9> d3dTex;
    HRESULT hr = device->CreateCubeTexture(edgeLength, mipLevels, usage, format, pool, &d3dTex, nullptr);
    D3DThrowIfCreateFailed(hr, "IDirect3DCubeTexture9");
    return d3dTex;
}

static ComPtr<IDirect3DBaseTexture9> CreateD3DBaseTexture(IDirect3DDevice9* device, const TextureDescriptor& desc)
{
    D3DFORMAT d3dFormat = D3D9Types::ToD3DFormat(desc.format);
    switch (desc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture2D:
            return CreateD3DTexture(device, Extent2D{ desc.extent.width, desc.extent.height }, desc.mipLevels, GetD3DTextureUsage(desc.bindFlags), d3dFormat);
        case TextureType::Texture3D:
            return CreateD3DVolumeTexture(device, desc.extent, desc.mipLevels, 0, d3dFormat);
        case TextureType::TextureCube:
            return CreateD3DCubeTexture(device, Extent2D{ desc.extent.width, desc.extent.height }, desc.mipLevels, 0, d3dFormat);
        default:
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("array and multi-sample textures");
            break;
    }
}

D3D9Texture::D3D9Texture(IDirect3DDevice9* device, const TextureDescriptor& desc, const ImageView* initialImage) :
    Texture      { desc.type, desc.bindFlags          },
    baseTexture_ { CreateD3DBaseTexture(device, desc) }
{
    if (initialImage != nullptr)
    {
        Write(TextureRegion{ TextureSubresource{}, Offset3D{}, desc.extent }, *initialImage);
        if ((desc.miscFlags & MiscFlags::GenerateMips) != 0)
            baseTexture_->GenerateMipSubLevels();
    }
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

    const RECT srcRect{ 0, 0, static_cast<LONG>(textureRegion.extent.width), static_cast<LONG>(textureRegion.extent.height) };
    const POINT dstPoint{ textureRegion.offset.x, textureRegion.offset.y };

    switch (baseTexture_->GetType())
    {
        case D3DRTYPE_TEXTURE:
        {
            /* Get surface description of standard D3D9 texture */
            ComPtr<IDirect3DTexture9> d3dTexture;
            LLGL_VERIFY_HRESULT(baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dTexture)));

            /* Create staging texture to copy CPU data to GPU */
            ComPtr<IDirect3DDevice9> device;
            d3dTexture->GetDevice(device.GetAddressOf());

            D3DSURFACE_DESC surfaceDesc = {};
            d3dTexture->GetLevelDesc(mipLevel, &surfaceDesc);
            ComPtr<IDirect3DTexture9> d3dStagingTexture = CreateD3DTexture(
                device.Get(),
                Extent2D{ textureRegion.extent.width, textureRegion.extent.height },
                1, // single MIP-map
                0, // default usage
                surfaceDesc.Format,
                D3DPOOL_SYSTEMMEM // CPU write access
            );

            LLGL_VERIFY_HRESULT(d3dStagingTexture->LockRect(0, &lockedRect, &srcRect, 0));
            {
                /* Copy source image into staging texture */
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
            }
            LLGL_VERIFY_HRESULT(d3dStagingTexture->UnlockRect(0));

            /* Copy staging texture into destination GPU texture */
            ComPtr<IDirect3DSurface9> d3dSurface, d3dStagingSurface;
            LLGL_VERIFY_HRESULT(d3dTexture->GetSurfaceLevel(mipLevel, d3dSurface.GetAddressOf()));
            LLGL_VERIFY_HRESULT(d3dStagingTexture->GetSurfaceLevel(0, d3dStagingSurface.GetAddressOf()));
            LLGL_VERIFY_HRESULT(device->UpdateSurface(d3dStagingSurface.Get(), &srcRect, d3dSurface.Get(), &dstPoint));
        }
        break;

        case D3DRTYPE_VOLUMETEXTURE:
        {
            /* Get surface description of D3D9 volume texture */
            ComPtr<IDirect3DVolumeTexture9> d3dVolumeTexture;
            LLGL_VERIFY_HRESULT(baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dVolumeTexture)));

            //TODO
        }
        break;

        case D3DRTYPE_CUBETEXTURE:
        {
            /* Get surface description of D3D9 cube texture */
            ComPtr<IDirect3DCubeTexture9> d3dCubeTexture;
            LLGL_VERIFY_HRESULT(baseTexture_->QueryInterface(IID_PPV_ARGS(&d3dCubeTexture)));

            //TODO
        }
        break;
    }

    return S_OK;
}

HRESULT D3D9Texture::Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    return E_NOTIMPL; //TODO
}


/*
 * ======= Private: =======
 */


} // /namespace LLGL



// ================================================================================
