/*
 * D3D11MipGenerator.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_MIP_GENERATOR_H
#define LLGL_D3D11_MIP_GENERATOR_H


#include "D3D11Texture.h"
#include "../../DXCommon/ComPtr.h"
#include "../Direct3D11.h"


namespace LLGL
{


class D3D11Texture;

// Direct3D 11 MIP-map generator singleton.
class D3D11MipGenerator
{

    public:

        // Returns the instance of this singleton.
        static D3D11MipGenerator& Get();

    public:

        D3D11MipGenerator(const D3D11MipGenerator&) = delete;
        D3D11MipGenerator& operator = (const D3D11MipGenerator&) = delete;

        D3D11MipGenerator(D3D11MipGenerator&&) = delete;
        D3D11MipGenerator& operator = (D3D11MipGenerator&&) = delete;

        void InitializeDevice(const ComPtr<ID3D11Device>& device);
        void Clear();

        void GenerateMips(ID3D11DeviceContext* context, D3D11Texture& textureD3D);

        void GenerateMipsRange(
            ID3D11DeviceContext*    context,
            D3D11Texture&           textureD3D,
            std::uint32_t           baseMipLevel,
            std::uint32_t           numMipLevels,
            std::uint32_t           baseArrayLayer = 0,
            std::uint32_t           numArrayLayers = 1
        );

    private:

        D3D11MipGenerator() = default;

        void GenerateMipsWithSubresourceSRV(
            ID3D11DeviceContext*    context,
            D3D11Texture&           textureD3D,
            std::uint32_t           baseMipLevel,
            std::uint32_t           numMipLevels,
            std::uint32_t           baseArrayLayer,
            std::uint32_t           numArrayLayers
        );

    private:

        ComPtr<ID3D11Device> device_;

};


} // /namespace LLGL


#endif



// ================================================================================
