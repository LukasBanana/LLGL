/*
 * D3D11Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Texture.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


Gs::Vector3i D3D11Texture::QueryMipLevelSize(int mipLevel) const
{
    Gs::Vector3i size;

    if (hardwareTexture_.resource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        hardwareTexture_.resource->GetType(&dimension);

        UINT level = static_cast<UINT>(mipLevel);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                hardwareTexture_.tex1D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), 1, 1).Cast<int>();
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                hardwareTexture_.tex2D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), (desc.Height << level), 1).Cast<int>();
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                hardwareTexture_.tex3D->GetDesc(&desc);

                if (level < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width << level), (desc.Height << level), (desc.Depth << level)).Cast<int>();
            }
            break;
        }
    }

    return size;
}

void D3D11Texture::CreateTexture1D(ID3D11Device* device, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    auto hr = device->CreateTexture1D(&desc, initialData, &hardwareTexture_.tex1D);
    DXThrowIfFailed(hr, "failed to create D3D11 1D-texture");
    CreateSRV(device);
}

void D3D11Texture::CreateTexture2D(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    auto hr = device->CreateTexture2D(&desc, initialData, &hardwareTexture_.tex2D);
    DXThrowIfFailed(hr, "failed to create D3D11 2D-texture");
    CreateSRV(device);
}

void D3D11Texture::CreateTexture3D(ID3D11Device* device, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData)
{
    auto hr = device->CreateTexture3D(&desc, initialData, &hardwareTexture_.tex3D);
    DXThrowIfFailed(hr, "failed to create D3D11 3D-texture");
    CreateSRV(device);
}


/*
 * ====== Private: ======
 */

void D3D11Texture::CreateSRV(ID3D11Device* device)
{
    auto hr = device->CreateShaderResourceView(hardwareTexture_.resource.Get(), nullptr, &srv_);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resouce-view (SRV) for texture");
}


} // /namespace LLGL



// ================================================================================
