/*
 * D3D11MipGenerator.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11MipGenerator.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D11MipGenerator& D3D11MipGenerator::Get()
{
    static D3D11MipGenerator instance;
    return instance;
}

void D3D11MipGenerator::InitializeDevice(const ComPtr<ID3D11Device>& device)
{
    device_ = device;
}

void D3D11MipGenerator::Clear()
{
    device_.Reset();
}

void D3D11MipGenerator::GenerateMips(ID3D11DeviceContext* context, D3D11Texture& textureD3D)
{
    /* Generate MIP-maps for the default SRV */
    if (auto srv = textureD3D.GetSRV())
    {
        /* Generate MIP-maps for default SRV */
        context->GenerateMips(srv);
    }
    else
    {
        /* Generate MIP-maps with a temporary subresource SRV */
        GenerateMipsWithSubresourceSRV(context, textureD3D, 0, textureD3D.GetNumMipLevels(), 0, textureD3D.GetNumArrayLayers());
    }
}

void D3D11MipGenerator::GenerateMipsRange(
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    std::uint32_t           baseMipLevel,
    std::uint32_t           numMipLevels,
    std::uint32_t           baseArrayLayer,
    std::uint32_t           numArrayLayers)
{
    if ( baseMipLevel        == 0                              &&
         numMipLevels        == textureD3D.GetNumMipLevels()   &&
         baseArrayLayer      == 0                              &&
         numArrayLayers      == textureD3D.GetNumArrayLayers() &&
         textureD3D.GetSRV() != nullptr )
    {
        /* Generate MIP-maps for the default SRV */
        context->GenerateMips(textureD3D.GetSRV());
    }
    else
    {
        /* Generate MIP-maps for a subresource SRV */
        GenerateMipsWithSubresourceSRV(context, textureD3D, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
    }
}


/*
 * ======= Private: =======
 */

void D3D11MipGenerator::GenerateMipsWithSubresourceSRV(
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    std::uint32_t           baseMipLevel,
    std::uint32_t           numMipLevels,
    std::uint32_t           baseArrayLayer,
    std::uint32_t           numArrayLayers)
{
    /* Generate MIP-maps for a subresource SRV */
    ComPtr<ID3D11ShaderResourceView> srv;
    textureD3D.CreateSubresourceSRV(
        device_.Get(),
        srv.GetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetDXFormat(),
        baseMipLevel,
        numMipLevels,
        baseArrayLayer,
        numArrayLayers
    );
    context->GenerateMips(srv.Get());
}


} // /namespace LLGL



// ================================================================================
